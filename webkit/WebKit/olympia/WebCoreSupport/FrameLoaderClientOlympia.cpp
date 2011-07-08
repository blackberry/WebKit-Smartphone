/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "FrameLoaderClientOlympia.h"

#include "BackingStore.h"
#include "BackingStore_p.h"
#include "BackForwardList.h"
#include "Base64.h"
#include "CString.h"
#include "DumpRenderTreeOlympia.h"
#include "FrameLoader.h"
#include "FrameView.h"
#include "Geolocation.h"
#include "GeolocationServiceOlympia.h"
#include "HistoryItem.h"
#include "HTMLMediaElement.h"
#include "HTMLMetaElement.h"
#include "HTMLPluginElement.h"
#include "InputHandler.h"
#include "MIMETypeRegistry.h"
#include "NodeList.h"
#include "NotImplemented.h"
#include "PluginWidget.h"
#include "ProgressTracker.h"
#include "RenderEmbeddedObject.h"
#include "ResourceError.h"
#include "ScopePointer.h"
#include "SecurityOrigin.h"
#include "TextEncoding.h"
#include "WebPage.h"
#include "WebPage_p.h"
#include "WebPageClient.h"
#include "WebPlugin.h"
#include "WebSettings.h"
#include "WebDOMDocument.h"
#include "JavaScriptCore/APICast.h"
#include "streams/IStream.h"
#include "streams/NetworkRequest.h"

using namespace WTF;
using namespace WebCore;
using namespace Olympia::WebKit;

namespace WebCore {

FrameLoaderClientOlympia::FrameLoaderClientOlympia()
    : m_frame(0)
    , m_webPage(0)
    , m_sentReadyToRender(false)
    , m_pendingFragmentScrollPolicyFunction(0)
{
    m_invalidateBackForwardTimer = new Timer<FrameLoaderClientOlympia>(this, &FrameLoaderClientOlympia::invalidateBackForwardTimerFired);
    m_deferredJobsTimer = new Timer<FrameLoaderClientOlympia>(this, &FrameLoaderClientOlympia::deferredJobsTimerFired);
}

FrameLoaderClientOlympia::~FrameLoaderClientOlympia()
{
    delete m_invalidateBackForwardTimer;
    m_invalidateBackForwardTimer = 0;
    delete m_deferredJobsTimer;
    m_deferredJobsTimer = 0;
}

int FrameLoaderClientOlympia::playerId() const
{
    if (m_webPage && m_webPage->client())
        return m_webPage->client()->getInstanceId();
    return 0;
}

void FrameLoaderClientOlympia::dispatchDidAddBackForwardItem(HistoryItem* item) const
{
    // inform the client that the back/forward list has changed
    invalidateBackForwardList();
}

void FrameLoaderClientOlympia::dispatchDidRemoveBackForwardItem(HistoryItem* item) const
{
    invalidateBackForwardList();
}

void FrameLoaderClientOlympia::dispatchDidChangeBackForwardIndex() const
{
    invalidateBackForwardList();
}

void FrameLoaderClientOlympia::dispatchDidChangeLocationWithinPage()
{
    if (!isMainFrame())
        return;

    String url = m_frame->loader()->url().string();
    String token = m_frame->loader()->documentLoader()->request().token();

    m_webPage->client()->notifyLoadToAnchor(url.characters(), url.length(), token.characters(), token.length());
}

void FrameLoaderClientOlympia::dispatchWillPerformClientRedirect(const KURL& url, double, double)
{
    if (!isMainFrame())
        return;

    String originalUrl = m_frame->loader()->url().string();
    String finalUrl = url.string();
    m_webPage->client()->notifyClientRedirect(originalUrl.characters(), originalUrl.length(),
            finalUrl.characters(), finalUrl.length());
}

void FrameLoaderClientOlympia::dispatchDecidePolicyForMIMEType(FramePolicyFunction function, const String&, const ResourceRequest&)
{
    // FIXME: stub
    (m_frame->loader()->policyChecker()->*function)(PolicyUse);
}

void FrameLoaderClientOlympia::dispatchDecidePolicyForNavigationAction(FramePolicyFunction function, const NavigationAction&, const ResourceRequest& request, PassRefPtr<FormState>)
{
    // Fragment scrolls on the same page should always be handled internally.
    // (Only count as a fragment scroll if we are scrolling to a #fragment url, not back to the top, and reloading
    // the same url is not a fragment scroll even if it has a #fragment.)
    const KURL& url = request.url();
    const KURL& currentUrl = m_frame->loader()->url();
    bool isFragmentScroll = url.hasFragmentIdentifier() && url != currentUrl && equalIgnoringFragmentIdentifier(currentUrl, url);

    PolicyAction decision = decidePolicyForExternalLoad(request, isFragmentScroll);

    // if the client has not delivered all data to the main frame, a fragment scroll to an anchor that hasn't arrived yet will fail
    // kick the client and try again
    if (decision == PolicyUse && isFragmentScroll && isMainFrame()) {
        String fragment = url.fragmentIdentifier();
        if (fragment != currentUrl.fragmentIdentifier()) {
            delayPolicyCheckUntilFragmentExists(fragment, function);
            return;
        }
    }

    (m_frame->loader()->policyChecker()->*function)(decision);
}

void FrameLoaderClientOlympia::delayPolicyCheckUntilFragmentExists(const String& fragment, FramePolicyFunction function)
{
    ASSERT(isMainFrame());

    if (m_webPage->d->loadState() < WebPagePrivate::Finished && !m_frame->document()->findAnchor(fragment)) {
        // tell the client we need more data, in case the fragment exists but is being held back
        m_webPage->client()->needMoreData();
        m_pendingFragmentScrollPolicyFunction = function;
        m_pendingFragmentScroll = fragment;
        return;
    }

    (m_frame->loader()->policyChecker()->*function)(PolicyUse);
}

void FrameLoaderClientOlympia::cancelPolicyCheck()
{
    m_pendingFragmentScrollPolicyFunction = 0;
    m_pendingFragmentScroll = String();
}

void FrameLoaderClientOlympia::doPendingFragmentScroll()
{
    if (m_pendingFragmentScroll.isNull())
        return;

    // make sure to clear the pending members first to avoid recursion
    String fragment = m_pendingFragmentScroll;
    m_pendingFragmentScroll = String();

    FramePolicyFunction function = m_pendingFragmentScrollPolicyFunction;
    m_pendingFragmentScrollPolicyFunction = 0;

    delayPolicyCheckUntilFragmentExists(fragment, function);
}

void FrameLoaderClientOlympia::dispatchDecidePolicyForNewWindowAction(FramePolicyFunction function, const NavigationAction&, const ResourceRequest& request, PassRefPtr<FormState>, const String& frameName)
{
    // a new window can never be a fragment scroll
    PolicyAction decision = decidePolicyForExternalLoad(request, false);
    (m_frame->loader()->policyChecker()->*function)(decision);
}

void FrameLoaderClientOlympia::committedLoad(DocumentLoader* loader, const char* data, int length)
{
    const String& textEncoding = loader->response().textEncodingName();
    receivedData(data, length, textEncoding);
}

PassRefPtr<Widget> FrameLoaderClientOlympia::createPlugin(const IntSize& pluginSize,
    HTMLPlugInElement* element, const KURL& url, const Vector<String>& paramNames,
    const Vector<String>& paramValues, const String& mimeType, bool loadManually)
{
    // Check cached plugin. If shouldCancelled has been set to true, 
    // means it can't be loaded as plugin, so load the url in normal way. 
    RefPtr<WebPlugin> plugin = getCachedPlugin(element);
    if (plugin && plugin.get()->shouldBeCancelled()) {
        m_frame->loader()->requestFrame(element, url, String());
        return 0;
    }
    return createPluginHolder(pluginSize, static_cast<HTMLElement*>(element), url, paramNames, paramValues, mimeType, false, false, true);
}

#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
PassRefPtr<Widget> FrameLoaderClientOlympia::createMediaPlayerProxyPlugin(const IntSize& pluginSize,
    HTMLMediaElement* element, const KURL& url, const Vector<String>& paramNames,
    const Vector<String>& paramValues, const String& mimeType)
{
    // Check if plugin loading needs to be cancelled.
    RefPtr<WebPlugin> plugin = getCachedPlugin(element);
    if (plugin && plugin.get()->shouldBeCancelled())
        return 0;

    if (!m_frame)
        return 0;

    ASSERT(m_frame->document());

    String mediaUrl = url.string();
    for (unsigned i = 0; i < paramNames.size(); ++i) {
        if (paramNames[i] == "_media_element_src_") {
            mediaUrl = paramValues[i];
            break;
        }
    }

    bool isRendererNeeded = element->controls() || element->renderer();
    return createPluginHolder(pluginSize, static_cast<HTMLElement*>(element), m_frame->document()->completeURL(mediaUrl), paramNames, paramValues, String(), mediaUrl.isEmpty(), element->isVideo(), isRendererNeeded);
}
#endif

PassRefPtr<Widget> FrameLoaderClientOlympia::createPluginHolder(const IntSize& pluginSize,
    HTMLElement* element, const KURL& url, const Vector<String>& paramNames,
    const Vector<String>& paramValues, const String& mimeType, bool isPlaceHolder, 
    bool isHtml5Video, bool hasRenderer)
{
    // Check cached plugin, we don't support more than one media plugin currently, 
    // so stop create media plugin if there is one playing.
    if (m_cachedPlugins.size() > 0)
        return 0;

    if (!m_frame)
        return 0;

    // Check MIMEType again, because if an object's content can't be handled and it has no fallback, 
    // FrameLoader::requestObject will let it go here.
    if (objectContentType(url, mimeType) != ObjectContentOtherPlugin)
        return 0;

    
    int width = pluginSize.width();
    int height = pluginSize.height();

    // Check param setting.
    for (unsigned i = 0; i < paramNames.size(); ++i) {
        if (paramNames[i] == "width")
            width = paramValues[i].toInt();
        else if (paramNames[i] == "height")
            height = paramValues[i].toInt();
    }
    
    RefPtr<WebPlugin> plugin = isPlaceHolder ? adoptRef(new WebPlugin(m_webPage, width, height, element))
        : getCachedPlugin(element);

    if (!plugin && !isPlaceHolder) {
        // The plugin for this media URL was not loaded, so create a placeholder for it. 
        plugin = adoptRef(new WebPlugin(m_webPage, width, height, element, url.string(), isHtml5Video, hasRenderer));
        m_cachedPlugins.add(element, plugin);
    }

    if (!plugin)
        return 0;

    ASSERT(!plugin->isLoaded());

    RefPtr<PluginWidget> w = adoptRef(new PluginWidget());
    w->setPlatformWidget(plugin.get());
    plugin->setLoaded(true);
    plugin->setWidget(w);
    // Make sure it's invisible until properly placed into the layout.
    w->setFrameRect(IntRect(-1, -1, width, height));
    return w;
   
}

void FrameLoaderClientOlympia::cleanPluginList()
{
    for (HashMap<HTMLElement*, RefPtr<Olympia::WebKit::WebPlugin> >::iterator iter = m_cachedPlugins.begin(); iter != m_cachedPlugins.end(); ++iter) {
        WebPlugin* plugin = iter->second.get();
        plugin->detach();
    }
    m_cachedPlugins.clear();
}

void FrameLoaderClientOlympia::cancelLoadingPlugin(int id)
{
    for (HashMap<HTMLElement*, RefPtr<Olympia::WebKit::WebPlugin> >::iterator iter = m_cachedPlugins.begin(); iter != m_cachedPlugins.end(); ++iter) {
        WebPlugin* plugin = iter->second.get();
        if (plugin->playerID() == id) {
            plugin->setShouldBeCancelled(true);
            plugin->detach();
            if (plugin->element() && plugin->element()->renderer()) {
                HTMLElement* objectElement = plugin->element();
                RenderPart* renderer = toRenderPart(objectElement->renderer());
                renderer->setWidget(0);
                renderer->setNeedsLayout(true);
                m_frame->view()->scheduleRelayoutOfSubtree(objectElement->renderer());
            }
            return;
        }
    }
}

PassRefPtr<WebPlugin> FrameLoaderClientOlympia::getCachedPlugin(HTMLElement* element)
{
    if (m_cachedPlugins.contains(element))
        return m_cachedPlugins.get(element);

    return 0;
}

void FrameLoaderClientOlympia::receivedData(const char* data, int length, const String& textEncoding)
{
    if (!m_frame)
        return;

    // Set the encoding. This only needs to be done once, but it's harmless to do it again later.
    String encoding = m_frame->loader()->documentLoader()->overrideEncoding();
    bool userChosen = !encoding.isNull();
    if (encoding.isNull())
        encoding = textEncoding;
    m_frame->loader()->writer()->setEncoding(encoding, userChosen);
    m_frame->loader()->addData(data, length);
}

void FrameLoaderClientOlympia::finishedLoading(DocumentLoader* loader)
{
    // Telling the frame we received some data and passing 0 as the data is our
    // way to get work done that is normally done when the first bit of data is
    // received, even for the case of a document with no data (like about:blank)
    committedLoad(loader, 0, 0);

    if (!m_frame->document())
        return;
        
    SecurityOrigin* securityOrigin = m_frame->document()->securityOrigin();
    Olympia::WebKit::WebSettings* webSettings = m_webPage->settings();

    // whitelist all known protocols if BrowserField2 requires unrestricted requests
    if (webSettings->allowCrossSiteRequests() && !securityOrigin->isEmpty()) {
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "about", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "data", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "file", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "http", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "https", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "javascript", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "local", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "mailto", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "pattern", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "rtcp", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "rtp", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "rtsp", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "sms", "", true);
        securityOrigin->addOriginAccessWhitelistEntry(*securityOrigin, "tel", "", true);
    }
}

WTF::PassRefPtr<DocumentLoader> FrameLoaderClientOlympia::createDocumentLoader(const ResourceRequest& request, const SubstituteData& substituteData)
{
    // make a copy of the request with the token from the original request for this frame
    // (unless it already has a token, in which case the request came from the client)
    ResourceRequest newRequest(request);
    if (m_frame && m_frame->loader() && m_frame->loader()->documentLoader()) {
        const ResourceRequest& originalRequest = m_frame->loader()->documentLoader()->originalRequest();
        if (request.token().isNull() && !originalRequest.token().isNull())
            newRequest.setToken(originalRequest.token());
    }

    // FIXME: this should probably be shared
    RefPtr<DocumentLoader> loader = DocumentLoader::create(newRequest, substituteData);
    if (substituteData.isValid())
        loader->setDeferMainResourceDataLoad(false);
    return loader.release();
}

void FrameLoaderClientOlympia::frameLoaderDestroyed()
{
    delete this;
}

void FrameLoaderClientOlympia::transitionToCommittedForNewPage()
{
    // Clear cached plugins.
    m_cachedPlugins.clear();
    m_frame->createView(m_webPage->d->viewportSize(),     /*viewport*/
                        Color(),                          /*background color*/
                        false,                            /*is transparent*/
                        m_webPage->d->actualVisibleSize(),/*fixed reported size*/
                        m_webPage->d->fixedLayoutSize(),  /*fixed layout size*/
                        m_webPage->d->useFixedLayout(),   /*use fixed layout */
                        ScrollbarAlwaysOff,               /*hor mode*/
                        true,                             /*lock the mode*/
                        ScrollbarAlwaysOff,               /*ver mode*/
                        true);                            /*lock the mode*/
    m_frame->view()->updateCanHaveScrollbars();

    // Since the mainframe has a tiled backingstore request to receive all update
    // rects instead of the default which just sends update rects for currently
    // visible viewport
    if (isMainFrame())
        m_frame->view()->setPaintsEntireContents(true);
}

String FrameLoaderClientOlympia::userAgent(const KURL&)
{
    return m_webPage->settings()->userAgentString();
}

bool FrameLoaderClientOlympia::canHandleRequest(const ResourceRequest&) const
{
    // FIXME: stub
    return true;
}

bool FrameLoaderClientOlympia::canShowMIMEType(const String&) const
{
    // FIXME: respect the actual mimetype
    return true;
}

bool FrameLoaderClientOlympia::isMainFrame() const
{
    return m_frame == m_webPage->d->m_mainFrame;
}

void FrameLoaderClientOlympia::dispatchDidStartProvisionalLoad()
{
    if (isMainFrame())
        m_webPage->d->setLoadState(WebPagePrivate::Provisional);

    if (m_webPage->d->m_dumpRenderTree)
        m_webPage->d->m_dumpRenderTree->didStartProvisionalLoadForFrame(m_frame);
}

void FrameLoaderClientOlympia::dispatchDidReceiveTitle(const String& title)
{
    if (isMainFrame())
        m_webPage->client()->setPageTitle(title.characters(), title.length());

    if (m_webPage->d->m_dumpRenderTree)
        m_webPage->d->m_dumpRenderTree->didReceiveTitleForFrame(title, m_frame);
}

void FrameLoaderClientOlympia::dispatchDidCommitLoad()
{
    bool loadFailed = m_frame->loader()->documentLoader()->substituteData().isValid();

    m_frame->document()->setExtraLayoutDelay(250);

    if (isMainFrame()) {
        m_webPage->d->setLoadState(WebPagePrivate::Committed);

        String originalUrl = m_frame->loader()->documentLoader()->originalRequest().url().string();
        String url = m_frame->loader()->documentLoader()->request().url().string();
        String token = m_frame->loader()->documentLoader()->request().token();

        // update the token in the global history item
        HistoryItem* item = m_webPage->d->m_page->globalHistoryItem();
        if (item)
            item->setNetworkToken(token);

        // notify the client that the load succeeded or failed (if it failed, this
        // is actually committing the error page, which was set through
        // SubstituteData in dispatchDidFailProvisionalLoad)
        if (loadFailed)
            m_webPage->client()->notifyLoadFailedBeforeCommit(originalUrl.characters(), originalUrl.length(),
                    url.characters(), url.length(), token.characters(), token.length());
        else
            m_webPage->client()->notifyLoadCommitted(originalUrl.characters(), originalUrl.length(),
                    url.characters(), url.length(), token.characters(), token.length());
    }

    // TODO: creating a WebDOMDocument for subframes here is a bit heavy since it's not always used; revisit later
    if (!loadFailed) {
        JSDOMWindow* jsWindow = m_frame->script()->globalObject(WebCore::mainThreadNormalWorld());
        if (jsWindow) {
            JSContextRef contextRef = toRef(jsWindow->globalExec());
            if (m_webPage->settings()->isEmailMode())
                m_webPage->client()->notifyDocumentCreatedForFrameAsync(m_frame, isMainFrame(), WebDOMDocument(m_frame->document()), contextRef, toRef(jsWindow));
            else
                m_webPage->client()->notifyDocumentCreatedForFrame(m_frame, isMainFrame(), WebDOMDocument(m_frame->document()), contextRef, toRef(jsWindow));
        }
    }

    if (m_webPage->d->m_dumpRenderTree)
        m_webPage->d->m_dumpRenderTree->didCommitLoadForFrame(m_frame);
}

void FrameLoaderClientOlympia::dispatchDidFinishLoad()
{
    m_frame->document()->setExtraLayoutDelay(0);

    if (isMainFrame()) {
        m_loadError = ResourceError();
        m_webPage->d->setLoadState(WebPagePrivate::Finished);

        if (!m_webPage->settings()->isEmailMode()) {
            // pass header info from meta tags to client
            RefPtr<NodeList> nodeList = m_frame->document()->getElementsByTagName("meta");
            unsigned int size = nodeList->length();

            ScopeArray<Olympia::WebKit::String> headers;
            if (size > 0) {
                // this may allocate more space than needed since not all meta elements will be http-equiv
                headers.reset(new Olympia::WebKit::String[2 * size]);
            }

            unsigned int headersLength = 0;
            for (unsigned int i = 0; i < size; ++i) {
                HTMLMetaElement* metaElement = static_cast<HTMLMetaElement*>(nodeList->item(i));
                String httpEquiv = metaElement->httpEquiv().stripWhiteSpace();
                String content = metaElement->content().stripWhiteSpace();
                if (!httpEquiv.isNull() && !content.isNull()) {
                    headers[headersLength] = httpEquiv;
                    ++headersLength;
                    headers[headersLength] = content;
                    ++headersLength;
                }
            }

            m_webPage->client()->setMetaHeaders(headers, headersLength);
        }
    }

    if (m_webPage->d->m_dumpRenderTree)
        m_webPage->d->m_dumpRenderTree->didFinishLoadForFrame(m_frame);
}

void FrameLoaderClientOlympia::dispatchDidFinishDocumentLoad()
{
    if (m_webPage->d->m_dumpRenderTree)
        m_webPage->d->m_dumpRenderTree->didFinishDocumentLoadForFrame(m_frame);
    notImplemented();
}


void FrameLoaderClientOlympia::dispatchDidFailLoad(const ResourceError& error)
{
    m_frame->document()->setExtraLayoutDelay(0);

    if (isMainFrame()) {
        m_loadError = error;
        m_webPage->d->setLoadState(WebPagePrivate::Failed);
    }

    if (m_webPage->d->m_dumpRenderTree)
        m_webPage->d->m_dumpRenderTree->didFailLoadForFrame(m_frame);
}

void FrameLoaderClientOlympia::dispatchDidFailProvisionalLoad(const ResourceError& error)
{
    if (isMainFrame()) {
        m_loadError = error;
        m_webPage->d->setLoadState(WebPagePrivate::Failed);

        if (error.domain() == ResourceError::platformErrorDomain
                && (error.errorCode() == Olympia::Platform::IStream::StatusErrorAlreadyHandled)) {
            // error has already been displayed by client
            return;
        }

        if (error.domain() == "" && error.errorCode() == 0 && error.failingURL() == "" && error.localizedDescription() == "") {
            // don't try to display empty errors returned from the unimplemented error functions in FrameLoaderClientOlympia - there's nothing to display anyway
            return;
        }
    }

    if (m_webPage->d->m_dumpRenderTree)
        m_webPage->d->m_dumpRenderTree->didFailProvisionalLoadForFrame(m_frame);

    if (!isMainFrame())
        return;

    String errorPage = m_webPage->d->m_client->getErrorPage(error.errorCode()
            , error.localizedDescription().isEmpty() ? "" : error.localizedDescription().latin1().data()
            , error.failingURL().isEmpty() ? "" : error.failingURL().latin1().data());

    // make sure we're still in the provisionalLoad state - getErrorPage runs a
    // nested event loop while it's waiting for client resources to load so
    // there's a small window for the user to hit stop
    if (m_frame->loader()->provisionalDocumentLoader()) {
        SubstituteData errorData(utf8Buffer(errorPage), "text/html", "utf-8", KURL(KURL(), error.failingURL()));

        ResourceRequest originalRequest = m_frame->loader()->provisionalDocumentLoader()->originalRequest();

        // Loading using SubstituteData will replace the original request with out
        // error data.  This must be done within dispatchDidFailProvisionalLoad,
        // and do NOT call stopAllLoaders first, because the loader checks the
        // provisionalDocumentLoader to decide the load type; if called any other
        // way, the error page is added to the end of the history instead of
        // replacing the failed load.
        m_frame->loader()->load(originalRequest, errorData, false);
    }
}

void FrameLoaderClientOlympia::dispatchWillSubmitForm(FramePolicyFunction function, PassRefPtr<FormState>)
{
    // FIXME: stub
    (m_frame->loader()->policyChecker()->*function)(PolicyUse);
}

WTF::PassRefPtr<Frame> FrameLoaderClientOlympia::createFrame(const KURL& url, const String& name
    , HTMLFrameOwnerElement* ownerElement, const String& referrer, bool allowsScrolling, int marginWidth, int marginHeight)
{
    if (!m_webPage)
        return 0;

    FrameLoaderClientOlympia* frameLoaderClient = new FrameLoaderClientOlympia();
    RefPtr<Frame> childFrame = Frame::create(m_frame->page(), ownerElement, frameLoaderClient);
    frameLoaderClient->setFrame(childFrame.get(), m_webPage);

    // Initialize FrameView
    RefPtr<FrameView> frameView = FrameView::create(childFrame.get());
    childFrame->setView(frameView.get());
    if (!allowsScrolling)
        frameView->setScrollbarModes(ScrollbarAlwaysOff, ScrollbarAlwaysOff);
    if (marginWidth != -1)
        frameView->setMarginWidth(marginWidth);
    if (marginHeight != -1)
        frameView->setMarginHeight(marginHeight);

    m_frame->tree()->appendChild(childFrame);
    childFrame->tree()->setName(name);
    childFrame->init();

    if (!childFrame->tree()->parent())
        return 0;

    m_frame->loader()->loadURLIntoChildFrame(url, referrer, childFrame.get());

    if (!childFrame->tree()->parent())
        return 0;

    return childFrame.release();
}

ObjectContentType FrameLoaderClientOlympia::objectContentType(const KURL& url, const String& mimeTypeIn)
{
    String mimeType = mimeTypeIn;
    if (mimeType.isEmpty())
        mimeType = MIMETypeRegistry::getMIMETypeForExtension(url.path().substring(url.path().reverseFind('.') + 1));

    // Get mapped type.
    mimeType = Olympia::WebKit::WebSettings::getNormalizedMIMEType(mimeType);

    ObjectContentType defaultType = FrameLoader::defaultObjectContentType(url, mimeType);
    if (defaultType != ObjectContentNone)
        return defaultType;

    if (Olympia::WebKit::WebSettings::isSupportedObjectMIMEType(mimeType))
        return WebCore::ObjectContentOtherPlugin;
    
    return ObjectContentNone;
}

void FrameLoaderClientOlympia::dispatchWillClose()
{
    m_webPage->d->m_inputHandler->frameUnloaded(m_frame);
}

void FrameLoaderClientOlympia::postProgressStartedNotification()
{
    if (!isMainFrame())
        return;

    // new load started, so clear the error
    m_loadError = ResourceError();
    m_sentReadyToRender = false;
    m_webPage->client()->notifyLoadStarted();
}

void FrameLoaderClientOlympia::postProgressEstimateChangedNotification()
{
    if (!isMainFrame() || !m_frame->page())
        return;

    m_webPage->client()->notifyLoadProgress(m_frame->page()->progress()->estimatedProgress() * 100);
}

void FrameLoaderClientOlympia::dispatchDidFirstVisuallyNonEmptyLayout()
{
    if (!isMainFrame())
        return;

    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "dispatchDidFirstVisuallyNonEmptyLayout");

    readyToRender(true);

    // FIXME: We shouldn't be getting here if we are not in the Committed state but we are
    // so we can not assert on that right now. But we only want to do this on load.
    // RIM Bug #555
    if (m_webPage->d->loadState() == WebPagePrivate::Committed) {
        m_webPage->d->zoomToInitialScaleOnLoad(); // set the proper zoom level first
        m_webPage->backingStore()->d->clearVisibleZoom(); // Clear the visible zoom since we're explicitly rendering+blitting below
        m_webPage->backingStore()->d->renderVisibleContents(false /*renderContentsOnly*/);
    }
}

void FrameLoaderClientOlympia::postProgressFinishedNotification()
{
    if (!isMainFrame())
        return;

    // empty pages will never have called
    // dispatchDidFirstVisuallyNonEmptyLayout, since they're visually empty, so
    // we may need to call readyToRender now
    readyToRender(false);

    // FIXME: send up a real status code
    m_webPage->client()->notifyLoadFinished(m_loadError.isNull() ? 0 : -1);
}

void FrameLoaderClientOlympia::dispatchDidClearWindowObjectInWorld(DOMWrapperWorld* world)
{
    if (m_webPage->d->m_dumpRenderTree)
        m_webPage->d->m_dumpRenderTree->didClearWindowObjectInWorld(world);
}

bool FrameLoaderClientOlympia::shouldGoToHistoryItem(HistoryItem*) const
{
    // stub
    return true;
}

void FrameLoaderClientOlympia::invalidateBackForwardList() const
{
    // since the list will change many times in a row,  batch up all
    // invalidations until next time through the event loop
    if (!m_invalidateBackForwardTimer->isActive())
        m_invalidateBackForwardTimer->startOneShot(0);
}

void FrameLoaderClientOlympia::invalidateBackForwardTimerFired(Timer<FrameLoaderClientOlympia>*)
{
    BackForwardList* backForwardList = m_webPage->d->m_page->backForwardList();
    ASSERT(backForwardList);

    unsigned int listSize = backForwardList->entries().size();
    unsigned int currentIndex = backForwardList->backListCount();
    m_webPage->client()->resetBackForwardList(listSize, currentIndex);
}

Frame* FrameLoaderClientOlympia::dispatchCreatePage()
{
    Olympia::WebKit::WebPage* webPage = m_webPage->client()->createWindow(0, 0, -1, -1, Olympia::WebKit::WebPageClient::FlagWindowDefault);
    if (!webPage)
        return 0;

    return webPage->d->m_page->mainFrame();
}

void FrameLoaderClientOlympia::detachedFromParent2()
{
    // Stop the timer if it's active. There's a chance
    // that the frame is held by a JS object but the page
    // has been destroyed. In any case, we don't want to
    // run the timer since this point.
    m_invalidateBackForwardTimer->stop();
    m_webPage->client()->notifyFrameDetached(m_frame);
    m_cachedPlugins.clear();
}

void FrameLoaderClientOlympia::loadIconExternally(const String& originalPageUrl, const String& finalPageUrl, const String& iconUrl)
{
    m_webPage->client()->setIconForUrl(originalPageUrl.latin1().data(), finalPageUrl.latin1().data(), iconUrl.latin1().data());
}

void FrameLoaderClientOlympia::saveViewStateToItem(HistoryItem* item)
{
    if (!isMainFrame())
        return;

    ASSERT(item);
    item->setScale(m_webPage->d->currentScale());
}

void FrameLoaderClientOlympia::restoreViewState()
{
    if (!isMainFrame())
        return;

    HistoryItem* currentItem = m_frame->loader()->history()->currentItem();
    ASSERT(currentItem);

    if (!currentItem)
        return;

    // WebPagePrivate is messing up FrameView::wasScrolledByUser() by sending
    // scroll events that look like they were user generated all the time.
    //
    // Even if the user did not scroll, FrameView is gonna think they did.
    // So we use our own bookkeeping code to keep track of whether we were
    // actually scrolled by the user during load.
    //
    // If the user did scroll though, all are going to be in agreement about
    // that, and the worst thing that could happen is that
    // HistoryController::restoreScrollPositionAndViewState calls
    // setScrollPosition with the the same point, which is a NOOP.
    IntSize contentsSize = currentItem->contentsSize();
    IntPoint scrollPosition = currentItem->scrollPoint();
    if (m_webPage->d->m_userPerformedManualScroll)
        scrollPosition = m_webPage->d->scrollPosition();

    // Also, try to keep the users zoom if any.
    double scale = currentItem->scale();
    if (m_webPage->d->m_userPerformedManualZoom)
        scale = m_webPage->d->currentScale();

    bool scrollChanged = scrollPosition != m_webPage->d->scrollPosition();
    bool scaleChanged = scale != m_webPage->d->currentScale();
    if (!scrollChanged && !scaleChanged)
        return;

    // It's not certain that we're going to get the scroll position we asked for,
    // so zoom about the scroll position that we actually got.
    m_webPage->backingStore()->d->suspendScreenUpdates(); // don't flash checkerboard for the setScrollPosition call
    m_frame->view()->setContentsSizeFromHistory(contentsSize);
    m_webPage->d->setScrollPosition(scrollPosition);
    // will restore updates to backingstore guaranteed!
    if (!m_webPage->d->zoomAboutPoint(scale, m_frame->view()->scrollPosition(), false /* enforceScaleClamping */, true /*forceRendering*/)) {
        // If we're already at that scale, then we should still force rendering since
        // our scroll position changed
        m_webPage->backingStore()->d->renderVisibleContents(false /*renderContentOnly*/);

        // We need to notify the client of the scroll position and content size change(s) above even if we didn't scale
        m_webPage->d->notifyTransformedContentsSizeChanged();
        m_webPage->d->notifyTransformedScrollChanged();
    }
}

PolicyAction FrameLoaderClientOlympia::decidePolicyForExternalLoad(const ResourceRequest& request, bool isFragmentScroll)
{
    const KURL& url = request.url();
    String pattern = m_webPage->d->findPatternStringForUrl(url);
    if (!pattern.isEmpty()) {
        m_webPage->client()->handleStringPattern(pattern.characters(), pattern.length());
        return PolicyIgnore;
    }

    if (m_webPage->settings()->areLinksHandledExternally()
            && request.targetType() == ResourceRequest::TargetIsMainFrame
            && !request.mustHandleInternally()
            && !isFragmentScroll) {
        Olympia::Platform::NetworkRequest platformRequest;
        request.initializePlatformRequest(platformRequest);
        m_webPage->client()->handleExternalLink(platformRequest, request.anchorText().characters(), request.anchorText().length());
        return PolicyIgnore;
    }

    return PolicyUse;
}

void FrameLoaderClientOlympia::willDeferLoading()
{
    m_deferredJobsTimer->stop();

    if (m_geolocation) {
        GeolocationServiceOlympia* service = static_cast<GeolocationServiceOlympia*>(m_geolocation->getGeolocationService());
        service->deferNotifications();
    }

    if (!isMainFrame())
        return;

    m_webPage->d->willDeferLoading();
}

void FrameLoaderClientOlympia::didResumeLoading()
{
    if (!m_deferredManualScript.isNull())
        m_deferredJobsTimer->startOneShot(0);
    else if (m_geolocation) {
        GeolocationServiceOlympia* service = static_cast<GeolocationServiceOlympia*>(m_geolocation->getGeolocationService());
        // If we have deferred notifications, we should use a timer to forward those notificatons.
        if (service->hasDeferredNotifications())
            m_deferredJobsTimer->startOneShot(0);
        else
            service->resumeNotifications();
    }

    if (!isMainFrame())
        return;

    m_webPage->d->didResumeLoading();
}

void FrameLoaderClientOlympia::setDeferredManualScript(const KURL& script)
{
    ASSERT(!m_deferredJobsTimer->isActive());
    m_deferredManualScript = script;
}

void FrameLoaderClientOlympia::deferredJobsTimerFired(Timer<FrameLoaderClientOlympia>*)
{
    ASSERT(!m_frame->page()->defersLoading());

    if (!m_deferredManualScript.isNull()) {
        // executing the script will set deferred loading, which could trigger this timer again if a script is set.  So clear the script first.
        KURL script = m_deferredManualScript;
        m_deferredManualScript = KURL();

        m_frame->script()->executeIfJavaScriptURL(script, /*user gesture*/ true);
    }

    ASSERT(!m_frame->page()->defersLoading());

    if (m_geolocation) {
        GeolocationServiceOlympia* service = static_cast<GeolocationServiceOlympia*>(m_geolocation->getGeolocationService());
        service->resumeNotifications();
    }
}

void FrameLoaderClientOlympia::didCreateGeolocation(Geolocation* geolocation)
{
    // There is only one Geolocation object per frame.
    ASSERT(!m_geolocation);
    m_geolocation = geolocation;
}

void FrameLoaderClientOlympia::readyToRender(bool pageIsVisuallyNonEmpty)
{
    // only send the notification once
    if (!m_sentReadyToRender) {
        m_webPage->client()->notifyLoadReadyToRender(pageIsVisuallyNonEmpty);
        m_sentReadyToRender = true;
    }
}

} // WebCore
