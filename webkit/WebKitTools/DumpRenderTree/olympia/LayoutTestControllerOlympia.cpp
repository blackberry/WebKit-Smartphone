/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"

#include "CString.h"
#include "DatabaseTracker.h"
#include "Document.h"
#include "DocumentLoader.h"
#include "DumpRenderTree.h"
#include "DumpRenderTreeOlympia.h"
#include "Element.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "KURL.h"
#include "LayoutTestController.h"
#include "NotImplemented.h"
#include "OwnArrayPtr.h"
#include "String.h"
#include "SVGSMILElement.h"
#include "Timer.h"
#include "UnusedParam.h"
#include "WebSettings.h"
#include "WorkQueue.h"
#include "WorkQueueItem.h"
#include "WorkerThread.h"

LayoutTestController::~LayoutTestController()
{
}

void LayoutTestController::addDisallowedURL(JSStringRef url)
{
    UNUSED_PARAM(url);
    notImplemented();
}

void LayoutTestController::clearAllDatabases()
{
#if ENABLE(DATABASE)
    WebCore::DatabaseTracker::tracker(Olympia::WebKit::DumpRenderTree::currentInstance()->pageGroupName()).deleteAllDatabases();
#endif
}

void LayoutTestController::clearBackForwardList()
{
    notImplemented();
}

void LayoutTestController::clearPersistentUserStyleSheet()
{
    notImplemented();
}

JSStringRef LayoutTestController::copyDecodedHostName(JSStringRef name)
{
    UNUSED_PARAM(name);
    notImplemented();
    return 0;
}

JSStringRef LayoutTestController::copyEncodedHostName(JSStringRef name)
{
    UNUSED_PARAM(name);
    notImplemented();
    return 0;
}

void LayoutTestController::dispatchPendingLoadRequests()
{
    notImplemented();
}

void LayoutTestController::display()
{
    notImplemented();
}

static WebCore::String jsStringRefToWebCoreString(JSStringRef str)
{
    size_t strArrSize = JSStringGetMaximumUTF8CStringSize(str);
    OwnArrayPtr<char> strArr(new char[strArrSize]);
    JSStringGetUTF8CString(str, strArr.get(), strArrSize);
    return WebCore::String(strArr.get());
}

void LayoutTestController::execCommand(JSStringRef name, JSStringRef value)
{
    if (!mainFrame)
        return;

    WebCore::String nameStr = jsStringRefToWebCoreString(name);
    WebCore::String valueStr = jsStringRefToWebCoreString(value);

    mainFrame->editor()->command(nameStr).execute(valueStr);
}

bool LayoutTestController::isCommandEnabled(JSStringRef name)
{
    UNUSED_PARAM(name);
    notImplemented();
    return false;
}

void LayoutTestController::keepWebHistory()
{
    notImplemented();
}

void LayoutTestController::notifyDone()
{
    if (m_waitToDump && !topLoadingFrame && !WorkQueue::shared()->count())
        dump();

    m_waitToDump = false;
}

JSStringRef LayoutTestController::pathToLocalResource(JSContextRef, JSStringRef url)
{
    UNUSED_PARAM(url);
    notImplemented();
    return 0;
}

void LayoutTestController::queueLoad(JSStringRef url, JSStringRef target)
{
    size_t urlArrSize = JSStringGetMaximumUTF8CStringSize(url);
    OwnArrayPtr<char> urlArr(new char[urlArrSize]);
    JSStringGetUTF8CString(url, urlArr.get(), urlArrSize);

    WebCore::KURL base = mainFrame->loader()->documentLoader()->response().url();
    WebCore::KURL absolute(base, urlArr.get());

    JSRetainPtr<JSStringRef> absoluteURL(Adopt, JSStringCreateWithUTF8CString(absolute.string().utf8().data()));
    WorkQueue::shared()->queue(new LoadItem(absoluteURL.get(), target));
}

void LayoutTestController::setAcceptsEditing(bool acceptsEditing)
{
    Olympia::WebKit::DumpRenderTree::currentInstance()->setAcceptsEditing(acceptsEditing);
}

void LayoutTestController::setAppCacheMaximumSize(unsigned long long quota)
{
    UNUSED_PARAM(quota);
    notImplemented();
}

void LayoutTestController::setAuthorAndUserStylesEnabled(bool)
{
    notImplemented();
}

void LayoutTestController::setCacheModel(int)
{
    notImplemented();
}

void LayoutTestController::setCustomPolicyDelegate(bool setDelegate, bool permissive)
{
    UNUSED_PARAM(setDelegate);
    UNUSED_PARAM(permissive);
    notImplemented();
}

void LayoutTestController::setDatabaseQuota(unsigned long long quota)
{
    if (!mainFrame)
        return;

    WebCore::DatabaseTracker::tracker(Olympia::WebKit::DumpRenderTree::currentInstance()->pageGroupName()).setQuota(mainFrame->document()->securityOrigin(), quota);
}

void LayoutTestController::setDomainRelaxationForbiddenForURLScheme(bool, JSStringRef)
{
    notImplemented();
}

void LayoutTestController::setIconDatabaseEnabled(bool iconDatabaseEnabled)
{
    UNUSED_PARAM(iconDatabaseEnabled);
    notImplemented();
}

void LayoutTestController::setJavaScriptProfilingEnabled(bool profilingEnabled)
{
    UNUSED_PARAM(profilingEnabled);
    notImplemented();
}

void LayoutTestController::setMainFrameIsFirstResponder(bool flag)
{
    UNUSED_PARAM(flag);
    notImplemented();
}

void LayoutTestController::setPersistentUserStyleSheetLocation(JSStringRef path)
{
    UNUSED_PARAM(path);
    notImplemented();
}

void LayoutTestController::setPopupBlockingEnabled(bool flag)
{
    UNUSED_PARAM(flag);
    notImplemented();
}

void LayoutTestController::setPrivateBrowsingEnabled(bool flag)
{
    UNUSED_PARAM(flag);
    notImplemented();
}

void LayoutTestController::setXSSAuditorEnabled(bool flag)
{
    UNUSED_PARAM(flag);
    notImplemented();
}

void LayoutTestController::setSelectTrailingWhitespaceEnabled(bool flag)
{
    UNUSED_PARAM(flag);
    notImplemented();
}

void LayoutTestController::setSmartInsertDeleteEnabled(bool flag)
{
    UNUSED_PARAM(flag);
    notImplemented();
}

void LayoutTestController::setTabKeyCyclesThroughElements(bool cycles)
{
    UNUSED_PARAM(cycles);
    notImplemented();
}

void LayoutTestController::setUseDashboardCompatibilityMode(bool flag)
{
    UNUSED_PARAM(flag);
    notImplemented();
}

void LayoutTestController::setUserStyleSheetEnabled(bool flag)
{
    UNUSED_PARAM(flag);
    notImplemented();
}

void LayoutTestController::setUserStyleSheetLocation(JSStringRef path)
{
    UNUSED_PARAM(path);
    notImplemented();
}

void LayoutTestController::waitForPolicyDelegate()
{
    notImplemented();
}

size_t LayoutTestController::webHistoryItemCount()
{
    notImplemented();
    return 0;
}

int LayoutTestController::windowCount()
{
    notImplemented();
    return 0;
}

bool LayoutTestController::elementDoesAutoCompleteForElementWithId(JSStringRef id)
{
    UNUSED_PARAM(id);
    notImplemented();
    return false;
}

void LayoutTestController::setWaitToDump(bool waitToDump)
{
    static const double kWaitToDumpWatchdogInterval = 30.0;
    m_waitToDump = waitToDump;
    if (m_waitToDump)
        Olympia::WebKit::DumpRenderTree::currentInstance()->setWaitToDumpWatchdog(kWaitToDumpWatchdogInterval);
}

void LayoutTestController::setWindowIsKey(bool windowIsKey)
{
    m_windowIsKey = windowIsKey;
    notImplemented();
}

bool LayoutTestController::pauseAnimationAtTimeOnElementWithId(JSStringRef animationName, double time, JSStringRef elementId)
{
    if (!mainFrame)
        return false;

    int nameLen = JSStringGetMaximumUTF8CStringSize(animationName);
    int idLen = JSStringGetMaximumUTF8CStringSize(elementId);
    OwnArrayPtr<char> name(new char[nameLen]);
    OwnArrayPtr<char> eId(new char[idLen]);

    JSStringGetUTF8CString(animationName, name.get(), nameLen);
    JSStringGetUTF8CString(elementId, eId.get(), idLen);

    WebCore::AnimationController* animationController = mainFrame->animation();
    if (!animationController)
        return false;

    WebCore::Node* node = mainFrame->document()->getElementById(eId.get());
    if (!node || !node->renderer())
        return false;

    return animationController->pauseAnimationAtTime(node->renderer(), name.get(), time);
}

bool LayoutTestController::pauseTransitionAtTimeOnElementWithId(JSStringRef propertyName, double time, JSStringRef elementId)
{
    if (!mainFrame)
        return false;

    int nameLen = JSStringGetMaximumUTF8CStringSize(propertyName);
    int idLen = JSStringGetMaximumUTF8CStringSize(elementId);
    OwnArrayPtr<char> name(new char[nameLen]);
    OwnArrayPtr<char> eId(new char[idLen]);

    JSStringGetUTF8CString(propertyName, name.get(), nameLen);
    JSStringGetUTF8CString(elementId, eId.get(), idLen);

    WebCore::AnimationController* animationController = mainFrame->animation();
    if (!animationController)
        return false;

    WebCore::Node* node = mainFrame->document()->getElementById(eId.get());
    if (!node || !node->renderer())
        return false;

    return animationController->pauseTransitionAtTime(node->renderer(), name.get(), time);
}

unsigned LayoutTestController::numberOfActiveAnimations() const
{
    if (!mainFrame)
        return false;

    WebCore::AnimationController* animationController = mainFrame->animation();
    if (!animationController)
        return false;

    return animationController->numberOfActiveAnimations();
}

unsigned int LayoutTestController::workerThreadCount() const
{
#if ENABLE_WORKERS
    return WebCore::WorkerThread::workerThreadCount();
#else
    return 0;
#endif
}

void LayoutTestController::removeAllVisitedLinks()
{
    notImplemented();
}

void LayoutTestController::disableImageLoading()
{
    Olympia::WebKit::WebSettings::pageGroupSettings(Olympia::WebKit::DumpRenderTree::currentInstance()->pageGroupName())->setLoadsImagesAutomatically(false);
}

JSRetainPtr<JSStringRef> LayoutTestController::counterValueForElementById(JSStringRef id)
{
    notImplemented();
    return 0;
}

void LayoutTestController::overridePreference(JSStringRef key, JSStringRef value)
{
    notImplemented();
}

void LayoutTestController::setAlwaysAcceptCookies(bool alwaysAcceptCookies)
{
    notImplemented();
}

void LayoutTestController::setMockGeolocationPosition(double latitude, double longitude, double accuracy)
{
    notImplemented();
}

void LayoutTestController::setMockGeolocationError(int code, JSStringRef message)
{
    notImplemented();
}

void LayoutTestController::setAllowUniversalAccessFromFileURLs(bool enabled)
{
    notImplemented();
}

void LayoutTestController::setTimelineProfilingEnabled(bool enabled)
{
    notImplemented();
}

void LayoutTestController::showWebInspector()
{
    notImplemented();
}

void LayoutTestController::closeWebInspector()
{
    notImplemented();
}

void LayoutTestController::evaluateInWebInspector(long callId, JSStringRef script)
{
    notImplemented();
}

void LayoutTestController::evaluateScriptInIsolatedWorld(unsigned worldID, JSObjectRef globalObject, JSStringRef script)
{
    notImplemented();
}

void LayoutTestController::addUserScript(JSStringRef source, bool runAtStart)
{
    notImplemented();
}

void LayoutTestController::addUserStyleSheet(JSStringRef source)
{
    notImplemented();
}

bool LayoutTestController::sampleSVGAnimationForElementAtTime(JSStringRef animationId, double time, JSStringRef elementId)
{
#if ENABLE(SVG_ANIMATION)
    if (!mainFrame)
        return false;

    int aLen = JSStringGetMaximumUTF8CStringSize(animationId);
    int eLen = JSStringGetMaximumUTF8CStringSize(elementId);
    OwnArrayPtr<char> aId(new char[aLen]);
    OwnArrayPtr<char> eId(new char[eLen]);

    JSStringGetUTF8CString(animationId, aId.get(), aLen);
    JSStringGetUTF8CString(elementId, eId.get(), eLen);

    WebCore::Document* document = mainFrame->document();
    if (!document || !document->svgExtensions())
        return false;

    WebCore::Node* node = mainFrame->document()->getElementById(aId.get());
    if (!node || !WebCore::SVGSMILElement::isSMILElement(node))
        return false;

    return document->accessSVGExtensions()->sampleAnimationAtTime(eId.get(), static_cast<WebCore::SVGSMILElement*>(node), time);
#else
    return false;
#endif
}

int LayoutTestController::pageNumberForElementById(JSStringRef, float, float)
{
    notImplemented();
    return -1;
}

int LayoutTestController::numberOfPages(float, float)
{
    notImplemented();
    return -1;
}

void LayoutTestController::setScrollbarPolicy(JSStringRef orientation, JSStringRef policy)
{
    notImplemented();
}

void LayoutTestController::setWebViewEditable(bool)
{
    notImplemented();
}

void LayoutTestController::authenticateSession(JSStringRef url, JSStringRef username, JSStringRef password)
{
    notImplemented();
}

bool LayoutTestController::callShouldCloseOnWebView()
{
    notImplemented();
    return false;
}

void LayoutTestController::setFrameFlatteningEnabled(bool)
{
    notImplemented();
}

void LayoutTestController::setSpatialNavigationEnabled(bool enable)
{
    notImplemented();
}

void LayoutTestController::addOriginAccessWhitelistEntry(JSStringRef sourceOrigin, JSStringRef destinationProtocol, JSStringRef destinationHost, bool allowDestinationSubdomains)
{
    notImplemented();
}

void LayoutTestController::removeOriginAccessWhitelistEntry(JSStringRef sourceOrigin, JSStringRef destinationProtocol, JSStringRef destinationHost, bool allowDestinationSubdomains)
{
    notImplemented();
}

void LayoutTestController::setAllowFileAccessFromFileURLs(bool)
{
    notImplemented();
}

void LayoutTestController::apiTestNewWindowDataLoadBaseURL(JSStringRef utf8Data, JSStringRef baseURL)
{
    notImplemented();
}

void LayoutTestController::apiTestGoToCurrentBackForwardItem()
{
    notImplemented();
}

void LayoutTestController::setJavaScriptCanAccessClipboard(bool flag)
{
    notImplemented();
}

JSValueRef LayoutTestController::computedStyleIncludingVisitedInfo(JSContextRef, JSValueRef)
{
    notImplemented();
    return 0;
}

JSRetainPtr<JSStringRef> LayoutTestController::layerTreeAsText() const
{
    notImplemented();
    return 0;
}

JSRetainPtr<JSStringRef> LayoutTestController::markerTextForListItem(JSContextRef context, JSValueRef nodeObject) const
{
    notImplemented();
    return 0;
}


void LayoutTestController::setPluginsEnabled(bool flag)
{
    notImplemented();
}


void LayoutTestController::setEditingBehavior(const char* editingBehavior)
{
    notImplemented();
}
