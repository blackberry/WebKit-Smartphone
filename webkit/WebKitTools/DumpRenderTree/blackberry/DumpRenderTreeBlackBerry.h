/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef DumpRenderTreeOlympia_h
#define DumpRenderTreeOlympia_h

#include "BlackBerryGlobal.h"

#include "PlatformString.h"
#include "Timer.h"
#include "wtf/Vector.h"

namespace WebCore {
enum EAffinity;
class Frame;
class DOMWrapperWorld;
class Range;
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

    void runTest(const WTF::String& url);
    void runTests();
    void dump();

    void setWaitToDumpWatchdog(double interval);
    void processWork(WebCore::Timer<DumpRenderTree>*);

    WebPage* page() { return m_page; }
    WTF::String pageGroupName() const;

    // FrameLoaderClient delegates
    void didStartProvisionalLoadForFrame(WebCore::Frame* frame);
    void didCommitLoadForFrame(WebCore::Frame* frame);
    void didFailProvisionalLoadForFrame(WebCore::Frame* frame);
    void didFailLoadForFrame(WebCore::Frame* frame);
    void didFinishLoadForFrame(WebCore::Frame* frame);
    void didFinishDocumentLoadForFrame(WebCore::Frame* frame);
    void didClearWindowObjectInWorld(WebCore::DOMWrapperWorld* world);
    void didReceiveTitleForFrame(const WTF::String& title, WebCore::Frame* frame);

    // ChromeClient delegates
    void addMessageToConsole(const WTF::String& message, unsigned int lineNumber, const WTF::String& sourceID);
    void runJavaScriptAlert(const WTF::String& message);
    bool runJavaScriptConfirm(const WTF::String& message);
    WTF::String runJavaScriptPrompt(const WTF::String& message, const WTF::String& defaultValue);
    bool runBeforeUnloadConfirmPanel(const WTF::String& message);
    void setStatusText(const WTF::String& status);
    void exceededDatabaseQuota(WebCore::SecurityOrigin* origin, const WTF::String& name);

    // EditorClient delegates
    void setAcceptsEditing(bool acceptsEditing) { m_acceptsEditing = acceptsEditing; }

    void didBeginEditing();
    void didEndEditing();
    void didChange();
    void didChangeSelection();
    bool shouldBeginEditingInDOMRange(WebCore::Range* range);
    bool shouldEndEditingInDOMRange(WebCore::Range* range);
    bool shouldDeleteDOMRange(WebCore::Range* range);
    bool shouldChangeSelectedDOMRangeToDOMRangeAffinityStillSelecting(WebCore::Range* fromRange, WebCore::Range* toRange, WebCore::EAffinity affinity, bool stillSelecting);

private:
    static DumpRenderTree* s_currentInstance;

    WTF::String dumpFramesAsText(WebCore::Frame* frame);
    void locationChangeForFrame(WebCore::Frame* frame);

    void doneDrt();
    void getTestsToRun();
    void resetToConsistentStateBeforeTesting();
    void runRemainingTests();
    void invalidateAnyPreviousWaitToDumpWatchdog();
    void waitToDumpWatchdogTimerFired(WebCore::Timer<DumpRenderTree>*);

    Vector<WTF::String> m_tests;
    Vector<WTF::String>::iterator m_currentTest;


    WTF::String m_resultsDir;
    WTF::String m_indexFile;
    WTF::String m_doneFile;
    WTF::String m_currentTestFile;

    GCController* m_gcController;
    AccessibilityController* m_accessibilityController;
    WebPage* m_page;
    bool m_dumpPixels;
    WebCore::Timer<DumpRenderTree> m_waitToDumpWatchdogTimer;
    WebCore::Timer<DumpRenderTree> m_workTimer;

    bool m_acceptsEditing;
};
}
}

#endif // DumpRenderTreeOlympia_h
