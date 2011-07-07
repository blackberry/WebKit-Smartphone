/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef DumpRenderTreeOlympia_h
#define DumpRenderTreeOlympia_h

#include "OlympiaGlobal.h"

#include "PlatformString.h"
#include "Timer.h"
#include "wtf/Vector.h"

namespace WebCore {
class Frame;
class DOMWrapperWorld;
class SecurityOrigin;
}

extern WebCore::Frame* mainFrame;
extern WebCore::Frame* topLoadingFrame;

class AccessibilityController;
class GCController;

namespace Olympia {
namespace WebKit {
class WebPage;

class DumpRenderTree {
public:
    DumpRenderTree(WebPage* page);
    ~DumpRenderTree();

    static DumpRenderTree* currentInstance() { return s_currentInstance; }

    void runTest(const WebCore::String& url);
    void runTests();
    void dump();

    void setWaitToDumpWatchdog(double interval);
    void processWork(WebCore::Timer<DumpRenderTree>*);

    WebCore::String pageGroupName() const;

    // FrameLoaderClient delegates
    void didStartProvisionalLoadForFrame(WebCore::Frame* frame);
    void didCommitLoadForFrame(WebCore::Frame* frame);
    void didFailProvisionalLoadForFrame(WebCore::Frame* frame);
    void didFailLoadForFrame(WebCore::Frame* frame);
    void didFinishLoadForFrame(WebCore::Frame* frame);
    void didClearWindowObjectInWorld(WebCore::DOMWrapperWorld* world);
    void didReceiveTitleForFrame(const WebCore::String& title, WebCore::Frame* frame);

    // ChromeClient delegates
    void addMessageToConsole(const WebCore::String& message, unsigned int lineNumber, const WebCore::String& sourceID);
    void runJavaScriptAlert(const WebCore::String& message);
    bool runJavaScriptConfirm(const WebCore::String& message);
    WebCore::String runJavaScriptPrompt(const WebCore::String& message, const WebCore::String& defaultValue);
    bool runBeforeUnloadConfirmPanel(const WebCore::String& message);
    void setStatusText(const WebCore::String& status);
    void exceededDatabaseQuota(WebCore::SecurityOrigin* origin, const WebCore::String& name);

private:
    static DumpRenderTree* s_currentInstance;

    WebCore::String dumpFramesAsText(WebCore::Frame* frame);
    void locationChangeForFrame(WebCore::Frame* frame);

    void doneDrt();
    void getTestsToRun();
    void resetToConsistentStateBeforeTesting();
    void runRemainingTests();
    void invalidateAnyPreviousWaitToDumpWatchdog();
    void waitToDumpWatchdogTimerFired(WebCore::Timer<DumpRenderTree>*);

    Vector<WebCore::String> m_tests;
    Vector<WebCore::String>::iterator m_currentTest;


    WebCore::String m_resultsDir;
    WebCore::String m_indexFile;
    WebCore::String m_doneFile;
    WebCore::String m_currentTestFile;

    GCController* m_gcController;
    AccessibilityController* m_accessibilityController;
    WebPage* m_page;
    bool m_dumpPixels;
    WebCore::Timer<DumpRenderTree> m_waitToDumpWatchdogTimer;
    WebCore::Timer<DumpRenderTree> m_workTimer;

};
}
}

#endif // DumpRenderTreeOlympia_h
