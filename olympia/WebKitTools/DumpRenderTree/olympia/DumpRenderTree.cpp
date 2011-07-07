/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include <OlympiaPlatformMemory.h>
#include "config.h"

#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "AccessibilityController.h"
#include "CString.h"
#include "DatabaseTracker.h"
#include "DocumentLoader.h"
#include "DumpRenderTree.h"
#include "Element.h"
#include "EventSender.h"
#include "Frame.h"
#include "FrameTree.h"
#include "GCController.h"
#include "GCControllerOlympia.h"
#include "IntSize.h"
#include "LayoutTestController.h"
#include "NotImplemented.h"
#include <OlympiaPlatformText.h>
#include "OwnArrayPtr.h"
#include "SecurityOrigin.h"
#include "Timer.h"
#include "Vector.h"
#include "WebPage.h"
#include "WebPage_p.h"
#include "WebPageClient.h"
#include "WebSettings.h"
#include "WorkQueue.h"
#include "WorkQueueItem.h"

volatile bool done;

LayoutTestController* gLayoutTestController = 0;

WebCore::Frame* mainFrame = 0;
WebCore::Frame* topLoadingFrame = 0;

static const char* const kSDCLayoutTestsURI = "file:///SDCard/LayoutTests/";

using namespace std;

static WebCore::String drtFrameDescription(WebCore::Frame* frame)
{
    WebCore::String name = frame->tree()->name().string();
    if (frame == mainFrame) {
        if (!name.isNull() && name.length())
            return WebCore::String::format("main frame \"%s\"", name.utf8().data());
        else
            return "main frame";
    } else {
        if (!name.isNull())
            return WebCore::String::format("frame \"%s\"", name.utf8().data());
        else
            return "frame (anonymous)";
    }
}

namespace Olympia {
namespace WebKit {

DumpRenderTree* DumpRenderTree::s_currentInstance = 0;

static void createFile(const WebCore::String& fileName)
{
    FILE* fd = fopen(fileName.utf8().data(), "wb");
    fclose(fd);
}

DumpRenderTree::DumpRenderTree(Olympia::WebKit::WebPage* page)
    : m_gcController(0)
    , m_accessibilityController(0)
    , m_page(page)
    , m_dumpPixels(false)
    , m_waitToDumpWatchdogTimer(this, &DumpRenderTree::waitToDumpWatchdogTimerFired)
    , m_workTimer(this, &DumpRenderTree::processWork)
{
    WebCore::String sdcardPath = "/media/usd";
    m_resultsDir = sdcardPath + "/results/";
    m_indexFile = sdcardPath + "/index.drt";
    m_doneFile = sdcardPath + "/done";
    m_currentTestFile = sdcardPath + "/current.drt";
    m_page->resetVirtualViewportOnCommitted(false);
    m_page->setVirtualViewportSize(800, 600);
    s_currentInstance = this;
}

DumpRenderTree::~DumpRenderTree()
{
    delete m_gcController;
    delete m_accessibilityController;
}

void DumpRenderTree::runTest(const WebCore::String& url)
{
    createFile(m_resultsDir + *m_currentTest + ".dump.crash");

    mainFrame->loader()->stopForUserCancel();
    resetToConsistentStateBeforeTesting();
    WebCore::String stdoutFile = m_resultsDir + *m_currentTest + ".dump";
    WebCore::String stderrFile = m_resultsDir + *m_currentTest + ".stderr";

    // XXX we should preserve the original stdout and stderr here but aren't doing
    // that yet due to issues with dup, etc.
    freopen(stdoutFile.utf8().data(), "wb", stdout);
    freopen(stderrFile.utf8().data(), "wb", stderr);

    FILE* current = fopen(m_currentTestFile.utf8().data(), "w");
    fwrite(m_currentTest->utf8().data(), 1, m_currentTest->utf8().length(), current);
    fclose(current);
    m_page->load(url.utf8().data(), false);
}

void DumpRenderTree::doneDrt()
{
    // Notify the external world that we're done
    createFile(m_doneFile);
    (m_page->client())->notifyRunLayoutTestsFinished();
}

void DumpRenderTree::runRemainingTests()
{
    // XXX: fflush should not be necessary but is temporarily required due to a bug in stdio
    // output
    fflush(stdout);
    fflush(stderr);

    if (m_currentTest < m_tests.end() - 1) {
        m_currentTest++;
        runTest(kSDCLayoutTestsURI + *m_currentTest);
    } else {
        doneDrt();
    }
}

void DumpRenderTree::resetToConsistentStateBeforeTesting()
{
    if (gLayoutTestController) {
        gLayoutTestController->deref();
        gLayoutTestController = 0;
    }

    gLayoutTestController = new LayoutTestController((kSDCLayoutTestsURI + *m_currentTest).utf8().data(), "");
    ASSERT(gLayoutTestController);

    topLoadingFrame = 0;

    done = false;
    WorkQueue::shared()->clear();
    WorkQueue::shared()->setFrozen(false);

    WebSettings* settings = WebSettings::globalSettings();

    settings->setTextReflowMode(WebSettings::TextReflowDisabled);
    settings->setJavaScriptEnabled(true);
    settings->setLoadsImagesAutomatically(true);
    settings->setJavaScriptOpenWindowsAutomatically(true);

    m_page->setVirtualViewportSize(800, 600);
    m_page->resetVirtualViewportOnCommitted(false);
    m_page->d->setUserScalable(false);

    // For now we manually garbage collect between each test to make sure the device won't run out of memory due to lazy collection
    WebCore::gcController().garbageCollectNow();
}

void DumpRenderTree::runTests()
{
    WebCore::String fontName("Ahem");
    WebCore::String fontPath("/media/usd/fonts/AHEM____.TTF");
    Olympia::Platform::Text::FontDataId fontDataId;
    Olympia::Platform::Text::engine()->loadFontData(fontDataId, fontPath.charactersWithNullTermination(), fontName.charactersWithNullTermination());

    m_gcController = new GCController();
    m_accessibilityController = new AccessibilityController();
    getTestsToRun();

    mainFrame = m_page->d->m_mainFrame;

    m_currentTest = m_tests.begin();

    if (m_currentTest == m_tests.end()) {
        doneDrt();
        return;
    }

    runTest(kSDCLayoutTestsURI + *m_currentTest);
}


WebCore::String DumpRenderTree::dumpFramesAsText(WebCore::Frame* frame)
{
    WebCore::String s;
    WebCore::Element* documentElement = frame->document()->documentElement();
    if (!documentElement)
        return s.utf8().data();

    if (frame->tree()->parent())
        s = WebCore::String::format("\n-------\nFrame: '%s'\n--------\n", frame->tree()->name().string().utf8().data());

    s += documentElement->innerText() + "\n";

    if (gLayoutTestController->dumpChildFramesAsText()) {
        WebCore::FrameTree* tree = frame->tree();
        for (WebCore::Frame* child = tree->firstChild(); child; child = child->tree()->nextSibling()) {
            s += dumpFramesAsText(child);
        }
    }
    return s;
}

static void dumpToFile(const WebCore::String& data)
{
    fwrite(data.utf8().data(), 1, data.utf8().length(), stdout);
}

void DumpRenderTree::getTestsToRun()
{
    Vector<WebCore::String> files;

    FILE* fd = fopen(m_indexFile.utf8().data(), "r");
    fseek(fd, 0, SEEK_END);
    int size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    OwnArrayPtr<char> buf(new char[size]);
    fread(buf.get(), 1, size, fd);
    fclose(fd);
    WebCore::String s(buf.get(), size);
    s.split("\n", files);

    m_tests = files;
}

void DumpRenderTree::invalidateAnyPreviousWaitToDumpWatchdog()
{
    m_waitToDumpWatchdogTimer.stop();
}

void DumpRenderTree::dump()
{
    invalidateAnyPreviousWaitToDumpWatchdog();

    WebCore::String dumpFile = m_resultsDir + *m_currentTest + ".dump";

    WebCore::String resultMimeType = "text/plain";
    WebCore::String responseMimeType = mainFrame->loader()->documentLoader()->responseMIMEType();

    bool dumpAsText = gLayoutTestController->dumpAsText() || responseMimeType == "text/plain";
    WebCore::String data =  dumpAsText ? dumpFramesAsText(mainFrame) : m_page->d->renderTreeDump();

    WebCore::String result = "Content-Type: " + resultMimeType + "\n" + data;

    dumpToFile(result);
    // FIXME: Pixel tests not yet implemented
    if (m_dumpPixels && !dumpAsText)
        ;// Run pixel tests

    invalidateAnyPreviousWaitToDumpWatchdog();

    WebCore::String crashFile = dumpFile + ".crash";
    unlink(crashFile.utf8().data());
    done = true;
    runRemainingTests();
}

void DumpRenderTree::setWaitToDumpWatchdog(double interval)
{
    if (!m_waitToDumpWatchdogTimer.isActive())
        m_waitToDumpWatchdogTimer.startOneShot(interval);
}

void DumpRenderTree::waitToDumpWatchdogTimerFired(WebCore::Timer<DumpRenderTree>*)
{
    gLayoutTestController->waitToDumpWatchdogTimerFired();
}

void DumpRenderTree::processWork(WebCore::Timer<DumpRenderTree>*)
{
    if (topLoadingFrame)
        return;

    if (WorkQueue::shared()->processWork() && !gLayoutTestController->waitToDump())
        dump();
}

void DumpRenderTree::locationChangeForFrame(WebCore::Frame* frame)
{
    if (frame != topLoadingFrame)
        return;

    topLoadingFrame = 0;
    WorkQueue::shared()->setFrozen(true); // first complete load freezes the queue
    if (gLayoutTestController->waitToDump())
        return;

    if (WorkQueue::shared()->count())
        m_workTimer.startOneShot(0);
    else
        dump();
}

// FrameLoadClient delegates
void DumpRenderTree::didStartProvisionalLoadForFrame(WebCore::Frame* frame)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks())
        printf("%s - didStartProvisionalLoadForFrame\n", drtFrameDescription(frame).utf8().data());

    if (!topLoadingFrame && !done)
        topLoadingFrame = frame;

    if (!done && gLayoutTestController->stopProvisionalFrameLoads()) {
        printf("%s - stopping load in didStartProvisionalLoadForFrame callback\n", drtFrameDescription(frame).utf8().data());
        frame->loader()->stopForUserCancel();
    }
}

void DumpRenderTree::didCommitLoadForFrame(WebCore::Frame* frame)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks())
        printf("%s - didCommitLoadForFrame\n", drtFrameDescription(frame).utf8().data());

    gLayoutTestController->setWindowIsKey(true);
}

void DumpRenderTree::didFailProvisionalLoadForFrame(WebCore::Frame* frame)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks())
        printf("%s - didFailProvisionalLoadWithError\n", drtFrameDescription(frame).utf8().data());

    locationChangeForFrame(frame);
}

void DumpRenderTree::didFailLoadForFrame(WebCore::Frame* frame)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks())
        printf("%s - didFailLoadWithError\n", drtFrameDescription(frame).utf8().data());

    locationChangeForFrame(frame);
}

void DumpRenderTree::didFinishLoadForFrame(WebCore::Frame* frame)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks())
        printf("%s - didFinishLoadForFrame\n", drtFrameDescription(frame).utf8().data());

    locationChangeForFrame(frame);
}

void DumpRenderTree::didClearWindowObjectInWorld(WebCore::DOMWrapperWorld* world)
{
    JSValueRef exception = 0;
    ASSERT(gLayoutTestController);

    JSGlobalContextRef context = (JSGlobalContextRef)m_page->scriptContext();
    JSObjectRef windowObject = (JSObjectRef)m_page->windowObject();

    gLayoutTestController->makeWindowObject(context, windowObject, &exception);
    ASSERT(!exception);

    m_gcController->makeWindowObject(context, windowObject, &exception);
    ASSERT(!exception);

    m_accessibilityController->makeWindowObject(context, windowObject, &exception);
    ASSERT(!exception);

    JSStringRef eventSenderStr = JSStringCreateWithUTF8CString("eventSender");
    JSValueRef eventSender = makeEventSender(context);
    JSObjectSetProperty(context, windowObject, eventSenderStr, eventSender, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, 0);
    JSStringRelease(eventSenderStr);
}

void DumpRenderTree::didReceiveTitleForFrame(const WebCore::String& title, WebCore::Frame* frame)
{
    if (!done && gLayoutTestController->dumpFrameLoadCallbacks())
        printf("%s - didReceiveTitle: %s\n", drtFrameDescription(frame).utf8().data(), title.utf8().data());

    if (gLayoutTestController->dumpTitleChanges())
        printf("TITLE CHANGED: %s\n", title.utf8().data());
}

// ChromeClient delegates
void DumpRenderTree::addMessageToConsole(const WebCore::String& message, unsigned int lineNumber, const WebCore::String& sourceID)
{
    printf("CONSOLE MESSAGE: line %d: %s\n", lineNumber, message.utf8().data());
}

void DumpRenderTree::runJavaScriptAlert(const WebCore::String& message)
{
    if (!done)
        printf("ALERT: %s\n", message.utf8().data());
}

bool DumpRenderTree::runJavaScriptConfirm(const WebCore::String& message)
{
    if (!done)
        printf("CONFIRM: %s\n", message.utf8().data());
    return true;
}

WebCore::String DumpRenderTree::runJavaScriptPrompt(const WebCore::String& message, const WebCore::String& defaultValue)
{
    if (!done)
        printf("PROMPT: %s, default text: %s\n", message.utf8().data(), defaultValue.utf8().data());
    return defaultValue;
}

bool DumpRenderTree::runBeforeUnloadConfirmPanel(const WebCore::String& message)
{
    if (!done)
        printf("CONFIRM NAVIGATION: %s\n", message.utf8().data());
    return true;
}

void DumpRenderTree::setStatusText(const WebCore::String& status)
{
    if (gLayoutTestController->dumpStatusCallbacks())
        printf("UI DELEGATE STATUS CALLBACK: setStatusText:%s\n", status.utf8().data());
}

void DumpRenderTree::exceededDatabaseQuota(WebCore::SecurityOrigin* origin, const WebCore::String& name)
{
    if (!done && gLayoutTestController->dumpDatabaseCallbacks())
        printf("UI DELEGATE DATABASE CALLBACK: exceededDatabaseQuotaForSecurityOrigin:{%s, %s, %i} database:%s\n", origin->protocol().utf8().data(), origin->host().utf8().data(), origin->port(), name.utf8().data());

    WebCore::DatabaseTracker::tracker(pageGroupName()).setQuota(mainFrame->document()->securityOrigin(), 5 * 1024 * 1024);
}

WebCore::String DumpRenderTree::pageGroupName() const
{
   ASSERT(m_page);
   return m_page->settings()->pageGroupName();
}

}
}

// Static dump() function required by cross-platform DRT code

void dump()
{
    Olympia::WebKit::DumpRenderTree* dumper = Olympia::WebKit::DumpRenderTree::currentInstance();
    if (!dumper)
        return;

    dumper->dump();
}
