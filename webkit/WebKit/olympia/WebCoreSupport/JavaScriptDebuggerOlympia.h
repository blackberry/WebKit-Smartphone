/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#ifndef JavaScriptDebuggerOlympia_h
#define JavaScriptDebuggerOlympia_h

#include "ScriptDebugListener.h"

namespace Olympia {
    namespace WebKit {
        class WebPage;
        class WebPageClient;
    }
}

namespace JSC {
    class ExecState;
    class SourceCode;
    class UString;
}

namespace WebCore {

class JavaScriptCallFrame;
class ScriptDebugServer;

class JavaScriptDebuggerOlympia : public ScriptDebugListener {
public:
    JavaScriptDebuggerOlympia(Olympia::WebKit::WebPage* page);
    ~JavaScriptDebuggerOlympia();

    void addBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber, const unsigned short* condition, unsigned conditionLength);
    void updateBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber, const unsigned short* condition, unsigned conditionLength);
    void removeBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber);

    bool pauseOnExceptions();
    void setPauseOnExceptions(bool pause);

    void pauseInDebugger();
    void resumeDebugger();

    void stepOverStatementInDebugger();
    void stepIntoStatementInDebugger();
    void stepOutOfFunctionInDebugger();

    /* From ScriptDebugListener */
    virtual void didParseSource(const String&  sourceID, const String& url, const String& data, int firstLine);
    virtual void failedToParseSource(const String& url, const String& data, int firstLine, int errorLine, const String& errorMessage);
    virtual void didPause(ScriptState*);
    virtual void didContinue();

protected:
    void start();
    void stop();

private:
    Olympia::WebKit::WebPage* m_webPage;
    Olympia::WebKit::WebPageClient* m_pageClient;
    ScriptDebugServer& m_debugServer;

    JavaScriptCallFrame* m_currentCallFrame;
};

} // WebCore

#endif // JavaScriptDebuggerOlympia_h
