/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "WebPage.h"
#include "WebPage_p.h"

#include "BackForwardList.h"
#include "BackingStore.h"
#include "BackingStoreTile.h"
#include "BackingStore_p.h"
#include "CString.h"
#include "Chrome.h"
#include "ChromeClientOlympia.h"
#include "ContextMenuClientOlympia.h"
#include "Cursor.h"
#include "DragClientOlympia.h"
#include "DumpRenderTreeOlympia.h"
#include "EGLDisplayOpenVG.h"
#include "EditCommand.h"
#include "EditorClientOlympia.h"
#include "FocusController.h"
#include "Frame.h"
#include "FrameLoaderClientOlympia.h"
#include "FrameView.h"
#include "HTMLAnchorElement.h"
#include "HTMLAreaElement.h"
#include "HTMLFrameOwnerElement.h"
#include "HTMLImageElement.h"
#include "HTMLInputElement.h"
#include "HTMLNames.h"
#include "HTMLOptGroupElement.h"
#include "HTMLOptionElement.h"
#include "HTMLSelectElement.h"
#include "HistoryItem.h"
#include "HitTestResult.h"
#include "InputHandler.h"
#include "InspectorClientOlympia.h"
#include "IntPoint.h"
#include "IntRect.h"
#include "PlatformKeyboardEvent.h"
#include "JavaScriptCore/APICast.h"
#include "JavaScriptCore/JSContextRef.h"
#include "JavaScriptDebuggerOlympia.h"
#include "NetworkManager.h"
#include "Node.h"
#include "OlympiaPlatformInputEvents.h"
#include "OlympiaPlatformKeyboardEvent.h"
#include "OlympiaPlatformMemory.h"
#include "OlympiaPlatformMisc.h"
#include "OlympiaPlatformSettings.h"
#include "Page.h"
#include "PageGroup.h"
#include "PainterOpenVG.h"
#include "PlatformKeyboardEvent.h"
#include "PlatformTouchEvent.h"
#include "RenderLayer.h"
#include "RenderMenuList.h"
#include "RenderObject.h"
#include "RenderText.h"
#include "RenderThemeOlympia.h"
#include "RenderTreeAsText.h"
#include "RenderView.h"
#include "RenderWidget.h"
#include "ResourceHolderImpl.h"
#include "ScopePointer.h"
#include "ScrollTypes.h"
#include "SelectElement.h"
#include "SelectionHandler.h"
#include "Settings.h"
#include "Storage.h"
#include "StorageNamespace.h"
#include "SurfaceOpenVG.h"
#include "TouchEventHandler.h"
#include "TransformationMatrix.h"
#include "WebDOMDocument.h"
#include "WebPageClient.h"
#include "WebPlugin.h"
#include "WebSettings.h"
#include "runtime_root.h"

#include <vector>
#include <wtf/MathExtras.h>

#ifndef USER_PROCESSES
#include <memalloc.h>
#else
#if !defined(_MSC_VER)
#include <i_egl.h>
#endif
#endif

#define DEBUG_BLOCK_ZOOM 0
#define DEBUG_WEBPAGE_LOAD 0

using namespace WebCore;

typedef const unsigned short* CUShortPtr;

namespace Olympia {
namespace WebKit {

WebPage* WebPagePrivate::g_lastVisibleWebPage = 0; // Initially, no web page is visible.

const unsigned blockZoomMargin = 3; // add 3 pixel margin on each side
static int blockClickRadius = 25;
static double maximumBlockZoomScale = 3.0; // this scale can be clamped by the maximumScale set for the page

const double manualScrollInterval = 0.1; // the time interval during which we associate user action with scrolling

const double deferredScrollEventInterval = 1.0;
const double deferredResizeEventInterval = 1.0;
const double delayedZoomInterval = 1.0;

const IntSize minimumLayoutSize(10, 10); // needs to be a small size, greater than 0, that we can grow the layout from
const IntSize maximumLayoutSize(10000, 10000); // used with viewport meta tag, but we can still grow from this of course

// helper function to parse a URL and fill in missing parts
static KURL parseUrl(const char* url)
{
    WebCore::String urlString(url);
    KURL kurl = KURL(KURL(), urlString);
    if (kurl.protocol().isEmpty()) {
        urlString.insert("http://", 0);
        kurl = KURL(KURL(), urlString);
    }

    return kurl;
}

// helper functions to convert to WebCore types
static inline WebCore::MouseEventType toWebCoreMouseEventType(const MouseEventType type)
{
    switch (type) {
    case MouseEventPressed:
        return WebCore::MouseEventPressed;
    case MouseEventReleased:
        return WebCore::MouseEventReleased;
    case MouseEventMoved:
    default:
        return WebCore::MouseEventMoved;
    }
}

static inline WebCore::PlatformKeyboardEvent::Type toWebCorePlatformKeyboardEventType(const Olympia::Platform::KeyboardEvent::Type type)
{
    switch (type) {
    case Olympia::Platform::KeyboardEvent::KeyDown:
        return WebCore::PlatformKeyboardEvent::KeyDown;
    case Olympia::Platform::KeyboardEvent::KeyUp:
        return WebCore::PlatformKeyboardEvent::KeyUp;
    case Olympia::Platform::KeyboardEvent::KeyChar:
    default:
        return WebCore::PlatformKeyboardEvent::Char;
    }
}

static inline WebCore::ResourceRequestCachePolicy toWebCoreCachePolicy(Platform::NetworkRequest::CachePolicy policy)
{
    switch (policy) {
    case Platform::NetworkRequest::UseProtocolCachePolicy:
        return WebCore::UseProtocolCachePolicy;
    case Platform::NetworkRequest::ReloadIgnoringCacheData:
        return WebCore::ReloadIgnoringCacheData;
    case Platform::NetworkRequest::ReturnCacheDataElseLoad:
        return WebCore::ReturnCacheDataElseLoad;
    case Platform::NetworkRequest::ReturnCacheDataDontLoad:
        return WebCore::ReturnCacheDataDontLoad;
    default:
        ASSERT_NOT_REACHED();
        return WebCore::UseProtocolCachePolicy;
    }
}

static inline HistoryItem* historyItemFromBackForwardId(WebPage::BackForwardId id)
{
    return reinterpret_cast<HistoryItem*>(id);
}

static inline WebPage::BackForwardId backForwardIdFromHistoryItem(HistoryItem* item)
{
    return reinterpret_cast<WebPage::BackForwardId>(item);
}

WebPagePrivate::WebPagePrivate(WebPage* webPage, WebPageClient* client, const Platform::IntRect& rect)
    : m_webPage(webPage)
    , m_client(client)
    , m_page(0) // initialized by init
    , m_mainFrame(0) // initialized by init
    , m_currentContextNode(0)
    , m_webSettings(0) // initialized by init
    , m_visible(false)
    , m_shouldResetTilesWhenShown(false)
    , m_userScalable(true)
    , m_userPerformedManualZoom(false)
    , m_userPerformedManualScroll(false)
    , m_contentsSizeChanged(false)
    , m_overflowExceedsContentsSize(false)
    , m_olympiaOriginatedScroll(false)
    , m_resetVirtualViewportOnCommitted(true)
    , m_shouldUseFixedDesktopMode(false)
#if ENABLE(TOUCH_EVENTS)
    , m_preventDefaultOnTouchStart(false)
#endif
    , m_nestedLayoutFinishedCount(0)
    , m_x(rect.x())
    , m_y(rect.y())
    , m_width(rect.width())
    , m_height(rect.height())
    , m_actualVisibleWidth(rect.width())
    , m_actualVisibleHeight(rect.height())
    , m_virtualViewportWidth(0)
    , m_virtualViewportHeight(0)
    , m_defaultLayoutSize(minimumLayoutSize)
    , m_viewMode(WebPagePrivate::Mobile)
    , m_loadState(WebPagePrivate::None)
    , m_transformationMatrix(new WebCore::TransformationMatrix())
    , m_backingStore(0) // initialized by init
    , m_inputHandler(new InputHandler(this))
    , m_selectionHandler(new SelectionHandler(this))
    , m_touchEventHandler(new TouchEventHandler(this))
    , m_dateTimeInput(0) // lazy initialization
    , m_colorInput(0) // lazy initialization
    , m_selectElement(0) // lazy initialization
    , m_currentCursor(Olympia::Platform::CursorNone)
    , m_dumpRenderTree(0) // lazy initialization
    , m_zoomBounceBackAnimation(new Olympia::WebKit::OlympiaAnimation<WebPagePrivate>(this, &WebPagePrivate::animationZoomBounceBackFrameChanged))
    , m_blockZoomAnimation(new Olympia::WebKit::OlympiaAnimation<WebPagePrivate>(this, &WebPagePrivate::animationBlockZoomFrameChanged))
    , m_bitmapScale(1.0)
    , m_initialScale(-1.0)
    , m_minimumScale(-1.0)
    , m_maximumScale(-1.0)
    , m_blockZoomInitialScale(1.0)
    , m_blockZoomFinalScale(1.0)
    , m_currentBlockZoomNode(0)
    , m_currentBlockZoomAdjustedNode(0)
    , m_shouldReflowBlock(false)
    , m_bitmapZoomBuffer(0)
    , m_bitmapZoomBufferStride(0)
    , m_delayedZoomTimer(new Timer<WebPagePrivate>(this, &WebPagePrivate::zoomAboutPointTimerFired))
    , m_lastUserEventTimestamp(0.0)
    , m_deferredScrollEventTimer(new Timer<WebPagePrivate>(this, &WebPagePrivate::scrollEventTimerFired))
    , m_deferredResizeEventTimer(new Timer<WebPagePrivate>(this, &WebPagePrivate::resizeEventTimerFired))
{
    m_deferrableTimers.append(m_deferredScrollEventTimer.get());
    m_deferrableTimers.append(m_deferredResizeEventTimer.get());
}

WebPagePrivate::~WebPagePrivate()
{
    FrameLoader* loader = m_mainFrame->loader();
    if (loader)
        loader->detachFromParent();

    delete m_webSettings;
    m_webSettings = 0;

    delete m_page;
    m_page = 0;

    delete m_transformationMatrix;
    m_transformationMatrix = 0;

    delete m_selectionHandler;
    m_selectionHandler = 0;

    delete m_inputHandler;
    m_inputHandler = 0;

    delete m_touchEventHandler;
    m_touchEventHandler = 0;

    delete m_backingStore;
    m_backingStore = 0;

    delete m_dumpRenderTree;
    m_dumpRenderTree = 0;

    delete m_zoomBounceBackAnimation;
    m_zoomBounceBackAnimation = 0;

    delete m_blockZoomAnimation;
    m_blockZoomAnimation = 0;
}

void WebPagePrivate::init(const String& pageGroupName)
{
    globalInitialize();

    m_backingStore = new BackingStore(m_webPage);

    ChromeClientOlympia* chromeClient = new ChromeClientOlympia(m_webPage);
    ContextMenuClientOlympia* contextMenuClient = 0;
#if ENABLE(CONTEXT_MENUS)
    contextMenuClient = new ContextMenuClientOlympia();
#endif
    EditorClientOlympia* editorClient = new EditorClientOlympia(m_webPage);
    DragClientOlympia* dragClient = 0;
#if ENABLE(DRAG_SUPPORT)
    dragClient = new DragClientOlympia();
#endif
    InspectorClientOlympia* inspectorClient = 0;
#if ENABLE(INSPECTOR)
    inspectorClient = new InspectorClientOlympia();
#endif
    FrameLoaderClientOlympia* frameLoaderClient = new FrameLoaderClientOlympia();

    m_page = new Page(chromeClient, contextMenuClient, editorClient,
                      dragClient, inspectorClient, /*WebCore::PluginHalterClient*/0, 0, 0);

    m_page->setGroupName(pageGroupName);
    m_page->setCustomHTMLTokenizerChunkSize(256);
    m_page->setCustomHTMLTokenizerTimeDelay(0.300);

    m_webSettings = new WebSettings(m_page->settings(), pageGroupName);

    RefPtr<Frame> newFrame = Frame::create(m_page, /*HTMLFrameOwnerElement**/0, frameLoaderClient);

    m_mainFrame = newFrame.get();
    frameLoaderClient->setFrame(m_mainFrame, m_webPage);
    m_mainFrame->init();

#if ENABLE(VIEWPORT_REFLOW)
    m_page->settings()->setTextReflowEnabled(m_webSettings->textReflowMode() == WebSettings::TextReflowEnabled);
#endif
}

void WebPagePrivate::load(const char* url, const char* networkToken, const char* method, Platform::NetworkRequest::CachePolicy cachePolicy, const char* data, size_t dataLength, const char* const* headers, size_t headersLength, bool isInitial)
{
    stopCurrentLoad();

    KURL kurl = parseUrl(url);
    if (protocolIs(kurl, "javascript")) {
        // never run javascript while loading is deferred
        if (m_page->defersLoading()) {
            FrameLoaderClientOlympia* frameLoaderClient = static_cast<FrameLoaderClientOlympia*>(m_mainFrame->loader()->client());
            frameLoaderClient->setDeferredManualScript(kurl);
        } else
            m_mainFrame->script()->executeIfJavaScriptURL(kurl, /*user gesture*/ true);
        return;
    }

    if (isInitial)
        NetworkManager::instance()->setInitialURL(kurl);

    WebCore::ResourceRequest request(kurl, ""/*referrer*/);
    request.setToken(networkToken);
    if (isInitial)
        request.setMustHandleInternally(true);
    request.setHTTPMethod(method);
    request.setCachePolicy(toWebCoreCachePolicy(cachePolicy));
    request.setTargetType(ResourceRequest::TargetIsMainFrame);

    if (data)
        request.setHTTPBody(FormData::create(data, dataLength));

    for (int i = 0; i + 1 < headersLength; i += 2)
        request.addHTTPHeaderField(headers[i], headers[i + 1]);

    m_mainFrame->loader()->load(request, ""/*name*/, false);
}

void WebPagePrivate::stopCurrentLoad()
{
    // This function should contain all common code triggered by WebPage::load
    // (which stops any load in progress before starting the new load) and
    // WebPage::stoploading (the entry point for the client to stop the load
    // explicitly). If it should only be done while stopping the load
    // explicitly, it goes in WebPage::stopLoading, not here.
    m_mainFrame->loader()->stopAllLoaders();

    // cancel any deferred script that hasn't been processed yet
    FrameLoaderClientOlympia* frameLoaderClient = static_cast<FrameLoaderClientOlympia*>(m_mainFrame->loader()->client());
    frameLoaderClient->setDeferredManualScript(KURL());
}

void WebPagePrivate::setLoadState(LoadState state)
{
    if (m_loadState == state)
        return;

    bool isFirstLoad = m_loadState == None;

    // See RIM Bug #1068
    if (state == Finished && m_mainFrame && m_mainFrame->document())
        m_mainFrame->document()->updateStyleIfNeeded();

    m_loadState = state;

#if DEBUG_WEBPAGE_LOAD
    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "WebPagePrivate::setLoadState %d", state);
#endif

    switch (m_loadState) {
    case Provisional:
        if (isFirstLoad) {
            // Paints the visible backingstore as white to prevent initial checkerboard on
            // the first blit
            m_backingStore->d->renderVisibleContents(false /*renderContentOnly*/);
        }
        break;
    case Committed:
        {
            unscheduleZoomAboutPoint();
            unscheduleAllDeferrableTimers();

            m_previousContentsSize = IntSize();
            m_backingStore->d->clearRenderQueue();
            m_backingStore->d->resetTiles(true /*resetBackground*/);
            m_userPerformedManualZoom = false;
            m_userPerformedManualScroll = false;
            m_userScalable = m_webSettings->isUserScalable();
            m_shouldUseFixedDesktopMode = false;
            resetScales();
            if (m_resetVirtualViewportOnCommitted) { // for DRT
                m_virtualViewportWidth = 0;
                m_virtualViewportHeight = 0;
            }
            if (m_webSettings->viewportWidth() > 0) {
                m_virtualViewportWidth = m_webSettings->viewportWidth();
                m_virtualViewportHeight = m_defaultLayoutSize.height();
            }
            m_viewportArguments = ViewportArguments();

            // If it's a outmost SVG document, we use FixedDesktop mode, otherwise
            // we default to Mobile mode. For example, using FixedDesktop mode to
            // render http://www.croczilla.com/bits_and_pieces/svg/samples/tiger/tiger.svg
            // is user-experience friendly.
            if (m_page->mainFrame()->document()->isSVGDocument()) {
                setShouldUseFixedDesktopMode(true);
                setViewMode(FixedDesktop);
            } else
                setViewMode(Mobile);

            // Reset block zoom and reflow
            resetBlockZoom();
#if ENABLE(VIEWPORT_REFLOW)
            toggleTextReflowIfTextReflowEnabledOnlyForBlockZoom();
#endif

            // Set the scroll to origin here and notify the client since we'll be
            // zooming below without any real contents yet thus the contents size
            // we report to the client could make our current scroll position invalid
            setScrollPosition(IntPoint(0, 0));
            notifyTransformedScrollChanged();

            // Paints the visible backingstore as white.  Note it is important we do
            // this strictly after re-setting the scroll position to origin and resetting
            // the scales otherwise the visible contents calculation is wrong and we
            // can end up blitting artifacts instead. See: RIM Bug #401
            m_backingStore->d->renderVisibleContents(false /*renderContentOnly*/);

            zoomToInitialScaleOnLoad();

            // Update cursor status
            updateCursor();
            break;
        }
    case Finished:
    case Failed:
        // notify client of the initial zoom change
        m_client->zoomChanged(m_webPage->isMinZoomed(), m_webPage->isMaxZoomed(), !shouldZoomOnEscape(), currentScale());
        m_backingStore->d->updateTiles(true /*updateVisible*/, false /*immediate*/);
        break;
    default:
        break;
    }
}

bool WebPagePrivate::shouldZoomAboutPoint(double scale, const WebCore::FloatPoint&, bool enforceScaleClamping, double* clampedScale)
{
    if (!m_mainFrame->view())
        return false;

    if (enforceScaleClamping) {
        if (scale < minimumScale())
            scale = minimumScale();
        else if (scale > maximumScale())
            scale = maximumScale();
    }

    ASSERT(clampedScale);
    *clampedScale = scale;

    if (currentScale() == scale) {
        // Make sure backingstore updates resume from pinch zoom in the case where the final zoom level doesn't change
        m_backingStore->d->resumeScreenUpdates(BackingStorePrivate::None);
        m_client->zoomChanged(m_webPage->isMinZoomed(), m_webPage->isMaxZoomed(), !shouldZoomOnEscape(), currentScale());
        return false;
    }

    return true;
}

bool WebPagePrivate::zoomAboutPoint(double unclampedScale, const WebCore::FloatPoint& anchor, bool enforceScaleClamping, bool forceRendering)
{
    // The reflow and block zoom stuff here needs to happen regardless of
    // whether we shouldZoomAboutPoint.
#if ENABLE(VIEWPORT_REFLOW)
    toggleTextReflowIfTextReflowEnabledOnlyForBlockZoom();
    if (m_page->settings()->isTextReflowEnabled() && m_mainFrame->view())
        setNeedsLayout();
#endif

    resetBlockZoom();

    double scale;
    if (!shouldZoomAboutPoint(unclampedScale, anchor, enforceScaleClamping, &scale)) {
        if (m_webPage->settings()->textReflowMode() == WebSettings::TextReflowEnabled) {
            m_currentPinchZoomNode = 0;
            m_currentPinchZoomCaretPosition.clear();
        }
        return false;
    }
    TransformationMatrix zoom;
    zoom.scale(scale);

#if DEBUG_WEBPAGE_LOAD
    if (loadState() < Finished)
        Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "WebPagePrivate::zoomAboutPoint scale %f anchor (%f, %f)", scale, anchor.x(), anchor.y());
#endif

    // Our current scroll position in float
    FloatPoint scrollPosition = this->scrollPosition();

    // Anchor offset from scroll position in float
    FloatPoint anchorOffset(anchor.x() - scrollPosition.x(), anchor.y() - scrollPosition.y());

    // The horizontal scaling factor and vertical scaling factor should be equal
    // to preserve aspect ratio of content
    ASSERT(m_transformationMatrix->m11() == m_transformationMatrix->m22());

    // Need to invert the previous transform to anchor the viewport
    double inverseScale = scale / m_transformationMatrix->m11();

    // Actual zoom
    *m_transformationMatrix = zoom;

    // Suspend all screen updates to the backingstore
    m_backingStore->d->suspendScreenUpdates();

    updateViewportSize();

    IntPoint newScrollPosition(IntPoint(max(0, static_cast<int>(roundf(anchor.x() - anchorOffset.x() / inverseScale))),
                                        max(0, static_cast<int>(roundf(anchor.y() - anchorOffset.y() / inverseScale)))));

    if (m_webPage->settings()->textReflowMode() == WebSettings::TextReflowEnabled) {
        // This is a hack for email which has reflow always turned on
        m_mainFrame->view()->setNeedsLayout();
        requestLayoutIfNeeded();

        if (scale != minimumScale()) {
            IntRect reflowedRect = rectForNode(m_currentPinchZoomNode.get());
            reflowedRect = adjustRectOffsetForFrameOffset(reflowedRect, m_currentPinchZoomNode.get());
            int caretOffset = 0;
            if (!m_currentPinchZoomCaretPosition.isNull() && m_currentPinchZoomNode.get()) {
                RenderObject* renderer = m_currentPinchZoomNode->renderer();
                if (renderer->isText())
                    caretOffset = m_currentPinchZoomCaretPosition.localCaretRect(renderer).y();
            }
            newScrollPosition = IntPoint(max(0, static_cast<int>(roundf(reflowedRect.x()))),
                                         max(0, static_cast<int>(roundf(reflowedRect.y() + caretOffset - anchorOffset.y() / inverseScale))));
        }

        m_currentPinchZoomNode = 0;
        m_currentPinchZoomCaretPosition.clear();
    }

    setScrollPosition(newScrollPosition);

    notifyTransformChanged();

    bool isLoading = this->isLoading();

    // We need to invalidate all tiles both visible and non-visible if we're loading
    m_backingStore->d->updateTiles(isLoading /*updateVisible*/, false /*immediate*/);

    m_bitmapScale = m_transformationMatrix->m11();

    bool shouldRender = !isLoading || m_userPerformedManualZoom || forceRendering;
    bool shouldClearVisibleZoom = isLoading && shouldRender;

    if (shouldClearVisibleZoom) {
        // If we are loading and rendering then we need to clear the render queue's
        // visible zoom jobs as they will be irrelevant with the render below
        m_backingStore->d->clearVisibleZoom();
    }

    // Clear canvas to make sure there are no artifacts
    if (shouldRender) {
        m_client->lockCanvas();
        m_client->clearCanvas();
        // Resume all screen updates to the backingstore and render+blit visible contents to screen
        m_backingStore->d->resumeScreenUpdates(BackingStorePrivate::RenderAndBlit);
        m_client->unlockCanvas();
    } else {
        // Resume all screen updates to the backingstore but do not blit to the screen because we not rendering
        m_backingStore->d->resumeScreenUpdates(BackingStorePrivate::None);
    }

    m_client->zoomChanged(m_webPage->isMinZoomed(), m_webPage->isMaxZoomed(), !shouldZoomOnEscape(), currentScale());

    return true;
}

bool WebPagePrivate::scheduleZoomAboutPoint(double unclampedScale, const FloatPoint& anchor, bool enforceScaleClamping, bool forceRendering)
{
    double scale;
    if (!shouldZoomAboutPoint(unclampedScale, anchor, enforceScaleClamping, &scale)) {
        // We could be back to the right zoom level before the timer has
        // timed out, because of wiggling back and forth. Stop the timer.
        unscheduleZoomAboutPoint();
        return false;
    }

    // Prohibit backingstore from updating the canvas overtop of the bitmap
    m_backingStore->d->suspendScreenUpdates();

    IntSize requiredSize = transformedVisibleContentsRect().size();
    unsigned short* tempSurface = new unsigned short[requiredSize.width() * requiredSize.height() * 2];
    unsigned int tempSurfaceStride = requiredSize.width() * 2;
    // For some reason, renderBitmapZoomFrame wants an anchor in backingstore coordinates!
    // this is different from zoomAboutPoint, which wants content coordinates.
    // See RIM Bug #641
    renderBitmapZoomFrame(scale, mapToTransformedFloatPoint(anchor), tempSurface, tempSurfaceStride);
    delete [] tempSurface;

    m_delayedZoomArguments.scale = scale;
    m_delayedZoomArguments.anchor = anchor;
    m_delayedZoomArguments.enforceScaleClamping = enforceScaleClamping;
    m_delayedZoomArguments.forceRendering = forceRendering;
    m_delayedZoomTimer->startOneShot(delayedZoomInterval);

    return true;
}

void WebPagePrivate::unscheduleZoomAboutPoint()
{
    if (m_delayedZoomTimer->isActive())
        m_backingStore->d->resumeScreenUpdates(BackingStorePrivate::None);

    m_delayedZoomTimer->stop();
}

void WebPagePrivate::zoomAboutPointTimerFired(WebCore::Timer<WebPagePrivate>*)
{
    zoomAboutPoint(m_delayedZoomArguments.scale, m_delayedZoomArguments.anchor, m_delayedZoomArguments.enforceScaleClamping, m_delayedZoomArguments.forceRendering);
}

void WebPagePrivate::setNeedsLayout()
{
    FrameView* view = m_mainFrame->view();
    ASSERT(view);
    view->setNeedsLayout();
}

void WebPagePrivate::requestLayoutIfNeeded() const
{
    FrameView* view = m_mainFrame->view();
    ASSERT(view);
    view->layoutIfNeededRecursive();
    ASSERT(!view->needsLayout());
}

void WebPagePrivate::renderContents(SurfaceOpenVG* surface, BackingStoreTile* tile, const IntPoint& surfaceOffset, const IntRect& contentsRect) const
{
    if (!m_webPage->isVisible())
        return;

    // It is up to callers of this method to perform layout themselves!
    ASSERT(!m_mainFrame->view()->needsLayout());

    IntSize contentsSize = this->contentsSize();

    if (contentsSize.isEmpty() || !IntRect(IntPoint(0, 0), transformedContentsSize()).contains(contentsRect)) {
        // If the contentsSize is empty then we're just painting our
        // default background color by clearing the image area to white.
        surface->makeCurrent();

        const VGfloat white[4] = { 1.0, 1.0, 1.0, 1.0 };

        vgSetfv(VG_CLEAR_COLOR, 4, white);
        vgClear(contentsRect.x() - surfaceOffset.x(), contentsRect.y() - surfaceOffset.y(),
            contentsRect.width(), contentsRect.height());
        ASSERT_VG_NO_ERROR();
    }

    if (!contentsSize.isEmpty()) {
        GraphicsContext graphicsContext(surface);

        // Translate context according to offset
        graphicsContext.translate(-surfaceOffset.x(), -surfaceOffset.y());

        // Add our transformation matrix as the global transform
        AffineTransform affineTransform(
            m_transformationMatrix->a(), m_transformationMatrix->b(),
            m_transformationMatrix->c(), m_transformationMatrix->d(),
            m_transformationMatrix->e(), m_transformationMatrix->f());
        graphicsContext.concatCTM(affineTransform);

        // Now that the matrix is applied we need untranformed contents coordinates
        IntRect untransformedContentsRect = mapFromTransformed(contentsRect);

        // We extract from the contentsRect but draw a slightly larger region than
        // we were told to, in order to avoid pixels being rendered only partially.
        const int atLeastOneDevicePixel = static_cast<int>(ceilf(1.0 / m_transformationMatrix->a()));
        untransformedContentsRect.inflate(atLeastOneDevicePixel);

        // Make sure the untransformed rectangle for the (slightly larger than
        // initially requested) repainted region is within the bounds of the page.
        untransformedContentsRect.intersect(IntRect(IntPoint(0, 0), contentsSize));

        // Let WebCore render the page contents into the drawing surface.
        m_mainFrame->view()->paintContents(&graphicsContext, untransformedContentsRect);
    }

    // Grab the requested region from the drawing surface into the tile image.
    surface->makeCurrent();

    const int dstX = contentsRect.x() - surfaceOffset.x();
    const int dstY = contentsRect.y() - surfaceOffset.y();
    const int srcX = dstX;
    const int srcY = dstY;
    vgReadPixels(tile->image() + (dstY * tile->imageStride()) + (dstX * tile->imageBytesPerPixel()),
        tile->imageStride(), VG_sRGB_565,
        srcX, srcY, contentsRect.width(), contentsRect.height());
    ASSERT_VG_NO_ERROR();
}

void WebPagePrivate::blitToCanvas(const unsigned char* image, int imageStride, const IntRect& dst, const IntRect& src)
{
    IntRect dstRect = dst;
    IntRect srcRect = src;

    IntRect viewPortRect(IntPoint(0, 0), transformedViewportSize());
    dstRect.intersect(viewPortRect);
    srcRect.intersect(IntRect(srcRect.location(), dstRect.size()));

    m_client->blitToCanvas(dstRect.x(), dstRect.y(), image, imageStride,
        srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height());
}

void WebPagePrivate::blitFromBufferToBuffer(unsigned char* dst, const int dstStride, const WebCore::IntRect& dstRect,
                                            const unsigned char* src, const int srcStride, const WebCore::IntRect& srcRect)
{
    m_client->blitFromBufferToBuffer(dst, dstStride, dstRect.x(), dstRect.y(), src, srcStride, srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height());
}

void WebPagePrivate::stretchBlitToCanvas(const unsigned char* src, int srcStride, const WebCore::IntRect& dstRect, const WebCore::IntRect& srcRect)
{
    m_client->stretchBlitToCanvas(src, srcStride,
        srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height(),
        dstRect.x(), dstRect.y(), dstRect.width(), dstRect.height());
}

void WebPagePrivate::stretchBlitFromBufferToBuffer(unsigned char* dst, const int dstStride, const WebCore::IntRect& dstRect,
                                            const unsigned char* src, const int srcStride, const WebCore::IntRect& srcRect)
{
    m_client->stretchBlitFromBufferToBuffer(dst, dstStride, dstRect, src, srcStride, srcRect);
}

void WebPagePrivate::blendOntoCanvas(const unsigned char* image, int imageStride,
                                     const unsigned char* alphaImage, int alphaImageStride, char globalAlpha,
                                     const IntRect& dst, const IntRect& src)
{
    IntRect dstRect = dst;
    IntRect srcRect = src;

    IntRect viewPortRect(IntPoint(0, 0), transformedViewportSize());
    dstRect.intersect(viewPortRect);
    srcRect.intersect(IntRect(srcRect.location(), dstRect.size()));

    m_client->blendOntoCanvas(dstRect.x(), dstRect.y(),
        image, imageStride, alphaImage, alphaImageStride, globalAlpha,
        srcRect.x(), srcRect.y(), srcRect.width(), srcRect.height());
}

void WebPagePrivate::invalidateWindow(const WebCore::IntRect& dst)
{
    IntRect dstRect = dst;

    IntRect viewPortRect(IntPoint(0, 0), transformedViewportSize());
    dstRect.intersect(viewPortRect);

    m_client->invalidateWindow(dstRect.x(), dstRect.y(), dstRect.width(), dstRect.height());
}

void WebPagePrivate::clearCanvas(const WebCore::IntRect& rect, unsigned short color)
{
    m_client->clearCanvas(rect, color);
}

void WebPagePrivate::clearBuffer(unsigned char* buffer, const int stride, const WebCore::IntRect& rect, unsigned short color)
{
    m_client->clearBuffer(buffer, stride, rect, color);
}

WebCore::IntPoint WebPagePrivate::scrollPosition() const
{
    ASSERT(m_mainFrame->view());
    return m_mainFrame->view()->scrollPosition();
}

WebCore::IntPoint WebPagePrivate::maximumScrollPosition() const
{
    ASSERT(m_mainFrame->view());
    return m_mainFrame->view()->maximumScrollPosition();
}

void WebPagePrivate::setScrollPosition(const WebCore::IntPoint& pos)
{
    ASSERT(m_mainFrame->view());
    if (pos == scrollPosition())
        return;

#if DEBUG_WEBPAGE_LOAD
    if (loadState() < Finished)
        Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "WebPagePrivate::setScrollPosition New: (%d, %d) != Old: (%d, %d)", pos.x(), pos.y(), scrollPosition().x(), scrollPosition().y());
#endif

    // We set a flag here to note that this scroll operation was originated within
    // the Olympia layer of WebKit and not by WebCore.  This flag is checked in
    // checkOriginOfCurrentScrollOperation() to decide whether to notify the client
    // of the current scroll operation. This is why it is important that all scroll
    // operations that originate within Olympia are encapsulated here and that callers
    // of this method also directly or indirectly call notifyTransformedScrollChanged().

    m_olympiaOriginatedScroll = true;
    m_mainFrame->view()->setScrollPosition(pos);
    scheduleDeferrableTimer(m_deferredScrollEventTimer.get(), deferredScrollEventInterval);
    m_olympiaOriginatedScroll = false;
}

void WebPagePrivate::scrollEventTimerFired(Timer<WebPagePrivate>*)
{
    m_mainFrame->view()->scrollPositionChanged();
}

void WebPagePrivate::resizeEventTimerFired(Timer<WebPagePrivate>*)
{
    if (FrameView* view = m_mainFrame->view()) {
        if (view->shouldSendResizeEvent())
            m_mainFrame->eventHandler()->sendResizeEvent();

        // When the actual visible size changes, we also
        // need to reposition fixed elements
        view->repaintFixedElementsAfterScrolling();
    }
}

void WebPagePrivate::scheduleDeferrableTimer(Timer<WebPagePrivate>* timer, double timeOut)
{
    if (m_page->defersLoading()) {
        m_deferredTimers.append(timer); // Will fire when we resume
        return;
    }

    timer->startOneShot(timeOut);
}

void WebPagePrivate::unscheduleAllDeferrableTimers()
{
    for (Vector<Timer<WebPagePrivate>* >::const_iterator it = m_deferrableTimers.begin(); it != m_deferrableTimers.end(); ++it)
        (*it)->stop();
}

void WebPagePrivate::willDeferLoading()
{
    for (Vector<Timer<WebPagePrivate>* >::const_iterator it = m_deferrableTimers.begin();
         it != m_deferrableTimers.end();
         ++it) {
        if ((*it)->isActive()) {
            (*it)->stop();
            m_deferredTimers.append(*it);
        }
    }

    m_client->willDeferLoading();
}

void WebPagePrivate::didResumeLoading()
{
    m_client->didResumeLoading();

    for (Vector<Timer<WebPagePrivate>* >::const_iterator it = m_deferredTimers.begin();
         it != m_deferredTimers.end();
         ++it) {
        (*it)->startOneShot(0); // We're not concerned with keeping the time interval here
    }
    m_deferredTimers.clear();
}

void WebPagePrivate::scrollBy(int deltaX, int deltaY)
{
    setScrollPosition(scrollPosition() + IntSize(deltaX, deltaY));
}

void WebPagePrivate::checkOriginOfCurrentScrollOperation()
{
    // This is called via ChromeClientOlympia::scroll in order to check the origin
    // of the current scroll operation to decide whether to notify the client.
    // If the current scroll operation was initiated internally by WebCore itself
    // either via JavaScript, back/forward or otherwise then we need to go ahead
    // and notify the client of this change.
    if (!m_olympiaOriginatedScroll)
        notifyTransformedScrollChanged();
}

WebCore::IntSize WebPagePrivate::viewportSize() const
{
    return mapFromTransformed(transformedViewportSize());
}

WebCore::IntSize WebPagePrivate::actualVisibleSize() const
{
    return mapFromTransformed(transformedActualVisibleSize());
}

bool WebPagePrivate::hasVirtualViewport() const
{
    return m_virtualViewportWidth && m_virtualViewportHeight;
}

void WebPagePrivate::updateViewportSize()
{
    ASSERT(m_mainFrame->view());
    m_mainFrame->view()->setFixedReportedSize(actualVisibleSize());

    IntRect frameRect = IntRect(scrollPosition(), viewportSize());
    if (frameRect == m_mainFrame->view()->frameRect()) {
        // We're going to need to send a resize event to JS because
        // innerWidth and innerHeight depend on fixed reported size.
        // This is a way to resize pages where JavaScript resizes
        // the visible page.
        scheduleDeferrableTimer(m_deferredResizeEventTimer.get(), deferredResizeEventInterval);
        return;
    }

    m_mainFrame->view()->setFrameRect(frameRect);
    m_mainFrame->view()->adjustViewSize();
}

WebCore::FloatPoint WebPagePrivate::centerOfVisibleContentsRect() const
{
    // The visible contents rect in float
    FloatRect visibleContentsRect = this->visibleContentsRect();

    // The center of the visible contents rect in float
    return FloatPoint(visibleContentsRect.x() + visibleContentsRect.width() / 2.0,
                      visibleContentsRect.y() + visibleContentsRect.height() / 2.0);
}

WebCore::IntRect WebPagePrivate::visibleContentsRect() const
{
    ASSERT(m_mainFrame->view());
    return m_mainFrame->view()->visibleContentRect();
}

WebCore::IntSize WebPagePrivate::contentsSize() const
{
    ASSERT(m_mainFrame->view());
    return m_mainFrame->view()->contentsSize();
}

WebCore::IntSize WebPagePrivate::absoluteVisibleOverflowSize() const
{
    if (!m_mainFrame->contentRenderer())
        return IntSize();
    return IntSize(m_mainFrame->contentRenderer()->rightAbsoluteVisibleOverflow(), m_mainFrame->contentRenderer()->bottomAbsoluteVisibleOverflow());
}

void WebPagePrivate::contentsSizeChanged(const WebCore::IntSize& contentsSize)
{
    if (m_previousContentsSize == contentsSize)
        return;

    // This should only occur in the middle of layout so we set a flag here and
    // handle it at the end of the layout
    m_contentsSizeChanged = true;

#if DEBUG_WEBPAGE_LOAD
    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "WebPagePrivate::contentsSizeChanged %dx%d", contentsSize.width(), contentsSize.height());
#endif
}

void WebPagePrivate::overflowExceedsContentsSize()
{
    m_overflowExceedsContentsSize = true;
}

void WebPagePrivate::layoutFinished()
{
    if (!m_contentsSizeChanged && !m_overflowExceedsContentsSize)
        return;

    m_contentsSizeChanged = false; // toggle to turn off notification again
    m_overflowExceedsContentsSize = false;

    if (contentsSize().isEmpty())
        return;

    // The call to zoomToInitialScaleOnLoad can cause recursive layout when called from
    // the middle of a layout, but the recursion is limited by detection code in
    // setViewMode() and mitigation code in fixedLayoutSize()
    if (didLayoutExceedMaximumIterations()) {
        notifyTransformedContentsSizeChanged();
        return;
    }

    m_nestedLayoutFinishedCount++;

    if (loadState() == Committed)
        zoomToInitialScaleOnLoad();
    else if (loadState() != None)
        notifyTransformedContentsSizeChanged();

    m_nestedLayoutFinishedCount--;

    if (!m_nestedLayoutFinishedCount) {
        // When the contents height shrinks, there is a risk that we
        // will be left at a scroll position that lies outside of the
        // contents rect. Since we allow overscrolling and neglect
        // to clamp overscroll in order to retain input focus (RIM Bug #414)
        // we need to clamp somewhere, and this is where we know the
        // contents size has changed.
        IntPoint scrollPoint = scrollPosition();
        IntPoint newScrollPosition = scrollPoint;
        newScrollPosition = scrollPoint.shrunkTo(maximumScrollPosition());
        newScrollPosition.clampNegativeToZero();
        setScrollPosition(newScrollPosition);
        if (scrollPoint != scrollPosition())
            notifyTransformedScrollChanged();

        // If we were waiting for more data to continue an fragment scroll, continue it now
        static_cast<FrameLoaderClientOlympia*>(m_mainFrame->loader()->client())->doPendingFragmentScroll();
    }
}

void WebPagePrivate::zoomToInitialScaleOnLoad()
{
#if DEBUG_WEBPAGE_LOAD
    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "WebPagePrivate::zoomToInitialScaleOnLoad");
#endif

    bool needsLayout = false;

    // If a web page would fit in either portrait or landscape mode, use view mode Mobile
    WebSettings* settings = WebSettings::pageGroupSettings(m_page->groupName());
    int maxMobileWidth = std::max(settings->screenWidth(), settings->screenHeight());

    // If the contents width exceeds the viewport width set to desktop mode
    if (m_shouldUseFixedDesktopMode)
        needsLayout = setViewMode(FixedDesktop);
    else if (contentsSize().width() > maxMobileWidth || absoluteVisibleOverflowSize().width() > maxMobileWidth)
        needsLayout = setViewMode(Desktop);
    else
        needsLayout = setViewMode(Mobile);

    if (needsLayout) {
        // This can cause recursive layout...
        setNeedsLayout();
    }

    if (contentsSize().isEmpty()) {
#if DEBUG_WEBPAGE_LOAD
        Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "WebPagePrivate::zoomToInitialScaleOnLoad content is empty!");
#endif
        return;
    }

    bool performedZoom = false;
    bool shouldZoom = !m_userPerformedManualZoom;

    // If this load should restore view state, don't zoom to initial scale
    // but instead let the HistoryItem's saved viewport reign supreme.
    if (m_mainFrame && m_mainFrame->loader() && m_mainFrame->loader()->shouldRestoreScrollPositionAndViewState())
        shouldZoom = false;

    if (shouldZoom && loadState() == Committed) {
        // Preserve at top and at left position, to avoid scrolling
        // to a non top-left position for web page with viewport meta tag
        // that specifies an initial-scale that is zoomed in.
        FloatPoint anchor = centerOfVisibleContentsRect();
        if (scrollPosition().x() == 0)
            anchor.setX(0);
        if (scrollPosition().y() == 0)
            anchor.setY(0);
        performedZoom = zoomAboutPoint(initialScale(), anchor);
    }

    // zoomAboutPoint above can also toggle setNeedsLayout and cause recursive layout...
    requestLayoutIfNeeded();

    if (!performedZoom) {
        // We only notify if we didn't perform zoom, because zoom will notify on
        // its own...
        notifyTransformedContentsSizeChanged();
    }
}

double WebPagePrivate::currentScale() const
{
    return m_transformationMatrix->m11();
}

double WebPagePrivate::zoomToFitScale() const
{
    // We must clamp the contents for this calculation so that we do not allow an
    // arbitrarily small zoomToFitScale much like we clamp the fixedLayoutSize()
    // so that we do not have arbitrarily large layout size.
    // If we have a specified viewport, we may need to be able to zoom out more.
    int contentWidth = std::min(contentsSize().width(), std::max(m_virtualViewportWidth, static_cast<int>(DEFAULT_MAX_LAYOUT_WIDTH)));

    // If we have a virtual viewport and its aspect ratio caused content to layout
    // wider than the default layout aspect ratio we need to zoom to fit the content height
    // in order to avoid showing a grey area below the web page.
    // Without virtual viewport we can never get into this situation.
    if (hasVirtualViewport()) {
        int contentHeight = std::min(contentsSize().height(), std::max(m_virtualViewportHeight, static_cast<int>(DEFAULT_MAX_LAYOUT_HEIGHT)));

        // Aspect ratio check without division
        if (contentWidth*m_defaultLayoutSize.height() > contentHeight*m_defaultLayoutSize.width())
            return contentHeight > 0 ? static_cast<double>(m_defaultLayoutSize.height())/contentHeight : 1.0;
    }

    return contentWidth > 0.0 ? static_cast<double>(m_actualVisibleWidth) / contentWidth : 1.0;
}

double WebPagePrivate::initialScale() const
{
    if (m_initialScale > 0.0)
        return m_initialScale;

    if (m_webSettings->isZoomToFitOnLoad())
        return zoomToFitScale();

    return 1.0;
}

void WebPage::removePluginsFromList()
{
    cleanPluginListFromFrame(d->m_mainFrame);
}

void WebPage::cleanPluginListFromFrame(Frame* frame)
{
    if (!frame)
        return;
    
    for (Frame* childFrame = frame->tree()->firstChild(); childFrame; childFrame = childFrame->tree()->nextSibling())
        cleanPluginListFromFrame(childFrame);

    FrameLoaderClientOlympia* frameLoaderClient = static_cast<FrameLoaderClientOlympia*>(frame->loader()->client());
    frameLoaderClient->cleanPluginList();
}

double WebPage::initialScale() const
{
    return d->initialScale();
}

void WebPage::setInitialScale(double initialScale)
{
    d->setInitialScale(initialScale);
}

double WebPage::minimumScale() const
{
    return d->minimumScale();
}

void WebPage::setMinimumScale(double minimumScale)
{
    d->setMinimumScale(minimumScale);
}

double WebPage::maximumScale() const
{
    return d->maximumScale();
}

void WebPage::setMaximumScale(double maximumScale)
{
    d->setMaximumScale(maximumScale);
}

double WebPagePrivate::minimumScale() const
{
    return (m_minimumScale > zoomToFitScale() && m_minimumScale <= maximumScale()) ? m_minimumScale : zoomToFitScale();
}

double WebPagePrivate::maximumScale() const
{
    return (m_maximumScale >= zoomToFitScale() && m_maximumScale >= m_minimumScale) ? m_maximumScale : 4.0;
}

void WebPagePrivate::resetScales()
{
    TransformationMatrix identity;
    *m_transformationMatrix = identity;
    m_initialScale = m_webSettings->initialScale() > 0 ? m_webSettings->initialScale() : -1.0;
    m_minimumScale = -1.0;
    m_maximumScale = -1.0;

    // We have to let WebCore know about updated framerect now that we've
    // reset our scales.  See: RIM Bug #401
    updateViewportSize();
}

WebCore::IntPoint WebPagePrivate::transformedScrollPosition() const
{
    return mapToTransformed(scrollPosition());
}

WebCore::IntPoint WebPagePrivate::transformedMaximumScrollPosition() const
{
    return mapToTransformed(maximumScrollPosition());
}

WebCore::IntSize WebPagePrivate::transformedActualVisibleSize() const
{
    return IntSize(m_actualVisibleWidth, m_actualVisibleHeight);
}

WebCore::IntSize WebPagePrivate::transformedViewportSize() const
{
    return IntSize(m_width, m_height);
}

WebCore::IntRect WebPagePrivate::transformedVisibleContentsRect() const
{
    // Usually this would be mapToTransformed(visibleContentsRect()), but
    // that results in rounding errors because we already set the WebCore
    // viewport size from our original transformedViewportSize().
    // Instead, we only transform the scroll position and take the
    // viewport size as it is, which ensures that e.g. blitting operations
    // always cover the whole widget/screen.
    return IntRect(transformedScrollPosition(), transformedViewportSize());
}

WebCore::IntSize WebPagePrivate::transformedContentsSize() const
{
    // mapToTransformed() functions use this method to crop their results,
    // so we can't make use of them here. While we want rounding inside page
    // boundaries to extend rectangles and round points, we need to crop the
    // contents size to the floored values so that we don't try to display
    // or report points that are not fully covered by the actual float-point
    // contents rectangle.
    const WebCore::IntSize untransformedContentsSize = contentsSize();
    const WebCore::FloatPoint transformedBottomRight = m_transformationMatrix->mapPoint(
        WebCore::FloatPoint(untransformedContentsSize.width(), untransformedContentsSize.height()));
    return WebCore::IntSize(floorf(transformedBottomRight.x()), floorf(transformedBottomRight.y()));
}

void WebPagePrivate::notifyPluginRectChanged(int id, const WebCore::IntRect& rectChanged)
{
    // Plugins have to map to visible region on the client, so only do transform here
    WebCore::IntRect transformedRect(mapToTransformed(rectChanged));

    // No need to intersect with content's rect if plugin rect's position is (-1,-1), which means it isn't ready to display.
    if (transformedRect.x() != -1 || transformedRect.y() != -1) 
        clipToTransformedContentsRect(transformedRect);

    m_client->notifyPluginRectChanged(id, Olympia::Platform::IntRect(transformedRect));
}

WebCore::IntPoint WebPagePrivate::mapFromContentsToViewport(const WebCore::IntPoint& point) const
{
    const IntPoint scrollPosition = this->scrollPosition();
    return IntPoint(point.x() - scrollPosition.x(), point.y() - scrollPosition.y());
}

WebCore::IntPoint WebPagePrivate::mapFromViewportToContents(const WebCore::IntPoint& point) const
{
    const IntPoint scrollPosition = this->scrollPosition();
    return IntPoint(point.x() + scrollPosition.x(), point.y() + scrollPosition.y());
}

WebCore::IntRect WebPagePrivate::mapFromContentsToViewport(const WebCore::IntRect& rect) const
{
    return IntRect(mapFromContentsToViewport(rect.location()), rect.size());
}

WebCore::IntRect WebPagePrivate::mapFromViewportToContents(const WebCore::IntRect& rect) const
{
    return IntRect(mapFromViewportToContents(rect.location()), rect.size());
}

WebCore::IntPoint WebPagePrivate::mapFromTransformedContentsToTransformedViewport(const WebCore::IntPoint& point) const
{
    const IntPoint scrollPosition = this->transformedScrollPosition();
    return IntPoint(point.x() - scrollPosition.x(), point.y() - scrollPosition.y());
}

WebCore::IntPoint WebPagePrivate::mapFromTransformedViewportToTransformedContents(const WebCore::IntPoint& point) const
{
    const IntPoint scrollPosition = this->transformedScrollPosition();
    return IntPoint(point.x() + scrollPosition.x(), point.y() + scrollPosition.y());
}

WebCore::IntRect WebPagePrivate::mapFromTransformedContentsToTransformedViewport(const WebCore::IntRect& rect) const
{
    return IntRect(mapFromTransformedContentsToTransformedViewport(rect.location()), rect.size());
}

WebCore::IntRect WebPagePrivate::mapFromTransformedViewportToTransformedContents(const WebCore::IntRect& rect) const
{
    return IntRect(mapFromTransformedViewportToTransformedContents(rect.location()), rect.size());
}

/*
 * NOTE: PIXEL ROUNDING!
 * Accurate back-and-forth rounding is not possible with information loss
 * by integer points and sizes, so we always expand the resulting mapped
 * float rectangles to the nearest integer. For points, we always use
 * floor-rounding in mapToTransformed() so that we don't have to crop to
 * the (floor'd) transformed contents size.
 */
static inline WebCore::IntPoint roundTransformedPoint(const WebCore::FloatPoint &point)
{
    // Maps by rounding half towards zero
    return IntPoint(static_cast<int>(floorf(point.x())), static_cast<int>(floorf(point.y())));
}

static inline WebCore::IntPoint roundUntransformedPoint(const WebCore::FloatPoint &point)
{
    // Maps by rounding half away from zero
    return IntPoint(static_cast<int>(ceilf(point.x())), static_cast<int>(ceilf(point.y())));
}

WebCore::IntPoint WebPagePrivate::mapToTransformed(const WebCore::IntPoint& point) const
{
    return roundTransformedPoint(m_transformationMatrix->mapPoint(FloatPoint(point)));
}

WebCore::FloatPoint WebPagePrivate::mapToTransformedFloatPoint(const WebCore::FloatPoint& point) const
{
    return m_transformationMatrix->mapPoint(point);
}

WebCore::IntPoint WebPagePrivate::mapFromTransformed(const WebCore::IntPoint& point) const
{
    return roundUntransformedPoint(m_transformationMatrix->inverse().mapPoint(FloatPoint(point)));
}

WebCore::FloatPoint WebPagePrivate::mapFromTransformedFloatPoint(const WebCore::FloatPoint& point) const
{
    return m_transformationMatrix->inverse().mapPoint(point);
}

WebCore::IntSize WebPagePrivate::mapToTransformed(const WebCore::IntSize& size) const
{
    return mapToTransformed(IntRect(IntPoint(0, 0), size)).size();
}

WebCore::IntSize WebPagePrivate::mapFromTransformed(const WebCore::IntSize& size) const
{
    return mapFromTransformed(IntRect(IntPoint(0, 0), size)).size();
}

WebCore::IntRect WebPagePrivate::mapToTransformed(const WebCore::IntRect& rect) const
{
    return enclosingIntRect(m_transformationMatrix->mapRect(FloatRect(rect)));
}

// Use this in conjunction with mapToTransformed(IntRect), in most cases.
void WebPagePrivate::clipToTransformedContentsRect(WebCore::IntRect& rect) const
{
    rect.intersect(IntRect(IntPoint(0, 0), transformedContentsSize()));
}

WebCore::IntRect WebPagePrivate::mapFromTransformed(const WebCore::IntRect& rect) const
{
    return enclosingIntRect(m_transformationMatrix->inverse().mapRect(FloatRect(rect)));
}

bool WebPagePrivate::transformedPointEqualsUntransformedPoint(const WebCore::IntPoint& transformedPoint, const WebCore::IntPoint& untransformedPoint)
{
    // Scaling down is always more accurate than scaling up.
    if (m_transformationMatrix->a() > 1.0)
        return transformedPoint == mapToTransformed(untransformedPoint);

    return mapFromTransformed(transformedPoint) == untransformedPoint;
}

void WebPagePrivate::notifyTransformChanged()
{
    notifyTransformedContentsSizeChanged();
    notifyTransformedScrollChanged();

    m_backingStore->d->transformChanged();
    m_selectionHandler->selectionPositionChanged();
}

void WebPagePrivate::notifyTransformedContentsSizeChanged()
{
    // We mark here as the last reported content size we sent to the client
    m_previousContentsSize = contentsSize();

    const IntSize size = transformedContentsSize();
    m_backingStore->d->contentsSizeChanged(size);
    m_client->contentsSizeChanged(size.width(), size.height());
}

void WebPagePrivate::notifyTransformedScrollChanged()
{
    const IntPoint pos = transformedScrollPosition();
    m_backingStore->d->scrollChanged(pos);
    m_client->scrollChanged(pos.x(), pos.y());
}

bool WebPagePrivate::setViewMode(ViewMode mode)
{
    if (!m_mainFrame->view())
        return false;

    m_viewMode = mode;

    // If we're in the middle of a nested layout with a recursion count above
    // some maximum threshold, then our algorithm for finding the minimum content
    // width of a given page has become dependent on the visible width.
    //
    // We need to find some method to ensure that we don't experience excessive
    // and even infinite recursion.  This can even happen with valid html.  The
    // former can happen when we run into inline text with few candidates for line
    // break. The latter can happen for instance if the page has a negative margin
    // set against the right border.  Note: this is valid by spec and can lead to
    // a situation where there is no value for which the content width will ensure
    // no horizontal scrollbar.
    // Example: LayoutTests/css1/box_properties/margin.html
    //
    // In order to address such situations when we detect a recursion above some
    // maximum threshold we snap our fixed layout size to a defined quantum increment.
    // Eventually, either the content width will be satisfied to ensure no horizontal
    // scrollbar or this increment will run into the maximum layout size and the
    // recursion will necessarily end.
    bool snapToIncrement = didLayoutExceedMaximumIterations();

    IntSize currentSize = m_mainFrame->view()->fixedLayoutSize();
    IntSize newSize = fixedLayoutSize(snapToIncrement);
    if (currentSize == newSize)
        return false;

    // FIXME: Temp solution. We'll get back to this.
    if (m_nestedLayoutFinishedCount) {
        double widthChange = fabs(double(newSize.width() - currentSize.width()) / currentSize.width());
        double heightChange = fabs(double(newSize.height() - currentSize.height()) / currentSize.height());
        if (widthChange < 0.05 && heightChange < 0.05)
            return false;
    }

    m_mainFrame->view()->setUseFixedLayout(useFixedLayout());
    m_mainFrame->view()->setFixedLayoutSize(newSize);
    return true; // needs re-layout!
}

void WebPagePrivate::setCursor(WebCore::PlatformCursorHandle handle)
{
    if (m_currentCursor.type() != handle.type()) {
        m_currentCursor = handle;
        m_client->cursorChanged(handle.type(), handle.url().c_str(), handle.hotspot().x(), handle.hotspot().y());
    }
}

Olympia::Platform::NetworkStreamFactory* WebPagePrivate::networkStreamFactory()
{
    return m_client->networkStreamFactory();
}

WebCore::FloatRect WebPagePrivate::screenAvailableRect()
{
    WebSettings* settings = WebSettings::pageGroupSettings(m_page->groupName());
    return FloatRect(0, 0, settings->screenWidth(), settings->screenHeight());
}

WebCore::FloatRect WebPagePrivate::screenRect()
{
    WebSettings* settings = WebSettings::pageGroupSettings(m_page->groupName());
    return FloatRect(0, 0, settings->screenWidth(), settings->screenHeight());
}


bool WebPagePrivate::didNodeOpenPopup(Node* node)
{
    if (!node)
        return false;

    if (node->hasTagName(HTMLNames::selectTag))
        return openSelectPopup(static_cast<HTMLSelectElement*>(node));

    if (node->hasTagName(HTMLNames::optionTag)) {
        HTMLOptionElement* optionElement = static_cast<HTMLOptionElement*>(node);
        return openSelectPopup(optionElement->ownerSelectElement());
    }

    if (node && node->hasTagName(HTMLNames::inputTag))
        return openLineInputPopup(static_cast<HTMLInputElement*>(node));

    return false;
}

bool WebPagePrivate::openLineInputPopup(HTMLInputElement* input)
{
    DateTimePopupType type;
    switch (input->inputType()) {
    case HTMLInputElement::DATE:
        type = Date;
        break;
    case HTMLInputElement::DATETIME:
        type = DateTime;
        break;
    case HTMLInputElement::DATETIMELOCAL:
        type = DateTimeLocal;
        break;
    case HTMLInputElement::MONTH:
        type = Month;
        break;
    case HTMLInputElement::TIME:
        type = Time;
        break;
    case HTMLInputElement::WEEK:
        type = Week;
        break;
    case HTMLInputElement::COLOR:
        m_colorInput = input;
        m_client->openColorPopup(input->value());
        return true;
    default:
        return false;
    }
    m_dateTimeInput = input;
    String value = input->value();
    String min = input->getAttribute(HTMLNames::minAttr).string();
    String max = input->getAttribute(HTMLNames::maxAttr).string();
    double step = input->getAttribute(HTMLNames::stepAttr).toDouble();
    m_client->openDateTimePopup(type, value, min, max, step);
    return true;
}

bool WebPagePrivate::openSelectPopup(HTMLSelectElement* select)
{
    if (!select)
        return false;

    m_selectElement = select;
    const WTF::Vector<Element*>& listItems = m_selectElement->listItems();
    int size = listItems.size();
    if (!size)
        return false;

    bool multiple = m_selectElement->multiple();

    ScopeArray<String> labels;
    labels.reset(new String[size]);
    bool* enableds = new bool[size];
    int* itemTypes = new int[size];
    bool* selecteds = new bool[size];

    for (int i = 0; i < size; i++) {
        if (listItems[i]->hasTagName(HTMLNames::optionTag)) {
            HTMLOptionElement* option = static_cast<HTMLOptionElement*>(listItems[i]);
            labels[i] = option->textIndentedToRespectGroupLabel();
            enableds[i] = option->disabled() ? 0 : 1;
            selecteds[i] = option->selected();
            itemTypes[i] = TypeOption;
        } else if (listItems[i]->hasTagName(HTMLNames::optgroupTag)) {
            HTMLOptGroupElement* optGroup = static_cast<HTMLOptGroupElement*>(listItems[i]);
            labels[i] = optGroup->groupLabelText();
            enableds[i] = false;
            selecteds[i] = false;
            itemTypes[i] = TypeGroup;
        } else if (listItems[i]->hasTagName(HTMLNames::hrTag)) {
            enableds[i] = false;
            selecteds[i] = false;
            itemTypes[i] = TypeSeparator;
        }
    }

    m_client->openPopupList(multiple, size, labels, enableds, itemTypes, selecteds);
    return true;
}

void WebPagePrivate::setColorInput(const WebCore::String& value)
{
    if (m_colorInput)
        m_colorInput->setValue(value);
}

void WebPagePrivate::setDateTimeInput(const WebCore::String& value)
{
    if (m_dateTimeInput)
        m_dateTimeInput->setValue(value);
}

void WebPagePrivate::setPopupListIndex(int index)
{
    if (index == -2) // Abandon
        return;

    if (!m_selectElement)
        return;

    RenderObject* renderer = m_selectElement->renderer();
    if (renderer && renderer->isMenuList()) {
        RenderMenuList* renderMenu = toRenderMenuList(renderer);
        renderMenu->hidePopup();
    }

    int optionIndex = (static_cast<WebCore::SelectElement*>(m_selectElement))->listToOptionIndex(index);
    m_selectElement->setSelectedIndexByUser(optionIndex);
    m_selectElement->dispatchFormControlChangeEvent();
}

void WebPagePrivate::setPopupListIndexes(int size, bool* selecteds)
{
    if (!m_selectElement)
        return;

    const WTF::Vector<Element*>& items = m_selectElement->listItems();
    if (items.size() != size)
        return;

    HTMLOptionElement* option;
    for (int i = 0; i < size; i++) {
        if (items[i]->hasTagName(HTMLNames::optionTag)) {
            option = static_cast<HTMLOptionElement*>(items[i]);
            option->setSelectedState(selecteds[i]);
        }
    }
    // Force repaint because we do not send mouse events to the select element
    // and the element doesn't automatically repaint itself.
    m_selectElement->dispatchFormControlChangeEvent();
    m_selectElement->renderer()->repaint(true);
}

bool WebPagePrivate::useFixedLayout() const
{
    return true;
}

PassRefPtr<Node> WebPagePrivate::contextNode() const
{
    RefPtr<Node> node;
    EventHandler* eventHandler = m_page->mainFrame()->eventHandler();

    if (eventHandler->mousePressed()) {
        node = eventHandler->mousePressNode();
        eventHandler->setMousePressed(false);
    } else if (m_webSettings->doesGetFocusNodeContext())
        node = m_page->focusController()->focusedOrMainFrame()->document()->focusedNode();
    else if (m_currentCursor.type() != Olympia::Platform::CursorNone) {
        HitTestResult result = eventHandler->hitTestResultAtPoint(mapFromViewportToContents(m_lastMouseEvent.pos()), true);
        node = result.innerNode();
    }

    return node.release();
}

Context WebPagePrivate::getContext()
{
    // We're called to get a fresh context
    RefPtr<Node> node = contextNode();
    return getContextForNode(node.get());
}

void WebPagePrivate::sendContextIfChanged(Node* node)
{
    if (node == m_currentContextNode)
        return;

    m_client->contextChanged(getContextForNode(node));
}

Context WebPagePrivate::getContextForNode(Node* node)
{
    // Remember this node.
    m_currentContextNode = node;

    if (!node)
        return Context();

    Node* linkNode = node->enclosingLinkEventParentOrSelf();
    RenderObject* obj = node->renderer();

    std::vector<String> info;
    std::vector<Context::ContextType> types;

    if (!node->canStartSelection()){
        types.push_back(Context::ContextTypeNotSelectable);
        info.push_back("");
    }

    if (linkNode && linkNode->isHTMLElement()) {
        if (!linkNode->canStartSelection()) {
            types.push_back(Context::ContextTypeNotSelectable);
            info.push_back("");
        }

        KURL href;
        if (linkNode->hasTagName(HTMLNames::aTag)) {
            HTMLAnchorElement* anchor = static_cast<HTMLAnchorElement*>(linkNode);
            href = anchor->href();

            types.push_back(Context::ContextTypeText);
            info.push_back(anchor->text());
            obj = 0; // we already have the text, don't try again
        } else if (linkNode->hasTagName(HTMLNames::areaTag)) {
            HTMLAreaElement* area = static_cast<HTMLAreaElement*>(linkNode);
            href = area->href();
        }

        WebCore::String pattern = findPatternStringForUrl(href);
        if (!pattern.isEmpty()) {
            types.push_back(Context::ContextTypePattern);
            info.push_back(pattern);
        } else if (!href.string().isEmpty()) {
            types.push_back(Context::ContextTypeUrl);
            info.push_back(href.string());
        }
    }

    if (node->isHTMLElement() && node->hasTagName(HTMLNames::imgTag)) {
        HTMLImageElement* imageElement = static_cast<HTMLImageElement*>(node);
        // FIXME: At the mean time, we only show "Save Image" when the image data is available.
        if (CachedResource* cachedResource = imageElement->cachedImage()) {
            if (cachedResource->isLoaded() && cachedResource->data())
                types.push_back(Context::ContextTypeImageSrc);
        }
        WebCore::String url = imageElement->getAttribute(HTMLNames::srcAttr).string();
        info.push_back(m_mainFrame->document()->completeURL(url).string());
        types.push_back(Context::ContextTypeImageAltText);
        info.push_back(imageElement->altText());
    }

    if (obj && obj->isText()) {
        types.push_back(Context::ContextTypeText);
        info.push_back(WebCore::String(toRenderText(obj)->text()));
    }

    if (node->isElementNode()) {
        Element* element = static_cast<Element*>(node);
        if (element && ((element->hasTagName(HTMLNames::inputTag) && element->isTextFormControl())
                || element->hasTagName(HTMLNames::textareaTag)
                || element->isContentEditable())) {
            types.push_back(Context::ContextTypeInput);
            info.push_back("");
        }
    }

    if (node->isFocusable()) {
        types.push_back(Context::ContextTypeFocusable);
        info.push_back("");
    }

    return Context(types, info);
}

void WebPagePrivate::updateCursor()
{
    m_webPage->mouseEvent(Olympia::WebKit::MouseEventMoved, mapToTransformed(m_lastMouseEvent.pos()), mapToTransformed(m_lastMouseEvent.globalPos()));
}

WebCore::IntSize WebPagePrivate::fixedLayoutSize(bool snapToIncrement) const
{
    if (hasVirtualViewport())
        return IntSize(m_virtualViewportWidth, m_virtualViewportHeight);

    int defaultLayoutWidth = m_defaultLayoutSize.width();
    int defaultLayoutHeight = m_defaultLayoutSize.height();

    int minWidth = defaultLayoutWidth;
    int minHeight = defaultLayoutHeight;
    int maxWidth = DEFAULT_MAX_LAYOUT_WIDTH;
    int maxHeight = DEFAULT_MAX_LAYOUT_HEIGHT;

    if (m_viewMode == FixedDesktop) {
        int width  = maxWidth;
        // if the defaultLayoutHeight is at minimum, it probably was set as 0
        // and clamped, meaning it's effectively not set.  (Even if it happened
        // to be set exactly to the minimum, it's too small to be useful.)  So
        // ignore it.
        int height;
        if (defaultLayoutHeight <= minimumLayoutSize.height())
            height = maxHeight;
        else
            height = ceilf(static_cast<float>(width) / static_cast<float>(defaultLayoutWidth) * static_cast<float>(defaultLayoutHeight));
        return IntSize(width, height);
    }

    if (m_viewMode == Desktop) {
        // If we detect an overflow larger than the contents size then use that instead since
        // it'll still be clamped by the maxWidth below...
        int width = std::max(absoluteVisibleOverflowSize().width(), contentsSize().width());

        if (snapToIncrement) {
            // Snap to increments of defaultLayoutWidth / 2.0
            float factor = static_cast<float>(width) / (defaultLayoutWidth / 2.0);
            factor = ceilf(factor);
            width = (defaultLayoutWidth / 2.0) * factor;
        }

        if (width < minWidth)
            width = minWidth;
        if (width > maxWidth)
            width = maxWidth;
        int height = ceilf(static_cast<float>(width) / static_cast<float>(defaultLayoutWidth) * static_cast<float>(defaultLayoutHeight));
        return IntSize(width, height);
    }

    if (m_webSettings->isZoomToFitOnLoad()) {
        // We need to clamp the layout width to the minimum of the layout
        // width or the content width.  This is important under rotation for mobile
        // websites. We want the page to remain layouted at the same width which
        // it was loaded with, and instead change the zoom level to fit to screen.
        // The height is welcome to adapt to the height used in the new orientation,
        // otherwise we will get a grey bar below the web page.
        if (m_mainFrame->view() && !contentsSize().isEmpty())
            minWidth = contentsSize().width();
        else {
            // If there is no contents width, use the minimum of screen width,
            // screen height and default layout width to shape the first layout to
            // a contents width that we could reasonably zoom to fit,
            // in a manner that is rotation independent and still respects a small
            // default layout width.
            WebSettings* settings = WebSettings::pageGroupSettings(m_page->groupName());
            minWidth = std::min(settings->screenWidth(), settings->screenHeight());
        }
    }

    return IntSize(std::min(minWidth, defaultLayoutWidth), defaultLayoutHeight);
}

WebCore::String WebPagePrivate::renderTreeDump() const
{
    if (m_mainFrame->view() && m_mainFrame->view()->layoutPending())
        m_mainFrame->view()->layout();

    return externalRepresentation(m_mainFrame);
}

static inline Frame* frameForNode(Node* node)
{
    Node* origNode = node;
    for (; node; node = node->parent()) {
        if (WebCore::RenderObject* renderer = node->renderer()) {
            if (renderer->isRenderView()) {
                if (FrameView* view = toRenderView(renderer)->frameView()) {
                    if (Frame* frame = view->frame())
                        return frame;
                }
            }
            if (renderer->isWidget()) {
                WebCore::Widget* widget = toRenderWidget(renderer)->widget();
                if (widget && widget->isFrameView()) {
                    if (Frame* frame = static_cast<FrameView*>(widget)->frame())
                        return frame;
                }
            }
        }
    }

    for (node = origNode; node; node = node->parent()) {
        if (Document* doc = node->document()) {
            if (Frame* frame = doc->frame())
                return frame;
        }
    }

    return 0;
}

static WebCore::IntRect getNodeWindowRect(Node* node)
{
    if (Frame* frame = frameForNode(node)) {
        if (FrameView* view = frame->view())
            return view->contentsToWindow(node->getRect());
    }
    ASSERT_NOT_REACHED();
    return WebCore::IntRect();
}

static WebCore::IntRect getRecursiveVisibleWindowRect(WebCore::ScrollView* view)
{
    IntRect visibleWindowRect(view->contentsToWindow(view->visibleContentRect(false)));
    if (view->parent()) {
        // Intersect with parent visible rect.
        visibleWindowRect.intersect(getRecursiveVisibleWindowRect(view->parent()));
    }
    return visibleWindowRect;
}

class ComplicatedDistance {
public:

    enum DistanceType {
        DistanceNone,
        DistanceApart,
        DistanceIntersected,
        DistanceOverlapped,
    };

    DistanceType m_type;
    int m_dist;

    ComplicatedDistance() : m_type(DistanceNone), m_dist(0) {}

    void calcDistance(int baseStart, int baseEnd, int targetStart, int targetEnd)
    {
        m_dist = targetEnd <= baseStart ? targetEnd - baseStart - 1
            : targetStart >= baseEnd ? targetStart - baseEnd + 1
            : 0;
        if (m_dist) {
            m_type = DistanceApart;
            return;
        }

        m_dist = targetStart == baseStart
                ? 0
                : targetStart < baseStart
                ? targetEnd >= baseEnd ? 0 : baseStart - targetEnd
                : targetEnd <= baseEnd ? 0 : baseEnd - targetStart;
        if (m_dist) {
            m_type = DistanceIntersected;
            return;
        }

        m_dist = targetStart - baseStart + targetEnd - baseEnd;
        m_type = DistanceOverlapped;
    }

    bool isPositive() const { return m_dist >= 0; }
    bool isNegative() const { return m_dist <= 0; }

    bool operator<(const ComplicatedDistance& o) const
    {
        return m_type != o.m_type
            ? m_type > o.m_type
            : m_type == DistanceIntersected
            ? abs(m_dist) > abs(o.m_dist)
            : abs(m_dist) < abs(o.m_dist);
    }
    bool operator==(const ComplicatedDistance& o) const
    {
        return m_type == o.m_type && m_dist == o.m_dist;
    }
};

static inline bool checkDirection(Olympia::Platform::ScrollDirection dir, const ComplicatedDistance& delta, Olympia::Platform::ScrollDirection negDir, Olympia::Platform::ScrollDirection posDir)
{
    return dir == negDir ? delta.isNegative() : dir == posDir ? delta.isPositive() : false;
}

static inline bool areNodesPerpendicularlyNavigatable(Olympia::Platform::ScrollDirection dir, const WebCore::IntRect& baseRect, const WebCore::IntRect& targetRect)
{
    ComplicatedDistance distX, distY;
    distX.calcDistance(baseRect.x(), baseRect.right(), targetRect.x(), targetRect.right());
    distY.calcDistance(baseRect.y(), baseRect.bottom(), targetRect.y(), targetRect.bottom());

    return distY < distX ? dir == ScrollUp || dir == ScrollDown
        : dir == ScrollLeft || dir == ScrollRight;
}

struct NodeDistance {
    ComplicatedDistance m_dist;
    ComplicatedDistance m_perpendicularDist;
    WebCore::IntRect m_rect;

    bool isCloser(const NodeDistance& o, Olympia::Platform::ScrollDirection dir) const
    {
        return m_dist.m_type == o.m_dist.m_type
            ? (m_dist == o.m_dist ? m_perpendicularDist < o.m_perpendicularDist
                : m_dist < o.m_dist ? m_perpendicularDist.m_type >= o.m_perpendicularDist.m_type
                || !areNodesPerpendicularlyNavigatable(dir, o.m_rect, m_rect)
                : m_perpendicularDist.m_type > o.m_perpendicularDist.m_type
                && areNodesPerpendicularlyNavigatable(dir, m_rect, o.m_rect))
            : m_dist.m_type > o.m_dist.m_type;
    }
};

static inline bool tellDistance(Olympia::Platform::ScrollDirection dir
                                , const WebCore::IntRect& baseRect
                                , NodeDistance& nodeDist)
{
    ComplicatedDistance distX, distY;
    distX.calcDistance(baseRect.x(), baseRect.right(), nodeDist.m_rect.x(), nodeDist.m_rect.right());
    distY.calcDistance(baseRect.y(), baseRect.bottom(), nodeDist.m_rect.y(), nodeDist.m_rect.bottom());

    bool horizontal = distY < distX;
    if (horizontal ? checkDirection(dir, distX, Olympia::Platform::ScrollLeft, Olympia::Platform::ScrollRight) : checkDirection(dir, distY, Olympia::Platform::ScrollUp, Olympia::Platform::ScrollDown)) {
        nodeDist.m_dist = horizontal ? distX : distY;
        nodeDist.m_perpendicularDist = horizontal ? distY : distX;
        return true;
    }
    return false;
}

bool WebPagePrivate::focusField(bool focus)
{
    Frame* frame = focusedOrMainFrame();
    if (!frame)
        return false;

    // If focus is false, try to unfocus the current focused element.
    if (!focus) {
        Document* doc = frame->document();
        if (doc && doc->focusedNode() && doc->focusedNode()->isElementNode())
            static_cast<Element*>(doc->focusedNode())->blur();
    }

    m_page->focusController()->setFocused(focus);

    return true;
}

bool WebPagePrivate::moveToNextField(Olympia::Platform::ScrollDirection dir, int desiredScrollAmount)
{
    Frame* frame = focusedOrMainFrame();
    if (!frame)
        return false;

    return moveToNextField(frame, dir, desiredScrollAmount);
}

bool WebPagePrivate::moveToNextField(Frame* frame, Olympia::Platform::ScrollDirection dir, int desiredScrollAmount)
{
    Document* doc = frame->document();
    FrameView* view = frame->view();
    if (!doc || !view || view->needsLayout())
        return false;

    // The layout needs to be up-to-date to determine if a node is focusable.
    doc->updateLayoutIgnorePendingStylesheets();

    Node* origFocusedNode = doc->focusedNode();

    Node* startNode = doc->nextFocusableNode(origFocusedNode, 0);

    // Last node? get back to beginning
    if (!startNode && origFocusedNode)
        startNode = doc->nextFocusableNode(0, 0);

    if (!startNode || startNode == origFocusedNode)
        return false;

    Node* origRadioGroup = origFocusedNode
        && ((origFocusedNode->hasTagName(HTMLNames::inputTag)
        && static_cast<HTMLInputElement*>(origFocusedNode)->inputType() == HTMLInputElement::RADIO))
        ? origFocusedNode->parentNode() : 0;

    WebCore::IntRect visibleRect = IntRect(IntPoint(), actualVisibleSize());
    // Constrain the rect if this is a subframe.
    if (view->parent())
        visibleRect.intersect(getRecursiveVisibleWindowRect(view));

    WebCore::IntRect baseRect;
    if (origRadioGroup)
        baseRect = getNodeWindowRect(origRadioGroup);
    else if (origFocusedNode)
        baseRect = getNodeWindowRect(origFocusedNode);
    else {
        baseRect = visibleRect;
        switch (dir) {
        case Olympia::Platform::ScrollLeft:
            baseRect.setX(baseRect.right());
        case Olympia::Platform::ScrollRight:
            baseRect.setWidth(0);
            break;
        case Olympia::Platform::ScrollUp:
            baseRect.setY(baseRect.bottom());
        case Olympia::Platform::ScrollDown:
            baseRect.setHeight(0);
            break;
        }
    }

    Node* bestChoice = 0;
    NodeDistance bestChoiceDistance;
    Node* lastCandidate = 0;

    WebCore::IntRect bestChoiceRect = baseRect;

    Node* cur = startNode;

    do {
        if (cur->isFocusable()) {
            Node* curRadioGroup = (cur->hasTagName(HTMLNames::inputTag)
                && static_cast<HTMLInputElement*>(cur)->inputType() == HTMLInputElement::RADIO)
                ? cur->parentNode() : 0;
            if (!curRadioGroup || static_cast<HTMLInputElement*>(cur)->checked()) {
                WebCore::IntRect nodeRect = curRadioGroup ? getNodeWindowRect(curRadioGroup) : getNodeWindowRect(cur);
                // Warning, if for some reason the nodeRect is 0x0, intersects will not match.  This shouldn't be a
                // problem as we don't want to focus on a non-visible node.
                if (visibleRect.intersects(nodeRect)) {
                    NodeDistance nodeDistance;
                    nodeDistance.m_rect = nodeRect;
                    if (tellDistance(dir, baseRect, nodeDistance)) {
                        if (!bestChoice
                            || nodeDistance.isCloser(bestChoiceDistance, dir)) {
                            bestChoice = cur;
                            bestChoiceDistance = nodeDistance;
                            bestChoiceRect = nodeRect;
                        }
                    } else
                        lastCandidate = cur;
                }
            }
        }

        cur = doc->nextFocusableNode(cur, 0);

        // Last node? go back
        if (!cur && origFocusedNode)
            cur = doc->nextFocusableNode(0, 0);
    } while (cur && cur != origFocusedNode && cur != startNode);

    if (!bestChoice && !origFocusedNode) {
        // In the case that previously no node is on focus.
        bestChoice = lastCandidate;
    }

    if (bestChoice) {
        // FIXME: Temporarily disable automatical scrolling for HTML email
#if 0
        if (Document* nodeDoc = bestChoice->document()) {
            if (FrameView* nodeView = nodeDoc->view())
                nodeView->scrollRectIntoViewRecursively(bestChoice->getRect());
        }
#endif

        if (bestChoice->isElementNode())
            static_cast<Element*>(bestChoice)->focus(true);
        else
            m_page->focusController()->setFocusedNode(bestChoice, bestChoice->document()->frame());

        // Trigger a repaint of the node rect as it frequently isn't updated due to JavaScript being
        // disabled.  See RIM Bug #1242.
        m_backingStore->d->repaint(bestChoiceRect, true /*contentChanged*/, false /*immediate*/, false /*repaintContentOnly*/);

        return true;
    }

    if (Frame* parent = frame->tree()->parent()) {
        if (moveToNextField(parent, dir, desiredScrollAmount))
            return true;
    }

    // No new focus field found, a page scroll is about to occur.
    // Calculate the new visible rect after this move and if
    // the visible rect doesn't contain the focus node, tell Java
    // through the context mechanism only that the node is unfocused
    // to prevent GCM actions on it.
    // See RIM Bug #1396.
    switch (dir) {
    case Olympia::Platform::ScrollLeft:
        visibleRect.move(-min(scrollPosition().x(), desiredScrollAmount), 0);
        break;
    case Olympia::Platform::ScrollRight:
        visibleRect.move(min(contentsSize().width() - visibleRect.right(), desiredScrollAmount), 0);
        break;
    case Olympia::Platform::ScrollUp:
        visibleRect.move(0, -min(scrollPosition().y(), desiredScrollAmount));
        break;
    case Olympia::Platform::ScrollDown:
        visibleRect.move(0, min(contentsSize().height() - visibleRect.bottom(), desiredScrollAmount));
        break;
    }

    if (!visibleRect.intersects(baseRect))
        sendContextIfChanged(0); // Send an empty context.
    else
        sendContextIfChanged(origFocusedNode);  // Send the valid context if required.

    return false;
}

Olympia::Platform::IntRect WebPagePrivate::focusNodeRect()
{
    Frame* frame = focusedOrMainFrame();
    if (!frame)
        return Olympia::Platform::IntRect();

    Document* doc = frame->document();
    FrameView* view = frame->view();
    if (!doc || !view || view->needsLayout())
        return Olympia::Platform::IntRect();

    WebCore::IntRect focusRect = mapToTransformed(rectForNode(doc->focusedNode()));
    clipToTransformedContentsRect(focusRect);
    return focusRect;
}

static inline int distanceBetweenPoints(WebCore::IntPoint p1, WebCore::IntPoint p2)
{
    int dx = p1.x() - p2.x();
    int dy = p1.y() - p2.y();
    return sqrt((double)((dx * dx) + (dy * dy)));
}

Node* WebPagePrivate::bestNodeForZoomUnderPoint(const WebCore::IntPoint& point)
{
    IntPoint pt = mapFromTransformed(point);
    IntRect clickRect(pt.x() - blockClickRadius, pt.y() - blockClickRadius, 2 * blockClickRadius, 2 * blockClickRadius);
    Node* originalNode = nodeForZoomUnderPoint(point);
    if (!originalNode)
        return 0;
    Node* node = bestChildNodeForClickRect(originalNode, clickRect);
    return node ? adjustedBlockZoomNodeForZoomLimits(node) : adjustedBlockZoomNodeForZoomLimits(originalNode);
}

Node* WebPagePrivate::bestChildNodeForClickRect(Node* parentNode, const WebCore::IntRect& clickRect)
{
    if (!parentNode)
        return 0;

    int bestDistance = std::numeric_limits<int>::max();

    Node* node = parentNode->firstChild();
    Node* bestNode = 0;
    while (node) {
        IntRect rect = rectForNode(node);
        if (clickRect.intersects(rect)) {
            int distance = distanceBetweenPoints(rect.center(), clickRect.center());
            Node* bestChildNode = bestChildNodeForClickRect(node, clickRect);
            if (bestChildNode) {
                IntRect bestChildRect = rectForNode(bestChildNode);
                int bestChildDistance = distanceBetweenPoints(bestChildRect.center(), clickRect.center());
                if (bestChildDistance < distance && bestChildDistance < bestDistance) {
                    bestNode = bestChildNode;
                    bestDistance = bestChildDistance;
                } else {
                    if (distance < bestDistance) {
                        bestNode = node;
                        bestDistance = distance;
                    }
                }
            } else {
                if (distance < bestDistance) {
                    bestNode = node;
                    bestDistance = distance;
                }
            }
        }
        node = node->nextSibling();
    }

    return bestNode;
}

double WebPagePrivate::maxBlockZoomScale() const
{
    return std::min(maximumBlockZoomScale, maximumScale());
}

Node* WebPagePrivate::nodeForZoomUnderPoint(const WebCore::IntPoint& point)
{
    if (!m_mainFrame)
        return 0;

    HitTestResult result = m_mainFrame->eventHandler()->hitTestResultAtPoint(mapFromTransformed(point), false);

    Node* node = result.innerNonSharedNode();

    if (!node)
        return 0;

    RenderObject* renderer = node->renderer();
    while (!renderer) {
        node = node->parent();
        renderer = node->renderer();
    }

    return node;
}

Node* WebPagePrivate::adjustedBlockZoomNodeForZoomLimits(Node* node)
{
    Node* initialNode = node;
    RenderObject* renderer = node->renderer();
    bool acceptableNodeSize = newScaleForBlockZoomRect(rectForNode(node), 1.0, 0) < maxBlockZoomScale();

    while (!renderer || !acceptableNodeSize) {
        node = node->parent();

        if (!node)
            return initialNode;

        renderer = node->renderer();
        acceptableNodeSize = newScaleForBlockZoomRect(rectForNode(node), 1.0, 0) < maxBlockZoomScale();
    }

    return node;
}

bool WebPagePrivate::compareNodesForBlockZoom(Node* n1, Node* n2)
{
    if (!n1 || !n2)
        return false;

    return (n2 == n1) || n2->isDescendantOf(n1);
}

double WebPagePrivate::newScaleForBlockZoomRect(const WebCore::IntRect& rect, double oldScale, double margin)
{
    if (rect.isEmpty())
        return std::numeric_limits<double>::max();

    ASSERT(rect.width() + margin);

    double newScale = oldScale * static_cast<double>(transformedActualVisibleSize().width()) / (rect.width() + margin);

    return newScale;
}

WebCore::IntRect WebPagePrivate::rectForNode(Node* node)
{
    if (!node)
        return IntRect();

    RenderObject* renderer = node->renderer();

    if (!renderer)
        return IntRect();

    // Return rect in un-transformed content coordinates
    IntRect blockRect = renderer->absoluteClippedOverflowRect();

    if (renderer->isText()) {
        RenderBlock* rb = renderer->containingBlock();

        // Inefficient? way to find width when floats intersect a block
        int blockWidth = 0;
        for (int i = 0; i < rb->lineCount(); i++)
            blockWidth = max(blockWidth, rb->lineWidth(i, false));

        blockRect.setWidth(blockWidth);
        blockRect.setX(blockRect.x() + rb->leftOffset(1, false));
    }

    // Strip off padding
    if (renderer->style()->hasPadding()) {
        blockRect.setX(blockRect.x() + renderer->style()->paddingLeft().value());
        blockRect.setY(blockRect.y() + renderer->style()->paddingTop().value());
        blockRect.setWidth(blockRect.width() - renderer->style()->paddingRight().value());
        blockRect.setHeight(blockRect.height() - renderer->style()->paddingBottom().value());
    }

    return blockRect;
}

WebCore::IntRect WebPagePrivate::adjustRectOffsetForFrameOffset(const WebCore::IntRect& rect, const Node* node)
{
    if (!node)
        return rect;

    // Adjust the offset of the rect if it is in an iFrame/frame or set of iFrames/frames
    const Node* tnode = node;
    IntRect adjustedRect = rect;
    do {
        Frame* frame = tnode->document()->frame();
        if (frame) {
            Node* ownerNode = static_cast<Node*>(frame->ownerElement());
            tnode = ownerNode;
            if (ownerNode && (ownerNode->hasTagName(HTMLNames::iframeTag) || ownerNode->hasTagName(HTMLNames::frameTag))) {
                IntRect iFrameRect;
                do {
                    iFrameRect = rectForNode(ownerNode);
                    adjustedRect.move(iFrameRect.x(), iFrameRect.y());
                    adjustedRect.intersect(iFrameRect);
                    ownerNode = ownerNode->parent();
                } while (iFrameRect.isEmpty() && ownerNode);
            } else
                break;
        }
    } while (tnode = tnode->parent());

    return adjustedRect;
}

WebCore::IntRect WebPagePrivate::blockZoomRectForNode(Node* node)
{
    if (!node || contentsSize().isEmpty())
        return IntRect();

    Node* tnode = node;
    m_currentBlockZoomAdjustedNode = tnode;

    IntRect blockRect = rectForNode(tnode);

    IntRect originalRect = blockRect;

    int originalArea = originalRect.width() * originalRect.height();
    int pageArea = contentsSize().width() * contentsSize().height();
    double blockToPageRatio = (double)(pageArea - originalArea) / pageArea;
    double blockExpansionRatio = 5.0 * blockToPageRatio * blockToPageRatio;

    if (!tnode->hasTagName(HTMLNames::imgTag) && !tnode->hasTagName(HTMLNames::inputTag) && !tnode->hasTagName(HTMLNames::textareaTag))
        while (tnode = tnode->parent()) {
            ASSERT(tnode);
            IntRect tRect = rectForNode(tnode);
            int tempBlockArea = tRect.width() * tRect.height();
            // Don't expand the block if it will be too large relative to the content
            if ((double)(pageArea - tempBlockArea) / pageArea < 0.15)
                break;
            if (tRect.isEmpty())
                continue; //no renderer
            if (tempBlockArea < blockExpansionRatio * originalArea) {
                blockRect = tRect;
                m_currentBlockZoomAdjustedNode = tnode;
            } else
                break;
        }

    blockRect = adjustRectOffsetForFrameOffset(blockRect, node);
    blockRect = mapToTransformed(blockRect);
    clipToTransformedContentsRect(blockRect);

#if DEBUG_BLOCK_ZOOM
    // re-paint the backingstore to screen to erase other annotations
    m_backingStore->d->resumeScreenUpdates(BackingStorePrivate::Blit);

    // Render a black square over the calculated block and a gray square over the original block for visual inspection
    originalRect = mapToTransformed(originalRect);
    clipToTransformedContentsRect(originalRect);
    IntRect renderRect = mapFromTransformedContentsToTransformedViewport(blockRect);
    IntRect originalRenderRect = mapFromTransformedContentsToTransformedViewport(originalRect);
    IntSize viewportSize = transformedViewportSize();
    renderRect.intersect(IntRect(0, 0, viewportSize.width(), viewportSize.height()));
    originalRenderRect.intersect(IntRect(0, 0, viewportSize.width(), viewportSize.height()));
    m_client->clearCanvas(renderRect, 0x0000);
    m_client->clearCanvas(originalRenderRect, 0x7BEF);
    invalidateWindow(renderRect);
#endif

    return blockRect;
}

void WebPagePrivate::animateBlockZoom()
{
    m_backingStore->d->suspendScreenUpdates();
    m_blockZoomAnimation->initializeBuffer(transformedVisibleContentsRect().size());
    m_blockZoomAnimation->setDuration(400);
    m_blockZoomAnimation->setUpdateInterval(33);
    m_blockZoomAnimation->setShapingFunction(OlympiaAnimationBase::EaseOutCurve);
    m_blockZoomAnimation->stop();
    m_blockZoomAnimation->start();
}

void WebPagePrivate::animationBlockZoomFrameChanged(Olympia::WebKit::OlympiaAnimation<WebPagePrivate>* animation, double currentValue, double previousValue)
{
    if (currentValue == -1.0) {
        if (previousValue != m_blockZoomAnimation->duration()) {
            double zoomFraction = m_blockZoomFinalScale / m_blockZoomInitialScale;
            IntPoint pt(roundTransformedPoint(m_finalBlockPoint));
            IntRect srcRect(pt.x(), pt.y(), ceil(static_cast<double>(transformedVisibleContentsRect().width()) / zoomFraction), ceil(static_cast<double>(transformedVisibleContentsRect().height()) / zoomFraction));
            m_backingStore->d->blitScaledArbitraryRectFromBackingStoreToScreen(srcRect, IntRect(IntPoint(0, 0), transformedVisibleContentsRect().size()), animation->buffer(), animation->bufferStride());
#if DEBUG_BLOCK_ZOOM
            m_currentBlockZoomNode = 0L;
            m_currentBlockZoomAdjustedNode = 0L;
            m_client->zoomChanged(m_webPage->isMinZoomed(), m_webPage->isMaxZoomed(), !shouldZoomOnEscape(), currentScale());
#else
            zoomBlock();
#endif
        }
        animation->destroyBuffer();
        return;
    }

    double zoomFraction = (m_blockZoomInitialScale + currentValue * (m_blockZoomFinalScale - m_blockZoomInitialScale)) / m_blockZoomInitialScale;
    FloatPoint scrollPosition = mapToTransformedFloatPoint(this->scrollPosition());
    FloatPoint anchor(scrollPosition.x() + currentValue * (m_finalBlockPoint.x() - scrollPosition.x()),
            scrollPosition.y() + currentValue * (m_finalBlockPoint.y() - scrollPosition.y()));
    FloatPoint anchorOffset(m_finalBlockPoint.x() - anchor.x(), m_finalBlockPoint.y() - anchor.y());
    FloatPoint srcPoint(m_finalBlockPoint.x() - anchorOffset.x() / zoomFraction,
            m_finalBlockPoint.y() - anchorOffset.y() / zoomFraction);
    IntRect srcRect(roundTransformedPoint(srcPoint), IntSize(ceil(static_cast<double>(transformedVisibleContentsRect().width()) / zoomFraction), ceil(static_cast<double>(transformedVisibleContentsRect().height()) / zoomFraction)));
    m_backingStore->d->blitScaledArbitraryRectFromBackingStoreToScreen(srcRect, IntRect(IntPoint(0, 0), transformedVisibleContentsRect().size()), animation->buffer(), animation->bufferStride());
    m_blockZoomAnimation->setCanRenderNextFrame(true);
}

void WebPagePrivate::animateZoomBounceBack(const FloatPoint& lastBouncePoint)
{
    double zoomFraction = m_bitmapScale / m_transformationMatrix->m11();
    FloatPoint scrollPosition = mapToTransformedFloatPoint(this->scrollPosition());
    FloatPoint anchorOffset(lastBouncePoint.x() - scrollPosition.x(), lastBouncePoint.y() - scrollPosition.y());
    m_lastBounceBackFocalPoint = FloatPoint(lastBouncePoint.x() - anchorOffset.x() / zoomFraction, lastBouncePoint.y() - anchorOffset.y() / zoomFraction);

    zoomFraction = minimumScale() / m_transformationMatrix->m11();
    FloatPoint fp = mapToTransformedFloatPoint(centerOfVisibleContentsRect());
    anchorOffset = FloatPoint(fp.x() - scrollPosition.x(), fp.y() - scrollPosition.y());
    m_bounceBackFinalPoint = FloatPoint(fp.x() - anchorOffset.x() / zoomFraction, fp.y() - anchorOffset.y() / zoomFraction);

    m_bounceBackFinalPoint.setX(std::max(0.0f, m_bounceBackFinalPoint.x()));
    if (m_bounceBackFinalPoint.x() + m_actualVisibleWidth * m_transformationMatrix->m11() / minimumScale() > transformedContentsSize().width())
        m_bounceBackFinalPoint.setX(std::max(0.0, transformedContentsSize().width() - m_actualVisibleWidth * m_transformationMatrix->m11() / minimumScale()));

    m_bounceBackFinalPoint.setY(std::max(0.0f, m_bounceBackFinalPoint.y()));
    if (m_bounceBackFinalPoint.y() + m_actualVisibleHeight * m_transformationMatrix->m11() / minimumScale() > transformedContentsSize().height())
        m_bounceBackFinalPoint.setY(std::max(0.0, transformedContentsSize().height() - m_actualVisibleHeight * m_transformationMatrix->m11() / minimumScale()));

    m_zoomBounceBackAnimation->setDuration(400);
    m_zoomBounceBackAnimation->setUpdateInterval(33);
    m_zoomBounceBackAnimation->setShapingFunction(OlympiaAnimationBase::Linear);
    m_zoomBounceBackAnimation->stop();
    m_zoomBounceBackAnimation->start();
}

void WebPagePrivate::animationZoomBounceBackFrameChanged(Olympia::WebKit::OlympiaAnimation<WebPagePrivate>*, double currentValue, double previousValue)
{
    if (contentsSize().isEmpty())
        return;

    if (currentValue == -1.0) {
        if (previousValue != m_zoomBounceBackAnimation->duration()) {
            double zoomFraction = minimumScale() / m_transformationMatrix->m11();
            IntRect srcRect(static_cast<int>(roundf(m_bounceBackFinalPoint.x())), static_cast<int>(roundf(m_bounceBackFinalPoint.y())), transformedVisibleContentsRect().width() / zoomFraction, transformedVisibleContentsRect().height() / zoomFraction);
            m_backingStore->d->blitScaledArbitraryRectFromBackingStoreToScreen(srcRect, IntRect(IntPoint(0, 0), transformedVisibleContentsRect().size()), m_bitmapZoomBuffer, m_bitmapZoomBufferStride);
            m_bitmapScale = minimumScale();
            m_webPage->zoomToFit();
        }
        delete [] m_bitmapZoomBuffer;
        m_bitmapZoomBuffer = 0;
        m_bitmapZoomBufferStride = 0;
        return;
    }

    double zoomFraction = (m_bitmapScale + currentValue * (minimumScale() - m_bitmapScale)) / m_transformationMatrix->m11();
    double dx = m_lastBounceBackFocalPoint.x() + currentValue * (m_bounceBackFinalPoint.x() - m_lastBounceBackFocalPoint.x());
    double dy = m_lastBounceBackFocalPoint.y() + currentValue * (m_bounceBackFinalPoint.y() - m_lastBounceBackFocalPoint.y());
    IntPoint srcPoint(static_cast<int>(roundf(dx)), static_cast<int>(roundf(dy)));
    IntRect srcRect(srcPoint.x(), srcPoint.y(), transformedVisibleContentsRect().width() / zoomFraction, transformedVisibleContentsRect().height() / zoomFraction);
    m_backingStore->d->blitScaledArbitraryRectFromBackingStoreToScreen(srcRect, IntRect(IntPoint(0, 0), transformedVisibleContentsRect().size()), m_bitmapZoomBuffer, m_bitmapZoomBufferStride);

    m_zoomBounceBackAnimation->setCanRenderNextFrame(true);
}

void WebPagePrivate::renderBitmapZoomFrame(double scale, const FloatPoint& anchor /* backingstore coordinates */, unsigned short* tempSurface /* = 0 */, unsigned int tempSurfaceStride)
{
    // Need to invert the previous transform to anchor the viewport.
    double zoomFraction = scale / m_transformationMatrix->m11();

    // Our current scroll position in float.
    FloatPoint scrollPosition = mapToTransformedFloatPoint(this->scrollPosition());

    // Anchor offset from scroll position in float.
    FloatPoint anchorOffset(anchor.x() - scrollPosition.x(), anchor.y() - scrollPosition.y());

    IntPoint srcPoint(static_cast<int>(roundf(anchor.x() - anchorOffset.x() / zoomFraction)),
                static_cast<int>(roundf(anchor.y() - anchorOffset.y() / zoomFraction)));

    // This is the rect to pass as the actual source rect in the backingstore for the transform given by zoom.
    IntRect srcRect(srcPoint.x(), srcPoint.y(), transformedVisibleContentsRect().width() / zoomFraction, transformedVisibleContentsRect().height() / zoomFraction);

    if (!tempSurface) {
        tempSurface = m_bitmapZoomBuffer;
        tempSurfaceStride = m_bitmapZoomBufferStride;
    }
    m_backingStore->d->blitScaledArbitraryRectFromBackingStoreToScreen(srcRect, IntRect(IntPoint(0, 0), transformedVisibleContentsRect().size()), tempSurface, tempSurfaceStride);
}

// This function should not be called directly. It is called after the animation ends automatically.
void WebPagePrivate::zoomBlock()
{
    if (!m_mainFrame)
        return;

    IntPoint anchor(roundUntransformedPoint(mapFromTransformedFloatPoint(m_finalBlockPoint)));
    bool willUseTextReflow = false;

#if ENABLE(VIEWPORT_REFLOW)
    willUseTextReflow = m_webPage->settings()->textReflowMode() != WebSettings::TextReflowDisabled;
    toggleTextReflowIfTextReflowEnabledOnlyForBlockZoom(m_shouldReflowBlock);
    setNeedsLayout();
#endif

    TransformationMatrix zoom;
    zoom.scale(m_blockZoomFinalScale);
    *m_transformationMatrix = zoom;
    m_bitmapScale = m_blockZoomFinalScale;
    m_backingStore->d->suspendScreenUpdates();
    updateViewportSize();

#if ENABLE(VIEWPORT_REFLOW)
    requestLayoutIfNeeded();
    if (willUseTextReflow && m_shouldReflowBlock) {
        IntRect reflowedRect = rectForNode(m_currentBlockZoomAdjustedNode.get());
        reflowedRect = adjustRectOffsetForFrameOffset(reflowedRect, m_currentBlockZoomAdjustedNode.get());
        reflowedRect.move(roundTransformedPoint(m_finalBlockPointReflowOffset).x(), roundTransformedPoint(m_finalBlockPointReflowOffset).y());
        setScrollPosition(reflowedRect.topLeft());
    } else if (willUseTextReflow) {
        IntRect finalRect = rectForNode(m_currentBlockZoomAdjustedNode.get());
        finalRect = adjustRectOffsetForFrameOffset(finalRect, m_currentBlockZoomAdjustedNode.get());
        setScrollPosition(IntPoint(0, finalRect.y() + m_finalBlockPointReflowOffset.y()));
        resetBlockZoom();
    }
#endif
    if (!willUseTextReflow) {
        setScrollPosition(anchor);
        if (!m_shouldReflowBlock)
            resetBlockZoom();
    }

    notifyTransformChanged();
    m_client->lockCanvas();
    m_client->clearCanvas();
    m_backingStore->d->resumeScreenUpdates(BackingStorePrivate::RenderAndBlit);
    m_client->unlockCanvas();
    m_client->zoomChanged(m_webPage->isMinZoomed(), m_webPage->isMaxZoomed(), !shouldZoomOnEscape(), currentScale());
}


void WebPagePrivate::resetBlockZoom()
{
    m_currentBlockZoomNode = 0;
    m_currentBlockZoomAdjustedNode = 0;
}

WebPage::WebPage(WebPageClient* client, const String& pageGroupName, const Platform::IntRect& rect)
{
    WebSettings* settings = WebSettings::pageGroupSettings(pageGroupName);
    settings->setScreenWidth(rect.width());
    settings->setScreenHeight(rect.height());

    d = new WebPagePrivate(this, client, rect);
    d->init(pageGroupName);
}

WebPage::~WebPage()
{
    delete d;
    d = 0;
}

WebPageClient* WebPage::client() const
{
    return d->m_client;
}

void WebPage::load(const char* url, const char* networkToken, bool isInitial)
{
    d->load(url, networkToken, "GET", Platform::NetworkRequest::UseProtocolCachePolicy, 0, 0, 0, 0, isInitial);
}

void WebPage::loadExtended(const char* url, const char* networkToken, const char* method, Platform::NetworkRequest::CachePolicy cachePolicy, const char* data, size_t dataLength, const char* const* headers, size_t headersLength)
{
    d->load(url, networkToken, method, cachePolicy, data, dataLength, headers, headersLength, false);
}

void WebPage::stopLoading()
{
    d->stopCurrentLoad();
}

bool WebPage::goBackOrForward(int delta)
{
    if (d->m_page->canGoBackOrForward(delta)) {
        d->m_page->goBackOrForward(delta);
        return true;
    }
    return false;
}

void WebPage::goToBackForwardEntry(BackForwardId id)
{
    HistoryItem* item = historyItemFromBackForwardId(id);
    ASSERT(item);
    d->m_page->goToItem(item, FrameLoadTypeIndexedBackForward);
}

void WebPage::reload()
{
    d->m_mainFrame->loader()->reload(/* bypassCache */ true);
}

void WebPage::reloadFromCache()
{
    d->m_mainFrame->loader()->reload(/* bypassCache */ false);
}

WebSettings* WebPage::settings() const
{
    return d->m_webSettings;
}

int WebPage::width() const
{
    return d->m_width;
}

int WebPage::height() const
{
    return d->m_height;
}

bool WebPage::isVisible() const
{
    return d->m_visible;
}

void WebPage::setVisible(bool visible)
{
    if (d->m_visible == visible)
        return;

    d->m_visible = visible;

    if (!visible)
        return;

    if (WebPagePrivate::g_lastVisibleWebPage != this || d->m_shouldResetTilesWhenShown) {
        // We need to reset all tiles so that we do not show any tiles whose content may
        // have been replaced by another WebPage instance (i.e. another tab).
        WebPagePrivate::g_lastVisibleWebPage = this;
        d->m_backingStore->d->orientationChanged(); // Updates tile geometry and creates visible tile buffer
        d->m_backingStore->d->createScrollbars();
        d->m_backingStore->d->resetTiles(true /*resetBackground*/);
        d->m_backingStore->d->updateTiles(false /*updateVisible*/, false /*immediate*/);
        d->m_backingStore->d->renderVisibleContents(false /*renderContentOnly*/);
    } else {
        // Rendering was disabled while we were hidden, so we need to update all tiles.
        d->m_backingStore->d->updateTiles(true /*updateVisible*/, false /*immediate*/);
    }

    d->m_shouldResetTilesWhenShown = false;
}

void WebPage::requestElementText(int requestedFrameId, int requestedElementId, int offset, int length)
{
    d->m_inputHandler->requestElementText(requestedFrameId, requestedElementId, offset, length);
}

void WebPage::selectionChanged()
{
    d->m_inputHandler->selectionChanged();
    d->m_selectionHandler->selectionPositionChanged();
}

void WebPage::setCaretPosition(int requestedFrameId, int requestedElementId, int caretPosition)
{
    d->m_inputHandler->setCaretPosition(requestedFrameId, requestedElementId, caretPosition);
}

void WebPage::setCaretHighlightStyle(Olympia::Platform::CaretHighlightStyle style)
{
    // Note, this method assumes we are using the Olympia theme.
    RenderThemeOlympia* theme = static_cast<RenderThemeOlympia*>(d->m_page->theme());
    theme->setCaretHighlightStyle(style);
    if (RenderObject* caretRenderer = d->m_mainFrame->selection()->caretRenderer())
        caretRenderer->repaint();
}

Olympia::Platform::ReplaceTextErrorCode WebPage::replaceText(const Olympia::Platform::ReplaceArguments& replaceArgs, const Olympia::Platform::AttributedText& attribText)
{
    return d->m_inputHandler->replaceText(replaceArgs, attribText);
}

Olympia::Platform::IntRect WebPage::rectForCaret(int index)
{
    // All rects being passed across bridge must be transformed from WebKit coords.
    WebCore::IntRect rect = d->mapToTransformed(d->m_inputHandler->rectForCaret(index));
    d->clipToTransformedContentsRect(rect);
    return Olympia::Platform::IntRect(rect);
}

void WebPage::popupListClosed(const int size, bool* selecteds)
{
    d->setPopupListIndexes(size, selecteds);
}

void WebPage::popupListClosed(const int index)
{
    d->setPopupListIndex(index);
}

void WebPage::setDateTimeInput(const Olympia::WebKit::String& value)
{
    d->setDateTimeInput(String(value.impl()));
}

void WebPage::setColorInput(const Olympia::WebKit::String& value)
{
    d->setColorInput(String(value.impl()));
}

Context WebPage::getContext() const
{
    return d->getContext();
}

void WebPage::setActualVisibleSize(int width, int height)
{
    if (width == d->m_actualVisibleWidth && height == d->m_actualVisibleHeight)
        return;

    d->m_actualVisibleWidth = width;
    d->m_actualVisibleHeight = height;
    d->updateViewportSize();
    d->m_backingStore->d->actualVisibleSizeChanged(d->transformedActualVisibleSize());
    d->m_inputHandler->ensureFocusElementVisible();
}

void WebPage::setVirtualViewportSize(int width, int height)
{
    d->m_virtualViewportWidth = width;
    d->m_virtualViewportHeight = height;
}

void WebPage::resetVirtualViewportOnCommitted(bool reset)
{
    d->m_resetVirtualViewportOnCommitted = reset;
}

WebCore::IntSize WebPagePrivate::recomputeVirtualViewportFromViewportArguments()
{
    if (!m_viewportArguments.hasCustomArgument())
        return IntSize();

    // Note, if either the width or height (or both) are not specified or negative then we infer the width
    // and height with respect to the orientation of the device at the time the page was loaded as per
    // <http://developer.apple.com/safari/library/documentation/AppleApplications/Reference/SafariHTMLRef/Articles/MetaTags.html#//apple_ref/doc/uid/TP40008193-SW6>.
    int virtualViewportWidth = m_viewportArguments.width;
    int virtualViewportHeight = m_viewportArguments.height;

    // Check against the explicitly set scales
    bool initialScaleIsExplicit = m_initialScale != -1;
    bool minimumScaleIsExplicit = m_minimumScale != -1;
    bool maximumScaleIsExplicit = m_maximumScale != -1;

    // We explicitly use m_defaultLayoutSize.width() and m_defaultLayoutSize.height() instead of
    // m_actualVisibleWidth and m_actualVisibleHeight so that we repaint the entire viewport and hence
    // prevent any rendering artifacts (see comment #1 of RIM Bug #450).
    if (!m_viewportArguments.width) {
        // m_viewportArguments.width == 0 does not indicate an valid viewport width.
        virtualViewportWidth = m_page->chrome()->client()->platformScreenSize().width();
    } else if (m_viewportArguments.width < 0)
        virtualViewportWidth = m_defaultLayoutSize.width();

    if (!m_viewportArguments.height) {
        // m_viewportArguments.height == 0 does not indicate an valid viewport height.
        virtualViewportHeight = m_page->chrome()->client()->platformScreenSize().height();
    } else if (m_viewportArguments.height < 0)
        virtualViewportHeight = static_cast<int>(ceilf(m_defaultLayoutSize.height()*virtualViewportWidth/static_cast<float>(m_defaultLayoutSize.width())));

    // If the viewport arguments conflict with the scale arguments,
    // then the latter take precedence
    if (initialScaleIsExplicit) {
        // The purpose of this block is to adapt the fixed layout size to match the
        // initial-scale, so we know that the contents size will expand to
        // allow zooming to the zoom factor specified by the initial-scale.
        double scale = initialScale();

        // Clamp the initial scale in case it conflicts with the minimum or maximum
        // Use m_minimumScale instead of minimumScale() because it calls zoomToFitScale()
        // which returns 1.0 if we have not yet computed our contents size.
        // Using 1.0 for clamping would prevents us from honoring an initial-scale
        // less than 1.0. Same thing goes for m_maximumScale vs. maximumScale()
        if (minimumScaleIsExplicit && scale < m_minimumScale)
            scale = m_minimumScale;
        else if (maximumScaleIsExplicit && scale > m_maximumScale)
            scale = m_maximumScale;

        // Ensure that the combination of viewport and scale combine to fill our real
        // viewport and if not, adjust the viewport to do so.
        virtualViewportWidth = std::max<int>(virtualViewportWidth, floor(m_defaultLayoutSize.width() / scale));
        virtualViewportHeight = std::max<int>(virtualViewportHeight, floor(m_defaultLayoutSize.height() / scale));
    }

    virtualViewportWidth = std::min(virtualViewportWidth, maximumLayoutSize.width());
    virtualViewportHeight = std::min(virtualViewportHeight, maximumLayoutSize.height());

    return IntSize(virtualViewportWidth, virtualViewportHeight);
}

void WebPagePrivate::didReceiveViewportArguments(const WebCore::ViewportArguments& arguments)
{
    if (!arguments.hasCustomArgument())
        return;

    m_viewportArguments = arguments;

    setUserScalable(arguments.userScalable == ViewportArguments::ValueUndefined ? true : arguments.userScalable);
    if (arguments.initialScale > 0)
        setInitialScale(arguments.initialScale);
    if (arguments.minimumScale > 0)
        setMinimumScale(arguments.minimumScale);
    if (arguments.maximumScale > 0)
        setMaximumScale(arguments.maximumScale);

    IntSize virtualViewport = recomputeVirtualViewportFromViewportArguments();
    m_webPage->setVirtualViewportSize(virtualViewport.width(), virtualViewport.height());

    if (loadState() == Olympia::WebKit::WebPagePrivate::Committed)
        zoomToInitialScaleOnLoad();
}

void WebPage::setScreenRotated(const Platform::IntSize& screenSize, const Platform::IntSize& defaultLayoutSize, const Platform::IntRect& viewportRect)
{
    int screenWidth = screenSize.width();
    int screenHeight = screenSize.height();
    if (screenWidth == d->m_width && screenHeight == d->m_height)
        return;

    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "WebPage::setScreenRotated screenWidth=%d, screenHeight=%d viewport=(%d,%d %dx%d)",
        screenWidth, screenHeight, viewportRect.x(), viewportRect.y(), viewportRect.width(), viewportRect.height());

    // This method should only be called by client when we've rotated the device
    ASSERT(d->m_width > d->m_height ? screenWidth < screenHeight : screenWidth > screenHeight);

    d->setScreenRotated(screenSize, defaultLayoutSize, IntSize(viewportRect.width(), viewportRect.height()));
}

void WebPagePrivate::setScreenRotated(const WebCore::IntSize& screenSize, const WebCore::IntSize& defaultLayoutSize, const WebCore::IntSize& transformedActualVisibleSize)
{
    // The screen rotation is a major state transition that in this case is not properly
    // communicated to the backing store, since it does early return in most methods when
    // not visible.
    if (!m_visible)
        m_shouldResetTilesWhenShown = true;

    IntSize viewportSizeBefore = viewportSize();
    FloatPoint centerOfVisibleContentsRect = this->centerOfVisibleContentsRect();

    bool atInitialScale = currentScale() == initialScale();
    bool atTop = scrollPosition().y() == 0;
    bool atLeft = scrollPosition().x() == 0;

    m_width = screenSize.width();
    m_height = screenSize.height();

    WebSettings* settings = WebSettings::pageGroupSettings(m_page->groupName());
    settings->setScreenWidth(screenSize.width());
    settings->setScreenHeight(screenSize.height());

    // We need to reorient the visibleTileRect because the following code
    // could cause BackingStore::transformChanged to be called, where it
    // is used.
    // It is only dependent on the transformedViewportSize (m_width, m_height)
    // which has been updated by now.
    m_backingStore->d->createVisibleTileBuffer();

    setDefaultLayoutSize(defaultLayoutSize);

    // Recompute our virtual viewport
    bool needsLayout = false;
    if (m_viewportArguments.hasCustomArgument()) {
        // We may need to infer the width and height for the viewport with respect to the rotation.
        IntSize newVirtualViewport = recomputeVirtualViewportFromViewportArguments();
        ASSERT(!newVirtualViewport.isEmpty());
        m_webPage->setVirtualViewportSize(newVirtualViewport.width(), newVirtualViewport.height());
        m_mainFrame->view()->setUseFixedLayout(useFixedLayout());
        m_mainFrame->view()->setFixedLayoutSize(fixedLayoutSize());
        needsLayout = true;
    }

    // We switch this strictly after recomputing our virtual viewport as zoomToFitScale is dependent
    // upon these values and so is the virtual viewport recalculation
    m_actualVisibleWidth = transformedActualVisibleSize.width();
    m_actualVisibleHeight = transformedActualVisibleSize.height();

    IntSize viewportSizeAfter = viewportSize();

    IntPoint offset(roundf((viewportSizeBefore.width() - viewportSizeAfter.width()) / 2.0),
                    roundf((viewportSizeBefore.height() - viewportSizeAfter.height()) / 2.0));

    // Suspend all screen updates to the backingstore
    m_backingStore->d->suspendScreenUpdates();

    // As a special case if we were anchored to the top left position at the beginning
    // of the rotation then preserve that anchor
    if (atTop)
        offset.setY(0);
    if (atLeft)
        offset.setX(0);

    // We need to update the viewport size of the WebCore::ScrollView...
    updateViewportSize();

    // ...before scrolling, because the backing store will align its
    // tile matrix with the viewport as reported by the ScrollView.
    scrollBy(offset.x(), offset.y());

    m_backingStore->d->orientationChanged();
    m_backingStore->d->actualVisibleSizeChanged(transformedActualVisibleSize);

    // Update view mode only after we have updated the actual
    // visible size and reset the contents rect if necessary.
    if (setViewMode(viewMode()))
        needsLayout = true;

    // If automatic zooming is disabled, prevent zooming below
    if (!m_webSettings->isZoomToFitOnLoad()) {
        atInitialScale = false;

        // Normally, if the contents size is smaller than the layout width,
        // we would zoom in. If zoom is disabled, we need to do something else,
        // or there will be artifacts due to non-rendered areas outside of the
        // contents size. If there is a virtual viewport, we are not allowed
        // to modify the fixed layout size, however.
        if (!hasVirtualViewport() && contentsSize().width() < m_defaultLayoutSize.width()) {
            m_mainFrame->view()->setUseFixedLayout(useFixedLayout());
            m_mainFrame->view()->setFixedLayoutSize(m_defaultLayoutSize);
            needsLayout = true;
        }
    }

    if (needsLayout)
        setNeedsLayout();

    // Need to resume so that the backingstore will start recording the invalidated
    // rects from below
    m_backingStore->d->resumeScreenUpdates(BackingStorePrivate::None);

    // We might need to layout here to get a correct contentsSize so that zoomToFit
    // is calculated correctly
    requestLayoutIfNeeded();

    // As a special case if we were zoomed to the initial scale at the beginning
    // of the rotation then preserve that zoom level even when it is zoomToFit
    double scale = atInitialScale ? initialScale() : currentScale();

    // As a special case if we were anchored to the top left position at the beginning
    // of the rotation then preserve that anchor
    FloatPoint anchor = centerOfVisibleContentsRect;
    if (atTop)
        anchor.setY(0);
    if (atLeft)
        anchor.setX(0);

    // Try and zoom here with clamping on
    if (!scheduleZoomAboutPoint(scale, anchor, true /*enforceScaleClamping*/, true /*forceRendering*/)) {
        // Suspend all screen updates to the backingstore
        m_backingStore->d->suspendScreenUpdates();

        // If the zoom failed, then we should still preserve the special case of scroll position
        IntPoint scrollPosition = this->scrollPosition();
        if (atTop)
            scrollPosition.setY(0);
        if (atLeft)
            scrollPosition.setX(0);
        setScrollPosition(scrollPosition);

        // These might have been altered even if we didn't zoom so notify the client
        notifyTransformedContentsSizeChanged();
        notifyTransformedScrollChanged();

        if (!needsLayout) {
            // The visible tiles for scroll must be up-to-date before we blit since we are not performing a layout.
            m_backingStore->d->updateTilesForScrollOrNotRenderedRegion();
        }
        m_inputHandler->ensureFocusElementVisible();

        if (needsLayout) {
            m_backingStore->d->resetTiles(true);
            m_backingStore->d->updateTiles(false /*updateVisible*/, false /*immediate*/);
        }

        // If we need layout then render and blit, otherwise just blit as our viewport has changed
        m_backingStore->d->resumeScreenUpdates(needsLayout ? BackingStorePrivate::RenderAndBlit : BackingStorePrivate::Blit);
    } else
        m_inputHandler->ensureFocusElementVisible();
}

void WebPage::setScreenOrientation(int orientation)
{
#if ENABLE(ORIENTATION_EVENTS)
    if (d->m_mainFrame->orientation() == orientation)
        return;
    d->m_mainFrame->sendOrientationChangeEvent(orientation);
#endif
}

void WebPage::setDefaultLayoutSize(int width, int height)
{
    IntSize size(width, height);
    if (size == d->m_defaultLayoutSize)
        return;

    d->setDefaultLayoutSize(size);
    bool needsLayout = d->setViewMode(d->viewMode());
    if (needsLayout) {
        d->setNeedsLayout();
        if (!d->isLoading())
            d->requestLayoutIfNeeded();
    }
}

void WebPagePrivate::setDefaultLayoutSize(const WebCore::IntSize& size)
{
    ASSERT(size.width() <= m_width && size.height() <= m_height);
    m_defaultLayoutSize = size.expandedTo(minimumLayoutSize).shrunkTo(IntSize(m_width, m_height));
}

void WebPage::setPlatformScreenSize(int width, int height)
{
    d->m_page->chrome()->client()->setPlatformScreenSize(WebCore::IntSize(width, height));
}

void WebPage::setApplicationViewSize(int width, int height)
{
    WebSettings::pageGroupSettings(d->m_page->groupName())->setApplicationViewSize(IntSize(width, height));
}

void WebPage::setFocused(bool focused)
{
    WebCore::FocusController* focusController = d->m_page->focusController();
    focusController->setActive(focused);
    if (focused) {
        Frame* frame = focusController->focusedFrame();
        if (!frame)
            focusController->setFocusedFrame(d->m_mainFrame);
    }
    focusController->setFocused(focused);
}

void WebPage::mouseEvent(MouseEventType type, const Platform::IntPoint& pos, const Platform::IntPoint& globalPos)
{
    if (!d->m_mainFrame->view())
        return;

    if (type == Olympia::WebKit::MouseEventAborted) {
        d->m_mainFrame->eventHandler()->setMousePressed(false);
        return;
    }

    d->m_lastUserEventTimestamp = currentTime();
    int clickCount = (d->m_selectionHandler->isSelectionActive() || type != MouseEventMoved) ? 1 : 0;

    // Create our event.
    WebCore::PlatformMouseEvent mouseEvent(d->mapFromTransformed(pos), d->mapFromTransformed(globalPos), toWebCoreMouseEventType(type), clickCount);
    d->m_lastMouseEvent = mouseEvent;
    d->handleMouseEvent(mouseEvent);
}

void WebPagePrivate::handleMouseEvent(WebCore::PlatformMouseEvent& mouseEvent)
{
    WebCore::EventHandler* eventHandler = m_mainFrame->eventHandler();

    switch (mouseEvent.eventType()) {
    case WebCore::MouseEventPressed:
        eventHandler->handleMousePressEvent(mouseEvent);
        sendContextIfChanged(eventHandler->mousePressNode());
        break;
    case WebCore::MouseEventReleased:
    {
        HitTestResult result = eventHandler->hitTestResultAtPoint(mapFromViewportToContents(mouseEvent.pos()), false);
        if (!didNodeOpenPopup(result.innerNode()))
            eventHandler->handleMouseReleaseEvent(mouseEvent);
        break;
    }
    case WebCore::MouseEventMoved:
    default:
        eventHandler->mouseMoved(mouseEvent);
        break;
    }
}

bool WebPage::touchEvent(Olympia::Platform::TouchEvent& event)
{
    if (!d->m_mainFrame)
        return false;

    d->m_lastUserEventTimestamp = currentTime();

    for (int i = 0; i < event.m_points.size(); i++) {
        event.m_points[i].m_pos = d->mapFromTransformed(event.m_points[i].m_pos);
        event.m_points[i].m_screenPos = d->mapFromTransformed(event.m_points[i].m_screenPos);
    }

#if ENABLE(TOUCH_EVENTS)
    bool handled = d->m_mainFrame->eventHandler()->handleTouchEvent(PlatformTouchEvent(&event));
    if (d->m_preventDefaultOnTouchStart) {
        if (event.m_type == Olympia::Platform::TouchEvent::TouchEnd || event.m_type == Olympia::Platform::TouchEvent::TouchCancel)
            d->m_preventDefaultOnTouchStart = false;
        return true;
    }

    if (handled) {
        if (event.m_type == Olympia::Platform::TouchEvent::TouchStart)
            d->m_preventDefaultOnTouchStart = true;
        return true;
    }
#endif

    return d->m_touchEventHandler->handleTouchEvent(event);
}

Frame* WebPagePrivate::focusedOrMainFrame()
{
    return m_page->focusController()->focusedOrMainFrame();
}

Frame* WebPagePrivate::mainFrame()
{
    return m_mainFrame;
}

void WebPagePrivate::clearFocusNode()
{
    Frame* frame = focusedOrMainFrame();
    if (!frame)
        return;
    ASSERT(frame->document());

    if (frame->document()->focusedNode())
        frame->page()->focusController()->setFocusedNode(0, frame);
}

bool WebPagePrivate::scrollNodeRecursively(WebCore::Node* node, const WebCore::IntSize& delta)
{
    if (delta.isZero())
        return true;
    if (!node)
        return false;

    WebCore::RenderObject* renderer = node->renderer();
    if (!renderer)
        return false;
    // Try scrolling the renderer.
    if (scrollRenderer(renderer, delta))
        return true;

    WebCore::FrameView* view = renderer->view()->frameView();
    if (!view)
        return false;

    // We've hit the page, don't scroll it and return false.
    if (view == m_mainFrame->view())
        return false;

    // Try scrolling the FrameView
    WebCore::IntSize viewDelta = delta;
    IntPoint newViewOffset = view->scrollPosition();
    IntPoint maxViewOffset = view->maximumScrollPosition();
    adjustScrollDelta(maxViewOffset, newViewOffset, viewDelta);

    if (!viewDelta.isZero()) {
        view->setCanBlitOnScroll(false);
        view->scrollBy(viewDelta);
        return true;
    }

    // Try scrolling the node of the enclosing frame
    Frame* frame = node->document()->frame();
    if (frame) {
        Node* ownerNode = static_cast<Node*>(frame->ownerElement());
        if (scrollNodeRecursively(ownerNode, delta))
            return true;
    }

    return false;
}

void WebPagePrivate::adjustScrollDelta(const WebCore::IntPoint& maxOffset, const WebCore::IntPoint& currentOffset, WebCore::IntSize& delta)
{
    if (currentOffset.x() + delta.width() > maxOffset.x())
        delta.setWidth(max(maxOffset.x() - currentOffset.x(), 0));

    if (currentOffset.x() + delta.width() < 0)
        delta.setWidth(min(-currentOffset.x(), 0));

    if (currentOffset.y() + delta.height() > maxOffset.y())
        delta.setHeight(max(maxOffset.y() - currentOffset.y(), 0));

    if (currentOffset.y() + delta.height() < 0)
        delta.setHeight(min(-currentOffset.y(), 0));
}

bool WebPagePrivate::scrollRenderer(WebCore::RenderObject* renderer, const WebCore::IntSize& delta)
{
    RenderLayer* layer = renderer->enclosingLayer();
    if (!layer)
        return false;

    // Try to scroll layer
    bool restrictedByLineClamp = false;
    if (renderer->parent())
        restrictedByLineClamp = !renderer->parent()->style()->lineClamp().isNone();

    if (renderer->hasOverflowClip() && !restrictedByLineClamp) {
        WebCore::IntSize layerDelta = delta;
        WebCore::IntPoint maxOffset(layer->scrollWidth() - layer->renderBox()->clientWidth(), layer->scrollHeight() - layer->renderBox()->clientHeight());
        WebCore::IntPoint currentOffset(layer->scrollXOffset(), layer->scrollYOffset());
        adjustScrollDelta(maxOffset, currentOffset, layerDelta);
        if (!layerDelta.isZero()) {
            WebCore::IntPoint newOffset = currentOffset + layerDelta;
            layer->scrollToOffset(newOffset.x(), newOffset.y());
            m_mainFrame->eventHandler()->updateAutoscrollRenderer();
            return true;
        }
    }

    // Nothing Scrollable try the parent layer.
    RenderObject* nextRenderer = renderer->parent();
    while (nextRenderer) {
        if (nextRenderer->isBox() && toRenderBox(nextRenderer)->canBeScrolledAndHasScrollableArea())
            return scrollRenderer(nextRenderer, delta);
        nextRenderer = nextRenderer->parent();
    }

    return false;
}

bool WebPage::keyEvent(Olympia::Platform::KeyboardEvent::Type type, const unsigned short character, bool shiftDown)
{
    if (!d->m_mainFrame->view())
        return false;

    ASSERT(d->m_page->focusController());
    return d->m_inputHandler->handleKeyboardInput(toWebCorePlatformKeyboardEventType(type), character, shiftDown);
}

void WebPage::navigationMoveEvent(const unsigned short character, bool shiftDown, bool altDown)
{
    if (!d->m_mainFrame->view())
        return;

    ASSERT(d->m_page->focusController());
    d->m_inputHandler->handleNavigationMove(character, shiftDown, altDown);
}

void WebPage::selectionCancelled()
{
    d->m_selectionHandler->cancelSelection();
}

Olympia::WebKit::String WebPage::selectedText() const
{
    return d->m_selectionHandler->selectedText();
}

void WebPage::setSelection(const Platform::IntPoint& startPoint, const Platform::IntPoint& endPoint)
{
    // Transform this events coordinates to webkit content coordinates.
    d->m_selectionHandler->setSelection(d->mapFromTransformed(startPoint), d->mapFromTransformed(endPoint));
}

void WebPage::selectAtPoint(const Platform::IntPoint& location)
{
    // Transform this events coordinates to webkit content coordinates.
    WebCore::IntPoint transformedLocation = d->mapFromTransformed(location);
    d->m_selectionHandler->selectAtPoint(transformedLocation);
}

// returned caret rect in transformed coordinates
Olympia::Platform::IntRect WebPage::selectionStartCaretRect()
{
    WebCore::IntRect rect = d->mapToTransformed(d->m_selectionHandler->selectionStartCaretRect());
    d->clipToTransformedContentsRect(rect);
    return Olympia::Platform::IntRect(rect);
}

// returned caret rect in transformed coordinates
Olympia::Platform::IntRect WebPage::selectionEndCaretRect()
{
    WebCore::IntRect rect = d->mapToTransformed(d->m_selectionHandler->selectionEndCaretRect());
    d->clipToTransformedContentsRect(rect);
    return Olympia::Platform::IntRect(rect);
}

// returned scroll position is in transformed coordinates
Platform::IntPoint WebPage::scrollPosition() const
{
    return d->transformedScrollPosition();
}

// setting the scroll position is in transformed coordinates
void WebPage::setScrollPosition(const Platform::IntPoint& point)
{
    if (d->transformedPointEqualsUntransformedPoint(point, d->scrollPosition()))
        return;

    d->m_touchEventHandler->setDidScrollPage(true);

    // If the user recently performed an event, this new scroll position
    // could possibly be a result of that. Or not, this is just a heuristic.
    if (currentTime() - d->m_lastUserEventTimestamp < manualScrollInterval)
        d->m_userPerformedManualScroll = true;

    d->m_mainFrame->view()->setCanOverscroll(true);
    d->setScrollPosition(d->mapFromTransformed(point));
    d->m_mainFrame->view()->setCanOverscroll(false);
}

BackingStore* WebPage::backingStore() const
{
    return d->m_backingStore;
}

// FIXME: We need to snap to 1:1 and minimum scale instead of skipping them...
bool WebPage::zoomIn()
{
    if (!d->isUserScalable())
        return false;

    d->m_userPerformedManualZoom = true;
    d->m_userPerformedManualScroll = true;
    double scale = d->m_transformationMatrix->m11() + 0.2;

    FloatPoint center = d->mapToTransformedFloatPoint(d->centerOfVisibleContentsRect());

    return bitmapZoom(center.x(), center.y(), scale / d->m_bitmapScale, true);
}

bool WebPage::zoomOut()
{
    if (d->contentsSize().isEmpty() || !d->isUserScalable())
        return false;

    double minimumScale = d->minimumScale();

    d->m_userPerformedManualZoom = true;
    d->m_userPerformedManualScroll = true;
    double scale = d->m_transformationMatrix->m11() - 0.2;

    if (scale < minimumScale)
        scale = minimumScale;

    FloatPoint center = d->mapToTransformedFloatPoint(d->centerOfVisibleContentsRect());

    return bitmapZoom(center.x(), center.y(), scale / d->m_bitmapScale, true);
}

bool WebPage::zoomToFit()
{
    if (d->contentsSize().isEmpty() || !d->isUserScalable())
        return false;

    d->m_userPerformedManualZoom = true;
    return d->zoomAboutPoint(d->zoomToFitScale(), d->centerOfVisibleContentsRect());
}

#if ENABLE(VIEWPORT_REFLOW)
void WebPagePrivate::toggleTextReflowIfTextReflowEnabledOnlyForBlockZoom(bool shouldEnableTextReflow)
{
    if (m_webPage->settings()->textReflowMode() == WebSettings::TextReflowEnabledOnlyForBlockZoom)
        m_page->settings()->setTextReflowEnabled(shouldEnableTextReflow);
}
#endif

bool WebPage::bitmapZoom(int x, int y, double scale, bool shouldZoomAboutPoint)
{
    d->m_touchEventHandler->setDidZoomPage(true);

    if (d->contentsSize().isEmpty() || !d->isUserScalable())
        return false;

    // Adjust the scale to clamp to the min and max zoom factors
    bool shouldRenderFrame = (scale != 1.0);
    // Note: the minimum bitmap scale is compensated for overscaling here. This could be calculated in a more intelligent fashion.
    double minimumBitmapScale = d->minimumScale() * 0.75;
    if ((scale < 1.0 && d->m_bitmapScale == minimumBitmapScale) || (scale > 1.0 && d->m_bitmapScale == d->maximumScale()))
        shouldRenderFrame = false;
    else if (d->m_bitmapScale * scale < minimumBitmapScale)
        d->m_bitmapScale = minimumBitmapScale;
    else if (d->m_bitmapScale * scale > d->maximumScale())
        d->m_bitmapScale = d->maximumScale();
    else
        d->m_bitmapScale = d->m_bitmapScale * scale;

    IntPoint anchor(x,y);

    if (!d->m_bitmapZoomBuffer) {
        d->m_bitmapZoomBuffer = new unsigned short[d->transformedVisibleContentsRect().width() * d->transformedVisibleContentsRect().height() * 2];
        d->m_bitmapZoomBufferStride = d->transformedVisibleContentsRect().width() * 2;
    }

    if (shouldRenderFrame) {
        // Prohibit backingstore from updating the canvas overtop of the bitmap
        d->m_backingStore->d->suspendScreenUpdates();

        // Scale is the incremental change in zoom level.
        d->renderBitmapZoomFrame(d->m_bitmapScale, FloatPoint(anchor));
    }

    if (!shouldZoomAboutPoint)
        return false;

    double minimumScale = d->minimumScale();

    if (d->m_bitmapScale < minimumScale)
        d->animateZoomBounceBack(FloatPoint(anchor));
    else {
        d->m_userPerformedManualZoom = true;
        d->m_userPerformedManualScroll = true;
        delete [] d->m_bitmapZoomBuffer;
        d->m_bitmapZoomBuffer = 0;
        d->m_bitmapZoomBufferStride = 0;

        if (d->m_webPage->settings()->textReflowMode() == WebSettings::TextReflowEnabled) {
            // This is a hack for email which has reflow always turned on
            d->m_currentPinchZoomNode = d->nodeForZoomUnderPoint(anchor);
            IntPoint nodeOrigin = d->rectForNode(d->m_currentPinchZoomNode.get()).topLeft();
            IntPoint point(0, d->mapFromTransformed(anchor).y() - nodeOrigin.y());
            d->m_currentPinchZoomCaretPosition = d->m_currentPinchZoomNode->renderer()->positionForPoint(point);
        }

        d->zoomAboutPoint(d->m_bitmapScale, d->mapFromTransformed(anchor));
    }

    return true;
}

bool WebPage::blockZoom(int x, int y)
{
    d->m_touchEventHandler->setDidZoomPage(true);

    if (!d->m_mainFrame->view() || !d->isUserScalable())
        return false;

    Node* node = d->bestNodeForZoomUnderPoint(IntPoint(x, y));
    if (!node)
        return false;

    IntRect nodeRect = d->rectForNode(node);
    IntRect blockRect;
    bool endOfBlockZoomMode = d->compareNodesForBlockZoom(d->m_currentBlockZoomAdjustedNode.get(), node);
    const double oldScale = d->m_transformationMatrix->m11();
    double newScale = 0.0;
    const double margin = endOfBlockZoomMode ? 0 : blockZoomMargin * 2 * oldScale;
    bool isFirstZoom = false;

    if (endOfBlockZoomMode) {
        // End of block zoom mode
        IntRect rect = d->blockZoomRectForNode(node);
        blockRect = IntRect(0, rect.y(), d->transformedContentsSize().width(), d->transformedContentsSize().height() - rect.y());
        d->m_shouldReflowBlock = false;
    } else {
        // Start/continue block zoom mode
        Node* tempBlockZoomAdjustedNode = d->m_currentBlockZoomAdjustedNode.get();
        blockRect = d->blockZoomRectForNode(node);

        // Don't use a block if it is too close to the size of the actual contents.
        // We allow this for images only so that they can be zoomed tight to the screen.
        if (!node->hasTagName(HTMLNames::imgTag)) {
            IntRect tRect = d->mapFromTransformed(blockRect);
            int blockArea = tRect.width() * tRect.height();
            int pageArea = d->contentsSize().width() * d->contentsSize().height();
            double blockToPageRatio = (double)(pageArea - blockArea) / pageArea;
            if (blockToPageRatio < 0.15) {
                // restore old adjust node because zoom was canceled
                d->m_currentBlockZoomAdjustedNode = tempBlockZoomAdjustedNode;
                return false;
            }
        }

        if (blockRect.isEmpty() || blockRect.width() == 0 || blockRect.height() == 0)
            return false;

        if (!d->m_currentBlockZoomNode.get())
            isFirstZoom = true;

        d->m_currentBlockZoomNode = node;
        d->m_shouldReflowBlock = true;
    }

    newScale = std::min(d->newScaleForBlockZoomRect(blockRect, oldScale, margin), d->maxBlockZoomScale());
    newScale = std::max(newScale, minimumScale());

#if DEBUG_BLOCK_ZOOM
    // render the double tap point for visual reference
    IntRect renderRect(x, y, 1, 1);
    renderRect = d->mapFromTransformedContentsToTransformedViewport(renderRect);
    IntSize viewportSize = d->transformedViewportSize();
    renderRect.intersect(IntRect(0, 0, viewportSize.width(), viewportSize.height()));
    d->clearCanvas(renderRect, 0x0000);
    d->invalidateWindow(renderRect);

    // uncomment this to return in order to see the blocks being selected
//    d->m_client->zoomChanged(isMinZoomed(), isMaxZoomed(), isAtInitialZoom(), currentZoomLevel());
//    return true;
#endif

#if ENABLE(VIEWPORT_REFLOW)
    // If reflowing, adjust the reflow-width of text node to make sure the font is a reasonable size
    if (d->m_currentBlockZoomNode && d->m_shouldReflowBlock && settings()->textReflowMode() != WebSettings::TextReflowDisabled) {
        RenderObject *renderer = d->m_currentBlockZoomNode->renderer();
        if (renderer && renderer->isText()) {
            double newFontSize = renderer->style()->fontSize() * newScale;
            if (newFontSize < d->m_webSettings->defaultFontSize()) {
                newScale = std::min(static_cast<double>(d->m_webSettings->defaultFontSize()) / renderer->style()->fontSize(), d->maxBlockZoomScale());
                newScale = std::max(newScale, minimumScale());
            }
            blockRect.setWidth(oldScale * static_cast<double>(d->transformedActualVisibleSize().width()) / newScale);
            // Re-calculate the scale here to take in to account the margin
            newScale = std::min(d->newScaleForBlockZoomRect(blockRect, oldScale, margin), d->maxBlockZoomScale());
            newScale = std::max(newScale, minimumScale()); // Still, it's not allowed to be smaller than minimum scale
        }
    }
#endif

    FloatPoint anchor(d->mapFromTransformed(blockRect).topLeft());

    // Align the zoomed block in the screen.
    double newBlockHeight = d->mapFromTransformed(blockRect).height();
    double newBlockWidth = d->mapFromTransformed(blockRect).width();
    double scaledViewportWidth = static_cast<double>(d->actualVisibleSize().width()) * oldScale / newScale;
    double scaledViewportHeight = static_cast<double>(d->actualVisibleSize().height()) * oldScale / newScale;
    double dx = std::max(0.0, (scaledViewportWidth - newBlockWidth) / 2.0);
    double dy = std::max(0.0, (scaledViewportHeight - newBlockHeight) / 2.0);
    if (newBlockHeight <= scaledViewportHeight) {
        // The block fits in the viewport so center it.
        d->m_finalBlockPoint = FloatPoint(anchor.x() - dx, anchor.y() - dy);
    } else {
        // The block is longer than the viewport so top align it and add 3 pixel margin.
        d->m_finalBlockPoint = FloatPoint(anchor.x() - dx, anchor.y() - 3);
    }

#if ENABLE(VIEWPORT_REFLOW)
    // we dont know how long the reflowed block will be so we position it at the top of the screen with a small margin
    if (settings()->textReflowMode() != WebSettings::TextReflowDisabled) {
        d->m_finalBlockPoint = FloatPoint(anchor.x() - dx, anchor.y() - 3);
        d->m_finalBlockPointReflowOffset = FloatPoint(-dx, -3);
    }
#endif

    // Make sure that the original node rect is visible in the screen after the zoom. This is necessary because the identified block rect might
    // not be the same as the original node rect, and it could force the original node rect off the screen.
    FloatRect br(anchor, FloatSize(scaledViewportWidth, scaledViewportHeight));
    IntPoint clickPoint = d->mapFromTransformed(IntPoint(x, y));
    if (!br.contains(clickPoint)) {
        d->m_finalBlockPointReflowOffset.move(0, (clickPoint.y() - scaledViewportHeight / 2) - d->m_finalBlockPoint.y());
        d->m_finalBlockPoint = FloatPoint(d->m_finalBlockPoint.x(), clickPoint.y() - scaledViewportHeight / 2);
    }

    // Clamp the finalBlockPoint to not cause any overflow scrolling
    if (d->m_finalBlockPoint.x() < 0) {
        d->m_finalBlockPoint.setX(0);
        d->m_finalBlockPointReflowOffset.setX(0);
    } else if (d->m_finalBlockPoint.x() + scaledViewportWidth > d->contentsSize().width()) {
        d->m_finalBlockPoint.setX(d->contentsSize().width() - scaledViewportWidth);
        d->m_finalBlockPointReflowOffset.setX(0);
    }

    if (d->m_finalBlockPoint.y() < 0) {
        d->m_finalBlockPoint.setY(0);
        d->m_finalBlockPointReflowOffset.setY(0);
    } else if (d->m_finalBlockPoint.y() + scaledViewportHeight > d->contentsSize().height()) {
        d->m_finalBlockPoint.setY(d->contentsSize().height() - scaledViewportHeight);
        d->m_finalBlockPointReflowOffset.setY(0);
    }

    d->m_finalBlockPoint = d->mapToTransformedFloatPoint(d->m_finalBlockPoint);

    // Don't block zoom if the user is zooming and the new scale is only marginally different from the
    // oldScale with only a marginal change in scroll position. Ignore scroll difference in the special case
    // that the zoom level is the minimumScale
    if (!endOfBlockZoomMode && abs(newScale - oldScale) / oldScale < 0.15) {
        const double minimumDisplacement = 0.15 * d->transformedActualVisibleSize().width();
        if (oldScale == d->minimumScale() || (distanceBetweenPoints(roundTransformedPoint(d->mapToTransformed(d->scrollPosition())), roundTransformedPoint(d->m_finalBlockPoint)) < minimumDisplacement && abs(newScale - oldScale) / oldScale < 0.10)) {
            if (isFirstZoom) {
                d->resetBlockZoom();
                return false;
            } else {
                // zoom out of block zoom
                blockZoom(x, y);
                return true;
            }
        }
    }

    d->m_blockZoomInitialScale = oldScale;
    d->m_blockZoomFinalScale = newScale;

    // We set this here to make sure we don't try to re-render the page at a different zoom level during loading
    d->m_userPerformedManualZoom = true;
    d->m_userPerformedManualScroll = true;
    d->animateBlockZoom();

    return true;
}

bool WebPage::isMaxZoomed() const
{
    return (d->currentScale() == d->maximumScale()) || !d->isUserScalable();
}

bool WebPage::isMinZoomed() const
{
    return (d->currentScale() == d->minimumScale()) || !d->isUserScalable();
}

bool WebPage::isAtInitialZoom() const
{
    return (d->currentScale() == d->initialScale()) || !d->isUserScalable();
}

bool WebPagePrivate::shouldZoomOnEscape() const
{
    if (!isUserScalable())
        return false;

    // If the initial scale is not reachable, don't try to zoom
    if (initialScale() < minimumScale() || initialScale() > maximumScale())
        return false;

    // Don't ever zoom in when we press escape
    if (initialScale() >= currentScale())
        return false;

    return currentScale() != initialScale();
}

void WebPage::zoomToInitialScale()
{
    if (!d->isUserScalable())
        return;

    d->zoomAboutPoint(d->initialScale(), d->centerOfVisibleContentsRect());
}

double WebPage::currentZoomLevel() const
{
    return d->m_bitmapScale;
}

bool WebPage::zoomToOneOne()
{
    if (!d->isUserScalable())
        return false;

    double scale = 1.0;
    return d->zoomAboutPoint(scale, d->centerOfVisibleContentsRect());
}

bool WebPage::moveToNextField(Olympia::Platform::ScrollDirection dir, int desiredScrollAmount)
{
    return d->moveToNextField(dir, desiredScrollAmount);
}

Olympia::Platform::IntRect WebPage::focusNodeRect()
{
    return d->focusNodeRect();
}

bool WebPage::focusField(bool focus)
{
    return d->focusField(focus);
}

bool WebPage::linkToLinkOnClick()
{
    Frame* frame = d->focusedOrMainFrame();
    if (!frame)
        return false;

    Document* doc = frame->document();
    if (!doc)
        return false;

    Node* focusedNode = doc->focusedNode();  

    if (!focusedNode)
        return false;

    // Make sure the node is visible before triggering the click.
    WebCore::IntRect visibleRect = IntRect(IntPoint(), d->actualVisibleSize());
    // Constrain the rect if this is a subframe.
    if (frame->view()->parent())
        visibleRect.intersect(getRecursiveVisibleWindowRect(frame->view()));

    if (!visibleRect.intersects(getNodeWindowRect(focusedNode)))
        return false;

    // Create click event.
    focusedNode->dispatchSimulatedClick(0, true);
    return true;
}

bool WebPage::findNextString(const char* text)
{
    return d->m_selectionHandler->findNextString(WebCore::String::fromUTF8(text));
}

bool WebPage::findNextUnicodeString(const unsigned short* text)
{
    return d->m_selectionHandler->findNextString(WebCore::String(text));
}

void WebPage::runLayoutTests()
{
    if (!d->m_dumpRenderTree)
        d->m_dumpRenderTree = new DumpRenderTree(this);
    d->m_dumpRenderTree->runTests();
}

bool WebPage::enableScriptDebugger()
{
#if ENABLE(JAVASCRIPT_DEBUGGER)
    if (d->m_scriptDebugger)
        return true;

    d->m_scriptDebugger.set(new JavaScriptDebuggerOlympia(this));

    return !!d->m_scriptDebugger;
#endif
}

bool WebPage::disableScriptDebugger()
{
#if ENABLE(JAVASCRIPT_DEBUGGER)
    if (!d->m_scriptDebugger)
        return true;

    d->m_scriptDebugger.clear();
    return true;
#endif
}

void WebPage::addBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber, const unsigned short* condition, unsigned conditionLength)
{
#if ENABLE(JAVASCRIPT_DEBUGGER)
    if (d->m_scriptDebugger)
        d->m_scriptDebugger->addBreakpoint(url, urlLength, lineNumber, condition, conditionLength);
#endif
}

void WebPage::updateBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber, const unsigned short* condition, unsigned conditionLength)
{
#if ENABLE(JAVASCRIPT_DEBUGGER)
    if (d->m_scriptDebugger)
        d->m_scriptDebugger->updateBreakpoint(url, urlLength, lineNumber, condition, conditionLength);
#endif
}

void WebPage::removeBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber)
{
#if ENABLE(JAVASCRIPT_DEBUGGER)
    if (d->m_scriptDebugger)
        d->m_scriptDebugger->removeBreakpoint(url, urlLength, lineNumber);
#endif
}

bool WebPage::pauseOnExceptions()
{
#if ENABLE(JAVASCRIPT_DEBUGGER)
    return d->m_scriptDebugger ? d->m_scriptDebugger->pauseOnExceptions() : false;
#endif
}

void WebPage::setPauseOnExceptions(bool pause)
{
#if ENABLE(JAVASCRIPT_DEBUGGER)
    if (d->m_scriptDebugger)
        d->m_scriptDebugger->setPauseOnExceptions(pause);
#endif
}

void WebPage::pauseInDebugger()

{
#if ENABLE(JAVASCRIPT_DEBUGGER)
    if (d->m_scriptDebugger)
        d->m_scriptDebugger->pauseInDebugger();
#endif
}

void WebPage::resumeDebugger()
{
#if ENABLE(JAVASCRIPT_DEBUGGER)
    if (d->m_scriptDebugger)
        d->m_scriptDebugger->resumeDebugger();
#endif
}

void WebPage::stepOverStatementInDebugger()
{
#if ENABLE(JAVASCRIPT_DEBUGGER)
    if (d->m_scriptDebugger)
        d->m_scriptDebugger->stepOverStatementInDebugger();
#endif
}

void WebPage::stepIntoStatementInDebugger()
{
#if ENABLE(JAVASCRIPT_DEBUGGER)
    if (d->m_scriptDebugger)
        d->m_scriptDebugger->stepIntoStatementInDebugger();
#endif
}

void WebPage::stepOutOfFunctionInDebugger()
{
#if ENABLE(JAVASCRIPT_DEBUGGER)
    if (d->m_scriptDebugger)
        d->m_scriptDebugger->stepOutOfFunctionInDebugger();
#endif
}

JSContextRef WebPage::scriptContext() const
{
    if (!d->m_mainFrame)
        return 0;
    JSC::Bindings::RootObject *root = d->m_mainFrame->script()->bindingRootObject();
    if (!root)
        return 0;
    JSC::ExecState *exec = root->globalObject()->globalExec();
    return toRef(exec);
}

JSValueRef WebPage::windowObject() const
{
    return toRef(d->m_mainFrame->script()->globalObject(WebCore::mainThreadNormalWorld()));
}

WebDOMDocument WebPage::document() const
{
    if (!d->m_mainFrame)
        return WebDOMDocument();
    return WebDOMDocument(d->m_mainFrame->document());
}

// serialize only the members of HistoryItem which are needed by the client,
// and copy them into a SharedArray.  Also include the HistoryItem pointer which
// will be used by the client as an opaque reference to identify the item
void WebPage::getBackForwardList(SharedArray<BackForwardEntry>& result, unsigned int& resultSize) const
{
    HistoryItemVector entries = d->m_page->backForwardList()->entries();
    resultSize = entries.size();
    result.reset(new BackForwardEntry[resultSize]);

    for (int i = 0; i < resultSize; ++i) {
        RefPtr<HistoryItem> entry = entries[i];
        BackForwardEntry& resultEntry = result[i];
        resultEntry.url = entry->urlString();
        resultEntry.title = entry->title();
        resultEntry.networkToken = entry->networkToken();
        resultEntry.id = backForwardIdFromHistoryItem(entry.get());

        // make sure the HistoryItem is not disposed while the result list is still being used, to make sure the pointer is not reused
        // will be balanced by deref in releaseBackForwardEntry
        entry->ref();
    }
}

void WebPage::releaseBackForwardEntry(BackForwardId id) const
{
    HistoryItem* item = historyItemFromBackForwardId(id);
    ASSERT(item);
    item->deref();
}

ResourceHolder* WebPage::getImageFromContext()
{
    WebCore::Node* node = d->m_currentContextNode.get();
    if (!node)
        return 0;

    // FIXME: We only support HTMLImageElement for now.
    if (node->hasTagName(WebCore::HTMLNames::imgTag)) {
        WebCore::HTMLImageElement* image = static_cast<WebCore::HTMLImageElement*>(node);
        if (CachedResource* cachedResource = image->cachedImage()) {
            if (ResourceHolder* holder = ResourceHolderImpl::create(*cachedResource))
                return holder;
        }
    }

    return 0;
}

void WebPage::addVisitedLink(const unsigned short* url, unsigned int length)
{
    ASSERT(d->m_page);
    d->m_page->group().addVisitedLink(url, length);
}

void WebPage::cancelLoadingPlugin(int id)
{
    FrameLoaderClientOlympia* frameLoaderClient = static_cast<FrameLoaderClientOlympia*>(d->m_mainFrame->loader()->client());
    frameLoaderClient->cancelLoadingPlugin(id);
}

WebDOMNode WebPage::nodeAtPoint(int x, int y)
{
    HitTestResult result = d->m_mainFrame->eventHandler()->hitTestResultAtPoint(d->mapFromTransformed(WebCore::IntPoint(x, y)), false);
    Node* node = result.innerNonSharedNode();
    return WebDOMNode(node);
}

bool WebPage::getNodeRect(const WebDOMNode& node, Platform::IntRect& result)
{
    Node* nodeImpl = node.impl();
    if (nodeImpl && nodeImpl->renderer()) {
        result = nodeImpl->getRect();
        return true;
    }

    return false;
}

bool WebPage::setNodeFocus(const WebDOMNode& node, bool on)
{
    Node* nodeImpl = node.impl();

    if (nodeImpl && nodeImpl->isFocusable()) {
        Document* doc = nodeImpl->document();
        if (Page* page = doc->page()) {
            // modify if focusing on node or turning off focused node
            if (on) {
                page->focusController()->setFocusedNode(nodeImpl, doc->frame());
                if (nodeImpl->isElementNode())
                    static_cast<Element*>(nodeImpl)->updateFocusAppearance(true);
            } else if (doc->focusedNode() == nodeImpl) // && !on
                page->focusController()->setFocusedNode(0, doc->frame());

            return true;
        }
    }
    return false;
}

bool WebPage::setNodeHovered(const WebDOMNode& node, bool on)
{
    Node* nodeImpl = node.impl();
    if (nodeImpl) {
        nodeImpl->setHovered(on);
        return true;
    }
    return false;
}

bool WebPage::nodeHasHover(const WebDOMNode& node)
{
    Node* nodeImpl = node.impl();
    if (nodeImpl && nodeImpl->renderStyle())
        return nodeImpl->renderStyle()->affectedByHoverRules();
    return false;
}

WebCore::String WebPagePrivate::findPatternStringForUrl(const KURL& url) const
{
    if ((m_webSettings->shouldHandlePatternUrls() && protocolIs(url, "pattern"))
            || protocolIs(url, "tel")
            || protocolIs(url, "wtai")
            || protocolIs(url, "cti")
            || protocolIs(url, "mailto")
            || protocolIs(url, "sms")
            || protocolIs(url, "pin")) {
        return url;
    }
    return WebCore::String();
}

bool WebPage::defersLoading() const
{
    return d->m_page->defersLoading();
}

bool WebPage::willFireTimer()
{
    if (d->isLoading())
        return true;

    // Layout can be expensive. Leave it to the layout timer.
    if (d->m_mainFrame->view()->needsLayout())
        return true;

    return d->m_backingStore->d->willFireTimer();
}

static void clearStorageFromFrame(Frame* frame) {
    ASSERT(frame);

    if (Storage* storage = frame->domWindow()->optionalSessionStorage()) {
        storage->clear();
        frame->domWindow()->removeSessionStorage();
    }
    if (Storage* storage = frame->domWindow()->optionalLocalStorage()) {
        storage->clear();
        frame->domWindow()->removeLocalStorage();
    }
}

static void clearStorageFromFrameRecursively(Frame* frame)
{
    ASSERT(frame);

    for (Frame* child = frame->tree()->firstChild(); child; child = child->tree()->nextSibling())
        clearStorageFromFrameRecursively(child);

    clearStorageFromFrame(frame);
}

void WebPage::clearStorage()
{
    clearStorageFromFrameRecursively(d->m_page->mainFrame());
    d->m_page->removeSessionStorage();
}

}
}
