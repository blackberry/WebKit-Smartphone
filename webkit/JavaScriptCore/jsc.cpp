/*
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 *  Copyright (C) 2006 Bjoern Graf (bjoern.graf@gmail.com)
 *  Copyright (C) 2009 Research in Motion Ltd. http://www.rim.com/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "config.h"

#include "BytecodeGenerator.h"
#include "Completion.h"
#include "CurrentTime.h"
#include "ExceptionHelpers.h"
#include "InitializeThreading.h"
#include "JSArray.h"
#include "JSFunction.h"
#include "JSLock.h"
#include "JSString.h"
#include "PrototypeFunction.h"
#include "SamplingTool.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !OS(WINDOWS)
#include <unistd.h>
#endif

#if HAVE(READLINE)
#include <readline/history.h>
#include <readline/readline.h>
#endif

#if HAVE(SYS_TIME_H)
#include <sys/time.h>
#endif

#if HAVE(SIGNAL_H)
#include <signal.h>
#endif

#if COMPILER(MSVC) && !OS(WINCE) && !OS(OLYMPIA)
#include <crtdbg.h>
#include <mmsystem.h>
#include <windows.h>
#endif

#if PLATFORM(OLYMPIA)
#include "jsc.h"
#include "stdarg.h"
#define printf jscPrintf
#define putchar jscPutChar
static void jscPrintf(const char* format, ...);
static void jscPutChar(const char c);
#if COMPILER(RVCT)
// FIXME: what should we do to exit? longjmp?
#define exit(x)
#endif
#endif

#if PLATFORM(QT)
#include <QCoreApplication>
#include <QDateTime>
#endif

using namespace JSC;
using namespace WTF;

static void cleanupGlobalData(JSGlobalData*);
static bool fillBufferWithContentsOfFile(const UString& fileName, Vector<char>& buffer);

static EncodedJSValue JSC_HOST_CALL functionPrint(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionDebug(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionGC(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionVersion(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionRun(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionLoad(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionCheckSyntax(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionReadline(ExecState*);
static NO_RETURN_WITH_VALUE EncodedJSValue JSC_HOST_CALL functionQuit(ExecState*);

#if PLATFORM(OLYMPIA)
static EncodedJSValue JSC_HOST_CALL functionAlert(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionConfirm(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionPrompt(ExecState*);
#endif

#if ENABLE(SAMPLING_FLAGS)
static EncodedJSValue JSC_HOST_CALL functionSetSamplingFlags(ExecState*);
static EncodedJSValue JSC_HOST_CALL functionClearSamplingFlags(ExecState*);
#endif

struct Script {
    bool isFile;
    char* argument;

    Script(bool isFile, char *argument)
        : isFile(isFile)
        , argument(argument)
    {
    }
};

struct Options {
    Options()
        : interactive(false)
        , dump(false)
    {
    }

    bool interactive;
    bool dump;
    Vector<Script> scripts;
    Vector<UString> arguments;
};

static const char interactivePrompt[] = "> ";
static const UString interpreterName("Interpreter");

class StopWatch {
public:
    void start();
    void stop();
    long getElapsedMS(); // call stop() first

private:
    double m_startTime;
    double m_stopTime;
};

void StopWatch::start()
{
    m_startTime = currentTime();
}

void StopWatch::stop()
{
    m_stopTime = currentTime();
}

long StopWatch::getElapsedMS()
{
    return static_cast<long>((m_stopTime - m_startTime) * 1000);
}

class GlobalObject : public JSGlobalObject {
public:
    GlobalObject(const Vector<UString>& arguments);
    virtual UString className() const { return "global"; }
};
COMPILE_ASSERT(!IsInteger<GlobalObject>::value, WTF_IsInteger_GlobalObject_false);
ASSERT_CLASS_FITS_IN_CELL(GlobalObject);

GlobalObject::GlobalObject(const Vector<UString>& arguments)
    : JSGlobalObject()
{
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 1, Identifier(globalExec(), "debug"), functionDebug));
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 1, Identifier(globalExec(), "print"), functionPrint));
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 0, Identifier(globalExec(), "quit"), functionQuit));
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 0, Identifier(globalExec(), "gc"), functionGC));
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 1, Identifier(globalExec(), "version"), functionVersion));
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 1, Identifier(globalExec(), "run"), functionRun));
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 1, Identifier(globalExec(), "load"), functionLoad));
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 1, Identifier(globalExec(), "checkSyntax"), functionCheckSyntax));
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 0, Identifier(globalExec(), "readline"), functionReadline));

#if PLATFORM(OLYMPIA)
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 1, Identifier(globalExec(), "alert"), functionAlert));
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 1, Identifier(globalExec(), "confirm"), functionConfirm));
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 1, Identifier(globalExec(), "prompt"), functionPrompt));
#endif

#if ENABLE(SAMPLING_FLAGS)
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 1, Identifier(globalExec(), "setSamplingFlags"), functionSetSamplingFlags));
    putDirectFunction(globalExec(), new (globalExec()) NativeFunctionWrapper(globalExec(), this, prototypeFunctionStructure(), 1, Identifier(globalExec(), "clearSamplingFlags"), functionClearSamplingFlags));
#endif

    JSObject* array = constructEmptyArray(globalExec());
    for (size_t i = 0; i < arguments.size(); ++i)
        array->put(globalExec(), i, jsString(globalExec(), arguments[i]));
    putDirect(Identifier(globalExec(), "arguments"), array);
}

EncodedJSValue JSC_HOST_CALL functionPrint(ExecState* exec)
{
    for (unsigned i = 0; i < exec->argumentCount(); ++i) {
        if (i)
            putchar(' ');

        printf("%s", exec->argument(i).toString(exec).utf8().data());
    }

    putchar('\n');
    fflush(stdout);
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionDebug(ExecState* exec)
{
    fprintf(stderr, "--> %s\n", exec->argument(0).toString(exec).utf8().data());
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionGC(ExecState* exec)
{
    JSLock lock(SilenceAssertionsOnly);
    exec->heap()->collectAllGarbage();
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionVersion(ExecState*)
{
    // We need this function for compatibility with the Mozilla JS tests but for now
    // we don't actually do any version-specific handling
    return JSValue::encode(jsUndefined());
}

EncodedJSValue JSC_HOST_CALL functionRun(ExecState* exec)
{
    StopWatch stopWatch;
    UString fileName = exec->argument(0).toString(exec);
    Vector<char> script;
    if (!fillBufferWithContentsOfFile(fileName, script))
        return JSValue::encode(throwError(exec, createError(exec, "Could not open file.")));

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();

    stopWatch.start();
    evaluate(globalObject->globalExec(), globalObject->globalScopeChain(), makeSource(script.data(), fileName));
    stopWatch.stop();

    return JSValue::encode(jsNumber(globalObject->globalExec(), stopWatch.getElapsedMS()));
}

EncodedJSValue JSC_HOST_CALL functionLoad(ExecState* exec)
{
    UString fileName = exec->argument(0).toString(exec);
    Vector<char> script;
    if (!fillBufferWithContentsOfFile(fileName, script))
        return JSValue::encode(throwError(exec, createError(exec, "Could not open file.")));

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    Completion result = evaluate(globalObject->globalExec(), globalObject->globalScopeChain(), makeSource(script.data(), fileName));
    if (result.complType() == Throw)
        throwError(exec, result.value());
    return JSValue::encode(result.value());
}

EncodedJSValue JSC_HOST_CALL functionCheckSyntax(ExecState* exec)
{
    UString fileName = exec->argument(0).toString(exec);
    Vector<char> script;
    if (!fillBufferWithContentsOfFile(fileName, script))
        return JSValue::encode(throwError(exec, createError(exec, "Could not open file.")));

    JSGlobalObject* globalObject = exec->lexicalGlobalObject();
    Completion result = checkSyntax(globalObject->globalExec(), makeSource(script.data(), fileName));
    if (result.complType() == Throw)
        throwError(exec, result.value());
    return JSValue::encode(result.value());
}

#if ENABLE(SAMPLING_FLAGS)
EncodedJSValue JSC_HOST_CALL functionSetSamplingFlags(ExecState* exec)
{
    for (unsigned i = 0; i < exec->argumentCount(); ++i) {
        unsigned flag = static_cast<unsigned>(exec->argument(i).toNumber(exec));
        if ((flag >= 1) && (flag <= 32))
            SamplingFlags::setFlag(flag);
    }
    return JSValue::encode(jsNull());
}

EncodedJSValue JSC_HOST_CALL functionClearSamplingFlags(ExecState* exec)
{
    for (unsigned i = 0; i < exec->argumentCount(); ++i) {
        unsigned flag = static_cast<unsigned>(exec->argument(i).toNumber(exec));
        if ((flag >= 1) && (flag <= 32))
            SamplingFlags::clearFlag(flag);
    }
    return JSValue::encode(jsNull());
}
#endif

EncodedJSValue JSC_HOST_CALL functionReadline(ExecState* exec)
{
    Vector<char, 256> line;
    int c;
    while ((c = getchar()) != EOF) {
        // FIXME: Should we also break on \r? 
        if (c == '\n')
            break;
        line.append(c);
    }
    line.append('\0');
    return JSValue::encode(jsString(exec, line.data()));
}

EncodedJSValue JSC_HOST_CALL functionQuit(ExecState* exec)
{
    // Technically, destroying the heap in the middle of JS execution is a no-no,
    // but we want to maintain compatibility with the Mozilla test suite, so
    // we pretend that execution has terminated to avoid ASSERTs, then tear down the heap.
    exec->globalData().dynamicGlobalObject = 0;

    cleanupGlobalData(&exec->globalData());
    exit(EXIT_SUCCESS);

#if COMPILER(MSVC) && OS(WINCE)
    // Without this, Visual Studio will complain that this method does not return a value.
    return JSValue::encode(jsUndefined());
#endif
}

// Use SEH for Release builds only to get rid of the crash report dialog
// (luckily the same tests fail in Release and Debug builds so far). Need to
// be in a separate main function because the jscmain function requires object
// unwinding.

#if COMPILER(MSVC) && !defined(_DEBUG) && !OS(WINCE)
#define TRY       __try {
#define EXCEPT(x) } __except (EXCEPTION_EXECUTE_HANDLER) { x; }
#else
#define TRY
#define EXCEPT(x)
#endif

#if OS(OLYMPIA)
static JSGlobalData* globalData = 0;
static int jscInitialized = 0;

void jscInitialize()
{
    if (jscInitialized++)
        return;

    MemoryManager::initialize();
    JSC::initializeThreading();
    globalData = JSGlobalData::create().releaseRef();
}

void jscCleanup()
{
    if (--jscInitialized)
        return;

    cleanupGlobalData(globalData);
    globalData = 0;
}

static int jscmain(int argc, char** argv, JSGlobalData*);
static bool runScript(GlobalObject* globalObject, const char* source);

int jscTest(int argc, char** argv)
{
    jscInitialize();
    int res = 0;
    TRY
        res = jscmain(argc, argv, globalData);
    EXCEPT(res = 3)
    jscCleanup();
    return res;
}

int jscRunScript(const char* buffer)
{
    ASSERT(jscInitialized);
    if (!jscInitialized)
        return -1;

    JSLock lock(SilenceAssertionsOnly);

    Vector<UString> arguments;
    GlobalObject* globalObject = new (globalData) GlobalObject(arguments);
    bool success = runScript(globalObject, buffer);
    globalData->heap.collect();

    return success ? 0 : -2;
}

static bool runScript(GlobalObject* globalObject, const char* source)
{
    UString script(source);

    bool success = true;

    Completion completion = evaluate(globalObject->globalExec(), globalObject->globalScopeChain(), makeSource(script, "dummy_file"));
    success = success && completion.complType() != Throw;

    if (completion.complType() == Throw)
        printf("Exception: %s\n", completion.value().toString(globalObject->globalExec()).ascii());

    globalObject->globalExec()->clearException();

    return success;
}

#else
int jscmain(int argc, char** argv, JSGlobalData*);

int main(int argc, char** argv)
{
#if OS(OLYMPIA)
    MemoryManager::initialize();
#endif

#if defined(_DEBUG) && OS(WINDOWS)
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
#endif

#if COMPILER(MSVC) && !OS(WINCE) && !PLATFORM(OLYMPIA)
    timeBeginPeriod(1);
#endif

#if PLATFORM(QT)
    QCoreApplication app(argc, argv);
#endif

    // Initialize JSC before getting JSGlobalData.
    JSC::initializeThreading();

    // We can't use destructors in the following code because it uses Windows
    // Structured Exception Handling
    int res = 0;
    JSGlobalData* globalData = JSGlobalData::create(ThreadStackTypeLarge).releaseRef();
    TRY
        res = jscmain(argc, argv, globalData);
    EXCEPT(res = 3)

    cleanupGlobalData(globalData);
    return res;
}
#endif

static void cleanupGlobalData(JSGlobalData* globalData)
{
    JSLock lock(SilenceAssertionsOnly);
    globalData->heap.destroy();
    globalData->deref();
}

static bool runWithScripts(GlobalObject* globalObject, const Vector<Script>& scripts, bool dump)
{
    UString script;
    UString fileName;
    Vector<char> scriptBuffer;

    if (dump)
        BytecodeGenerator::setDumpsGeneratedCode(true);

    JSGlobalData* globalData = globalObject->globalData();

#if ENABLE(SAMPLING_FLAGS)
    SamplingFlags::start();
#endif

    bool success = true;
    for (size_t i = 0; i < scripts.size(); i++) {
        if (scripts[i].isFile) {
            fileName = scripts[i].argument;
            if (!fillBufferWithContentsOfFile(fileName, scriptBuffer))
                return false; // fail early so we can catch missing files
            script = scriptBuffer.data();
        } else {
            script = scripts[i].argument;
            fileName = "[Command Line]";
        }

        globalData->startSampling();

        Completion completion = evaluate(globalObject->globalExec(), globalObject->globalScopeChain(), makeSource(script, fileName));
        success = success && completion.complType() != Throw;
        if (dump) {
            if (completion.complType() == Throw)
                printf("Exception: %s\n", completion.value().toString(globalObject->globalExec()).utf8().data());
            else
                printf("End: %s\n", completion.value().toString(globalObject->globalExec()).utf8().data());
        }

        globalData->stopSampling();
        globalObject->globalExec()->clearException();
    }

#if ENABLE(SAMPLING_FLAGS)
    SamplingFlags::stop();
#endif
    globalData->dumpSampleData(globalObject->globalExec());
#if ENABLE(SAMPLING_COUNTERS)
    AbstractSamplingCounter::dump();
#endif
#if ENABLE(REGEXP_TRACING)
    globalData->dumpRegExpTrace();
#endif
    return success;
}

#define RUNNING_FROM_XCODE 0

static void runInteractive(GlobalObject* globalObject)
{
    while (true) {
#if HAVE(READLINE) && !RUNNING_FROM_XCODE
        char* line = readline(interactivePrompt);
        if (!line)
            break;
        if (line[0])
            add_history(line);
        Completion completion = evaluate(globalObject->globalExec(), globalObject->globalScopeChain(), makeSource(line, interpreterName));
        free(line);
#else
        printf("%s", interactivePrompt);
        Vector<char, 256> line;
        int c;
        while ((c = getchar()) != EOF) {
            // FIXME: Should we also break on \r? 
            if (c == '\n')
                break;
            line.append(c);
        }
        if (line.isEmpty())
            break;
        line.append('\0');
        Completion completion = evaluate(globalObject->globalExec(), globalObject->globalScopeChain(), makeSource(line.data(), interpreterName));
#endif
        if (completion.complType() == Throw)
            printf("Exception: %s\n", completion.value().toString(globalObject->globalExec()).utf8().data());
        else
            printf("%s\n", completion.value().toString(globalObject->globalExec()).utf8().data());

        globalObject->globalExec()->clearException();
    }
    printf("\n");
}

static NO_RETURN void printUsageStatement(JSGlobalData* globalData, bool help = false)
{
    fprintf(stderr, "Usage: jsc [options] [files] [-- arguments]\n");
    fprintf(stderr, "  -d         Dumps bytecode (debug builds only)\n");
    fprintf(stderr, "  -e         Evaluate argument as script code\n");
    fprintf(stderr, "  -f         Specifies a source file (deprecated)\n");
    fprintf(stderr, "  -h|--help  Prints this help message\n");
    fprintf(stderr, "  -i         Enables interactive mode (default if no files are specified)\n");
#if HAVE(SIGNAL_H)
    fprintf(stderr, "  -s         Installs signal handlers that exit on a crash (Unix platforms only)\n");
#endif

    cleanupGlobalData(globalData);
    exit(help ? EXIT_SUCCESS : EXIT_FAILURE);
}

static void parseArguments(int argc, char** argv, Options& options, JSGlobalData* globalData)
{
    int i = 1;
    for (; i < argc; ++i) {
        const char* arg = argv[i];
        if (!strcmp(arg, "-f")) {
            if (++i == argc)
                printUsageStatement(globalData);
            options.scripts.append(Script(true, argv[i]));
            continue;
        }
        if (!strcmp(arg, "-e")) {
            if (++i == argc)
                printUsageStatement(globalData);
            options.scripts.append(Script(false, argv[i]));
            continue;
        }
        if (!strcmp(arg, "-i")) {
            options.interactive = true;
            continue;
        }
        if (!strcmp(arg, "-d")) {
            options.dump = true;
            continue;
        }
        if (!strcmp(arg, "-s")) {
#if HAVE(SIGNAL_H)
            signal(SIGILL, _exit);
            signal(SIGFPE, _exit);
            signal(SIGBUS, _exit);
            signal(SIGSEGV, _exit);
#endif
            continue;
        }
        if (!strcmp(arg, "--")) {
            ++i;
            break;
        }
        if (!strcmp(arg, "-h") || !strcmp(arg, "--help"))
            printUsageStatement(globalData, true);
        options.scripts.append(Script(true, argv[i]));
    }

    if (options.scripts.isEmpty())
        options.interactive = true;

    for (; i < argc; ++i)
        options.arguments.append(argv[i]);
}

int jscmain(int argc, char** argv, JSGlobalData* globalData)
{
    JSLock lock(SilenceAssertionsOnly);

    Options options;
    parseArguments(argc, argv, options, globalData);

    GlobalObject* globalObject = new (globalData) GlobalObject(options.arguments);
    bool success = runWithScripts(globalObject, options.scripts, options.dump);
    if (options.interactive && success)
        runInteractive(globalObject);

    return success ? 0 : 3;
}

static bool fillBufferWithContentsOfFile(const UString& fileName, Vector<char>& buffer)
{
    FILE* f = fopen(fileName.utf8().data(), "r");
    if (!f) {
        fprintf(stderr, "Could not open file: %s\n", fileName.utf8().data());
        return false;
    }

    size_t bufferSize = 0;
    size_t bufferCapacity = 1024;

    buffer.resize(bufferCapacity);

    while (!feof(f) && !ferror(f)) {
        bufferSize += fread(buffer.data() + bufferSize, 1, bufferCapacity - bufferSize, f);
        if (bufferSize == bufferCapacity) { // guarantees space for trailing '\0'
            bufferCapacity *= 2;
            buffer.resize(bufferCapacity);
        }
    }
    fclose(f);
    buffer[bufferSize] = '\0';

    return true;
}

#if PLATFORM(OLYMPIA)

static JSCClient* jscClient = 0;

void jscSetClient(JSCClient* client)
{
    jscClient = client;
}

#undef printf
#undef putchar

static void jscPrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    if (!jscClient) {
        vprintf(format, args);
        va_end(args);
        return;
    }

    Vector<char, 256> buffer;

#if COMPILER(MSVC)

    // Do the format once to get the length.
    int result = _vscprintf(format, args);

    if (result == 0) {
        buffer.append(0);
    } else if (result > 0) {
        unsigned len = result;
        buffer.resize(len + 1);
        vsnprintf(buffer.data(), buffer.size(), format, args);
        buffer[len] = 0;
    }

    va_end(args);
    
    jscClient->output(buffer.data());
#else
    int bufferSize = 256;
    buffer.resize(bufferSize);
    for (;;) {
        int written = vsnprintf(buffer.data(), bufferSize - 1, format, args);
        va_end(args);

        if (written == 0)
            return;
        if (written > 0) {
            buffer[written] = 0;
            jscClient->output(buffer.data());
            return;
        }

        bufferSize <<= 1;
        buffer.resize(bufferSize);
        va_start(args, format);
    }
#endif
}

static void jscPutChar(const char c)
{
    if (!jscClient) {
        putchar(c);
        return;
    }

    char string[2] = { c, 0 };
    jscClient->output(string);
}

EncodedJSValue JSC_HOST_CALL functionAlert(ExecState* exec)
{
    if (jscClient) {
        if (exec->argumentCount()) {
            UString message = exec->argument(0).toString(exec);
            jscClient->alert(message.data(), message.size());
        } else
            jscClient->alert(0, 0);
    }
    return jsNull();
}

EncodedJSValue JSC_HOST_CALL functionConfirm(ExecState* exec)
{
    bool returnValue = false;
    if (jscClient) {
        if (exec->argumentCount()) {
            UString message = exec->argument(0).toString(exec);
            returnValue = jscClient->confirm(message.data(), message.size());
        } else
            returnValue = jscClient->confirm(0, 0);
    }
    return jsBoolean(returnValue);
}

struct JSCPromptCallbackImp: JSCPromptCallback
{
    JSCPromptCallbackImp(UString& r)
        : result(r)
    {
    }

    void setResult(const unsigned short* r, unsigned length)
    {
        result.append(r, length);
    }

    UString& result;
};

EncodedJSValue JSC_HOST_CALL functionPrompt(ExecState* exec)
{
    UString returnValue;
    if (jscClient) {
        JSCPromptCallbackImp callback(returnValue);
        unsigned numArgs = exec->argumentCount();
        const unsigned short* message = 0;
        if (!numArgs) {
            jscClient->prompt(0, 0, 0, 0, &callback);
        } else {
            UString message = exec->argument(0).toString(exec);
            if (numArgs == 1) {
                jscClient->prompt(message.data(), message.size(), 0, 0, &callback);
            } else {
                UString defaultValue = exec->argument(1).toString(exec);
                jscClient->prompt(message.data(), message.size(), defaultValue.data(), defaultValue.size(), &callback);
            }
        }
    }
    return returnValue.isNull() ? jsNull() : jsString(exec, returnValue);
}

#endif
