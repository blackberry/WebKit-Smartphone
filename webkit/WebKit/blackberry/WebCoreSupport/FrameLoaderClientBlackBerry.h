/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef FrameLoaderClientOlympia_h
#define FrameLoaderClientOlympia_h

#include "FrameLoaderClient.h"

#include "NotImplemented.h"
#include "DocumentLoader.h"
#include "FormState.h"
#include "HTMLFormElement.h"
#include "Frame.h"
#include "ResourceError.h"
#include "ResourceRequest.h"
#include "Timer.h"
#include "ViewportArguments.h"
#include "Widget.h"

namespace Olympia {
    namespace WebKit {
        class WebPage;
        class WebPlugin;
        class WebPluginClient;
    }
}

namespace WebCore {

class FrameNetworkingContext;
class PluginWidget;

class FrameLoaderClientBlackBerry : public FrameLoaderClient {
public:
    FrameLoaderClientBlackBerry();
    ~FrameLoaderClientBlackBerry();

    void setFrame(Frame* frame, Olympia::WebKit::WebPage* webPage) { m_frame = frame; m_webPage = webPage; }

    int playerId() const;
    PassRefPtr<Olympia::WebKit::WebPlugin> getCachedPlugin(HTMLElement* element);
    void cancelLoadingPlugin(int id);
    void cleanPluginList();
    void mediaReadyStateChanged(int id, int state);
    void mediaVolumeChanged(int id, int volume);
    void mediaDurationChanged(int id, float duration);

    virtual void frameLoaderDestroyed();
    virtual bool hasWebView() const { return true; }
    virtual void makeRepresentation(DocumentLoader*) { notImplemented(); }
    virtual void forceLayout() { notImplemented(); }
    virtual void forceLayoutForNonHTML() { notImplemented(); }
    virtual void setCopiesOnScroll() { notImplemented(); }
    virtual void detachedFromParent2();
    virtual void detachedFromParent3() { notImplemented(); }
    virtual void assignIdentifierToInitialRequest(long unsigned int, DocumentLoader*, const ResourceRequest&) { notImplemented(); }
    virtual void dispatchWillSendRequest(DocumentLoader*, long unsigned int, ResourceRequest&, const ResourceResponse&);
    virtual bool shouldUseCredentialStorage(DocumentLoader*, long unsigned int) { notImplemented(); return false; }
    virtual void dispatchDidReceiveAuthenticationChallenge(DocumentLoader*, long unsigned int, const AuthenticationChallenge&) { notImplemented(); }
    virtual void dispatchDidCancelAuthenticationChallenge(DocumentLoader*, long unsigned int, const AuthenticationChallenge&) { notImplemented(); }
    virtual void dispatchDidReceiveResponse(DocumentLoader*, long unsigned int, const ResourceResponse&) { notImplemented(); }
    virtual void dispatchDidReceiveContentLength(DocumentLoader*, long unsigned int, int) { notImplemented(); }
    virtual void dispatchDidFinishLoading(DocumentLoader*, long unsigned int) { notImplemented(); }
    virtual void dispatchDidFailLoading(DocumentLoader*, long unsigned int, const ResourceError&) { notImplemented(); }
    virtual bool dispatchDidLoadResourceFromMemoryCache(DocumentLoader*, const ResourceRequest&, const ResourceResponse&, int) { notImplemented(); return false; }
    virtual void dispatchDidLoadResourceByXMLHttpRequest(unsigned long identifier, const ScriptString&) { notImplemented(); }
    virtual void dispatchDidHandleOnloadEvents() { notImplemented(); }
    virtual void dispatchDidReceiveServerRedirectForProvisionalLoad() { notImplemented(); }
    virtual void dispatchDidCancelClientRedirect();
    virtual void dispatchWillPerformClientRedirect(const KURL&, double, double);
    virtual void dispatchDidChangeLocationWithinPage();
    virtual void dispatchDidPushStateWithinPage() { notImplemented(); }
    virtual void dispatchDidReplaceStateWithinPage() { notImplemented(); }
    virtual void dispatchDidPopStateWithinPage() { notImplemented(); }
    virtual void dispatchWillClose();
    virtual void dispatchDidReceiveIcon() { notImplemented(); }
    virtual void dispatchDidStartProvisionalLoad();
    virtual void dispatchDidReceiveTitle(const String&);
    virtual void dispatchDidCommitLoad();
    virtual void dispatchDidFailProvisionalLoad(const ResourceError&);
    virtual void dispatchDidFailLoad(const ResourceError&);
    virtual void dispatchDidFinishDocumentLoad();
    virtual void dispatchDidFinishLoad();
    virtual void dispatchDidFirstLayout() { notImplemented(); }
    virtual void dispatchDidFirstVisuallyNonEmptyLayout();
    virtual Frame* dispatchCreatePage();
    virtual void dispatchShow() { notImplemented(); }
    virtual void dispatchDecidePolicyForMIMEType(FramePolicyFunction, const String& MIMEType, const ResourceRequest&);
    virtual void dispatchDecidePolicyForNewWindowAction(FramePolicyFunction, const NavigationAction&, const ResourceRequest&, PassRefPtr<FormState>, const String& frameName);
    virtual void dispatchDecidePolicyForNavigationAction(FramePolicyFunction, const NavigationAction&, const ResourceRequest&, PassRefPtr<FormState>);
    virtual void cancelPolicyCheck();
    virtual void dispatchUnableToImplementPolicy(const ResourceError&) { notImplemented(); }
    virtual void dispatchWillSubmitForm(FramePolicyFunction, PassRefPtr<FormState>);
    virtual void dispatchDidLoadMainResource(DocumentLoader*) { notImplemented(); }
    virtual void revertToProvisionalState(DocumentLoader*) { notImplemented(); }
    virtual void setMainDocumentError(DocumentLoader*, const ResourceError&) { notImplemented(); }
    virtual void postProgressStartedNotification();
    virtual void postProgressEstimateChangedNotification();
    virtual void postProgressFinishedNotification();
    virtual void setMainFrameDocumentReady(bool) { notImplemented(); }
    virtual void startDownload(const ResourceRequest&) { notImplemented(); }
    virtual void willChangeTitle(DocumentLoader*) { notImplemented(); }
    virtual void didChangeTitle(DocumentLoader*) { notImplemented(); }
    virtual void committedLoad(DocumentLoader*, const char*, int);
    virtual void finishedLoading(DocumentLoader*);
    virtual void updateGlobalHistory() { notImplemented(); }
    virtual void updateGlobalHistoryRedirectLinks() { notImplemented(); }
    virtual bool shouldGoToHistoryItem(HistoryItem*) const;
    virtual void dispatchDidAddBackForwardItem(HistoryItem*) const;
    virtual void dispatchDidRemoveBackForwardItem(HistoryItem*) const;
    virtual void dispatchDidChangeBackForwardIndex() const;
    virtual void didDisplayInsecureContent() { notImplemented(); }
    virtual void didRunInsecureContent(SecurityOrigin*) { notImplemented(); }
    virtual ResourceError cancelledError(const ResourceRequest&) { notImplemented(); return ResourceError("", 0, "", ""); }
    virtual ResourceError blockedError(const ResourceRequest&) { notImplemented(); return ResourceError("", 0, "", ""); }
    virtual ResourceError cannotShowURLError(const ResourceRequest&) { notImplemented(); return ResourceError("", 0, "", ""); }
    virtual ResourceError interruptForPolicyChangeError(const ResourceRequest&) { notImplemented(); return ResourceError("", 0, "", ""); }
    virtual ResourceError cannotShowMIMETypeError(const ResourceResponse&) { notImplemented(); return ResourceError("", 0, "", ""); }
    virtual ResourceError fileDoesNotExistError(const ResourceResponse&) { notImplemented(); return ResourceError("", 0, "", ""); }
    virtual ResourceError pluginWillHandleLoadError(const ResourceResponse&) { notImplemented(); return ResourceError("", 0, "", ""); }
    virtual bool shouldFallBack(const ResourceError&) { notImplemented(); return false; }
    virtual bool canHandleRequest(const ResourceRequest&) const;
    virtual bool canShowMIMEType(const String&) const;
    virtual bool canShowMIMETypeAsHTML(const String&) const;
    virtual bool representationExistsForURLScheme(const String&) const { notImplemented(); return false; }
    virtual String generatedMIMETypeForURLScheme(const String&) const { notImplemented(); return String(); }
    virtual void frameLoadCompleted() { notImplemented(); }
    virtual void saveViewStateToItem(HistoryItem*);
    virtual void restoreViewState();
    virtual void provisionalLoadStarted() { notImplemented(); }
    virtual void didFinishLoad() { notImplemented(); }
    virtual void prepareForDataSourceReplacement() { notImplemented(); }
    virtual WTF::PassRefPtr<DocumentLoader> createDocumentLoader(const ResourceRequest&, const SubstituteData&);
    virtual void setTitle(const String&, const KURL&) { notImplemented(); }
    virtual String userAgent(const KURL&);
    virtual void savePlatformDataToCachedFrame(CachedFrame*) { notImplemented(); }
    virtual void transitionToCommittedFromCachedFrame(CachedFrame*) { notImplemented(); }
    virtual void transitionToCommittedForNewPage();
    virtual bool canCachePage() const { notImplemented(); return false; }
    virtual void download(ResourceHandle*, const ResourceRequest&, const ResourceRequest&, const ResourceResponse&) { notImplemented(); }
    virtual WTF::PassRefPtr<Frame> createFrame(const KURL&, const String&, HTMLFrameOwnerElement*, const String&, bool, int, int);
    virtual WTF::PassRefPtr<Widget> createPlugin(const IntSize&, HTMLPlugInElement*, const KURL&, const WTF::Vector<String, 0u>&, const WTF::Vector<String, 0u>&, const String&, bool);
    virtual void redirectDataToPlugin(Widget*) { notImplemented(); }
    virtual WTF::PassRefPtr<Widget> createJavaAppletWidget(const IntSize&, HTMLAppletElement*, const KURL&, const WTF::Vector<String, 0u>&, const WTF::Vector<String, 0u>&) { notImplemented(); return 0; }

#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    virtual WTF::PassRefPtr<Widget> createMediaPlayerProxyPlugin(const IntSize&, HTMLMediaElement*, const KURL&, const WTF::Vector<String>&, const WTF::Vector<String>&, const String&);
#endif

    virtual ObjectContentType objectContentType(const KURL&, const String&);
    virtual String overrideMediaType() const { notImplemented(); return String(); }
    virtual void dispatchDidClearWindowObjectInWorld(DOMWrapperWorld*);
    virtual void documentElementAvailable() { notImplemented(); }
    virtual void didPerformFirstNavigation() const { notImplemented(); }
    virtual void registerForIconNotification(bool) { notImplemented(); }

    virtual bool shouldLoadIconExternally() { return true; }
    virtual void loadIconExternally(const String& originalPageUrl, const String& finalPageUrl, const String& iconUrl);

    virtual void didTransferChildFrameToNewDocument() { notImplemented(); };
    virtual void dispatchDidChangeIcons() { notImplemented(); };
    virtual void dispatchWillSendSubmitEvent(HTMLFormElement*) { notImplemented(); };

    virtual void willDeferLoading();
    virtual void didResumeLoading();

    virtual void didCreateGeolocation(Geolocation*);

    virtual PassRefPtr<FrameNetworkingContext> createNetworkingContext();

    /**
     * Schedule a script that was loaded manually by the user (eg. a
     * bookmarklet) while page loading was deferred.
     */
    void setDeferredManualScript(const KURL&);

    void readyToRender(bool pageIsVisuallyNonEmpty);

    void doPendingFragmentScroll();

    virtual void hideMediaPlayerProxyPlugin(WebCore::Widget*);
    virtual void showMediaPlayerProxyPlugin(WebCore::Widget*);

private:
    void receivedData(const char*, int, const String&);
    void didFinishOrFailLoading(const ResourceError&);
    bool isMainFrame() const;

    void invalidateBackForwardList() const;
    void invalidateBackForwardTimerFired(Timer<FrameLoaderClientBlackBerry>*);

    PassRefPtr<Widget> createPluginHolder(const IntSize& pluginSize,HTMLElement* element, const KURL& url, const Vector<String>& paramNames,const Vector<String>& paramValues, const String& mimeType,bool isPlaceHolder);

    PolicyAction decidePolicyForExternalLoad(const ResourceRequest &, bool isFragmentScroll, PassRefPtr<FormState> formState);
    void delayPolicyCheckUntilFragmentExists(const String& fragment, FramePolicyFunction);

    void deferredJobsTimerFired(Timer<FrameLoaderClientBlackBerry>*);

    Frame* m_frame;
    ResourceError m_loadError;
    Olympia::WebKit::WebPage* m_webPage;
    mutable Timer<FrameLoaderClientBlackBerry>* m_invalidateBackForwardTimer;

    typedef HashMap<HTMLElement*, RefPtr<Olympia::WebKit::WebPlugin> > PluginCacheMap;
    PluginCacheMap m_cachedPlugins;

    Timer<FrameLoaderClientBlackBerry>* m_deferredJobsTimer;
    KURL m_deferredManualScript;
    RefPtr<Geolocation> m_geolocation;
    bool m_sentReadyToRender;

    FramePolicyFunction m_pendingFragmentScrollPolicyFunction;
    String m_pendingFragmentScroll;

    bool m_clientRedirectIsPending;
    bool m_clientRedirectIsUserGesture;

    // this set includes the original and final urls for server redirects
    HashSet<KURL> m_historyNavigationSourceURLs;
    HashSet<KURL> m_redirectURLsToSkipDueToHistoryNavigation;
};

} // WebCore

#endif // FrameLoaderClientOlympia_h
