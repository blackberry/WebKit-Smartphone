/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef WebPage_p_h
#define WebPage_p_h

#include "FloatPoint.h"
#include "IntPoint.h"
#include "IntSize.h"
#include "OlympiaAnimation.h"
#include "OlympiaContext.h"
#include "OlympiaPlatformInputEvents.h"
#include "PageClientOlympia.h"
#include "Timer.h"
#include "ViewportArguments.h"
#include "VisiblePosition.h"
#include "WebPage.h"

#include <openvg.h>
#include <wtf/HashMap.h>
#include <wtf/OwnPtr.h>
#include <wtf/Vector.h>

#define DEFAULT_MAX_LAYOUT_WIDTH 1024
#define DEFAULT_MAX_LAYOUT_HEIGHT 768

namespace WebCore {
    class Cursor;
    class Frame;
    class FrameView;
    class HTMLInputElement;
    class HTMLSelectElement;
    class IntPoint;
    class IntRect;
    class KURL;
    class Node;
    class Page;
    class PlatformMouseEvent;
    class RenderObject;
    class String;
    class SurfaceOpenVG;
    class TransformationMatrix;
    class JavaScriptDebuggerOlympia;
    template <typename T> class Timer;
}

namespace Olympia {
    namespace WebKit {

        class BackingStore;
        class BackingStoreTile;
        class DumpRenderTree;
        class InputHandler;
        class SelectionHandler;
        class TouchEventHandler;
        class WebPage;
        class WebPageClient;
        class WebSettings;

        // In the client code, there is screen size and viewport.
        // In WebPagePrivate, the screen size is called the transformedViewportSize,
        //                    the viewport position is called the transformedScrollPosition,
        //                and the viewport size is called the transformedActualVisibleSize.
        class WebPagePrivate : public PageClientOlympia {
        public:
            enum ViewMode { Mobile, Desktop, FixedDesktop };
            enum LoadState { None /*on instantiation of page*/, Provisional, Committed, Finished, Failed };
            enum DateTimePopupType { Date, DateTime, DateTimeLocal, Month, Time, Week };
            enum PopupItem { TypeSeparator, TypeGroup, TypeOption };

            WebPagePrivate(WebPage*, WebPageClient*, const Platform::IntRect&);
            virtual ~WebPagePrivate();

            void init();
            WebCore::PlatformMouseEvent transformMouseEvent(WebCore::PlatformMouseEvent eventIn);
            void handleMouseEvent(WebCore::PlatformMouseEvent& mouseEvent);

            void load(const char* url, const char* networkToken, const char* method, Platform::NetworkRequest::CachePolicy cachePolicy, const char* data, size_t dataLength, const char* const* headers, size_t headersLength, bool isInitial);
            void stopCurrentLoad();

            LoadState loadState() const { return m_loadState; }
            bool isLoading() const { return m_loadState == WebPagePrivate::Provisional || m_loadState == WebPagePrivate::Committed; }

            /* Called from within WebKit via FrameLoaderClientOlympia */
            void setLoadState(LoadState state);

            /* Determine if we should zoom, clamping the scale parameter if required */
            bool shouldZoomAboutPoint(double scale, const WebCore::FloatPoint& anchor, bool enforeScaleClamping, double* clampedScale);

            /* Scale the page to the given scale and anchor about the point which is specified in untransformed content coordinates; */
            bool zoomAboutPoint(double scale, const WebCore::FloatPoint& anchor, bool enforceScaleClamping = true, bool forceRendering = false);
            bool scheduleZoomAboutPoint(double scale, const WebCore::FloatPoint& anchor, bool enforceScaleClamping = true, bool forceRendering = false);

            // Perform actual zoom for block zoom.
            void zoomBlock();

            /* Called by the backingstore as well as the method below */
            void requestLayoutIfNeeded() const;
            void setNeedsLayout();

            /* This takes transformed contents coordinates */
            void renderContents(WebCore::SurfaceOpenVG* surface, BackingStoreTile* tile,
                                const WebCore::IntPoint& surfaceOffset, const WebCore::IntRect& contentsRect) const;

            /* These takes transformed viewport coordinates */
            void blitToCanvas(WebCore::SurfaceOpenVG* surface, const WebCore::IntRect& dst, const WebCore::IntRect& src);
            void blitToCanvas(const unsigned char* image, int imageStride, const WebCore::IntRect& dst, const WebCore::IntRect& src);
            void blitFromBufferToBuffer(unsigned char* dst, const int dstStride, const WebCore::IntRect& dstRect,
                                        const unsigned char* src, const int srcStride, const WebCore::IntRect& srcRect);
            void blendOntoCanvas(const unsigned char* image, int imageStride,
                                 const unsigned char* alphaImage, int alphaImageStride, char globalAlpha,
                                 const WebCore::IntRect& dst, const WebCore::IntRect& src);
            void invalidateWindow(const WebCore::IntRect& dst);

            WebCore::IntPoint scrollPosition() const;
            WebCore::IntPoint maximumScrollPosition() const;
            void setScrollPosition(const WebCore::IntPoint&);
            void scrollBy(int deltaX, int deltaY);

            /* Called from within WebKit via ChromeClientOlympia */
            void checkOriginOfCurrentScrollOperation();

            /* The actual visible size as reported by the client, but in webkit coordinates*/
            WebCore::IntSize actualVisibleSize() const;

            /* The viewport size is the same as the client's canvas size, but in webkit coordinates*/
            WebCore::IntSize viewportSize() const;

            /* Modifies the zoomToFit algorithm logic to construct a scale such that the viewportSize above is equal to this size */
            bool hasVirtualViewport() const;
            bool isUserScalable() const { return m_userScalable; }
            void setUserScalable(bool userScalable) { m_userScalable = userScalable; }

            /* Sets default layout size without doing layout or marking as needing layout */
            void setDefaultLayoutSize(const WebCore::IntSize& size);

            /* Updates WebCore when the viewportSize() or actualVisibleSize() change */
            void updateViewportSize();

            WebCore::FloatPoint centerOfVisibleContentsRect() const;
            WebCore::IntRect visibleContentsRect() const;
            WebCore::IntSize contentsSize() const;
            WebCore::IntSize absoluteVisibleOverflowSize() const;

            /* Virtual functions inherited from PageClientOlympia */
            virtual void setCursor(WebCore::PlatformCursorHandle handle);
            virtual Olympia::Platform::NetworkStreamFactory* networkStreamFactory();
            virtual Olympia::Platform::HttpStreamDebugger* httpStreamDebugger();
            virtual bool runMessageLoopForJavaScript();

            /* Called from within WebKit via ChromeClientOlympia */
            void contentsSizeChanged(const WebCore::IntSize&);
            void overflowExceedsContentsSize();

            /* Called from within WebKit via ChromeClientOlympia */
            void layoutFinished();

            /* Called according to our heuristic or from setLoadState depending on whether we have a virtual viewport */
            void zoomToInitialScaleOnLoad();

            /* Various scale factors */
            double currentScale() const;
            double zoomToFitScale() const;
            double initialScale() const;
            void setInitialScale(double scale) { m_initialScale = scale; }
            double minimumScale() const;
            void setMinimumScale(double scale) { m_minimumScale = scale; }
            double maximumScale() const;
            void setMaximumScale(double scale) { m_maximumScale = scale; }
            void resetScales();

            int reflowWidth() const
            {
                // Note: to make this reflow width transform invariant just use
                // transformedActualVisibleSize() here instead!
                return actualVisibleSize().width();
            }

            /* These methods give the real geometry of the device given the currently set transform */
            WebCore::IntPoint transformedScrollPosition() const;
            WebCore::IntPoint transformedMaximumScrollPosition() const;
            WebCore::IntSize transformedActualVisibleSize() const;
            WebCore::IntSize transformedViewportSize() const;
            WebCore::IntRect transformedVisibleContentsRect() const;
            WebCore::IntSize transformedContentsSize() const;

            /* Generic conversions of points, rects, relative to and from contents and viewport*/
            WebCore::IntPoint mapFromContentsToViewport(const WebCore::IntPoint&) const;
            WebCore::IntPoint mapFromViewportToContents(const WebCore::IntPoint&) const;
            WebCore::IntRect mapFromContentsToViewport(const WebCore::IntRect&) const;
            WebCore::IntRect mapFromViewportToContents(const WebCore::IntRect&) const;

            /* Generic conversions of points, rects, relative to and from transformed contents and transformed viewport*/
            WebCore::IntPoint mapFromTransformedContentsToTransformedViewport(const WebCore::IntPoint&) const;
            WebCore::IntPoint mapFromTransformedViewportToTransformedContents(const WebCore::IntPoint&) const;
            WebCore::IntRect mapFromTransformedContentsToTransformedViewport(const WebCore::IntRect&) const;
            WebCore::IntRect mapFromTransformedViewportToTransformedContents(const WebCore::IntRect&) const;

            /* Generic conversions of points, rects, and sizes to and from transformed coordinates*/
            WebCore::IntPoint mapToTransformed(const WebCore::IntPoint&) const;
            WebCore::FloatPoint mapToTransformedFloatPoint(const WebCore::FloatPoint&) const;
            WebCore::IntPoint mapFromTransformed(const WebCore::IntPoint&) const;
            WebCore::FloatPoint mapFromTransformedFloatPoint(const WebCore::FloatPoint&) const;
            WebCore::IntSize mapToTransformed(const WebCore::IntSize&) const;
            WebCore::IntSize mapFromTransformed(const WebCore::IntSize&) const;
            WebCore::IntRect mapToTransformed(const WebCore::IntRect&) const;
            void clipToTransformedContentsRect(WebCore::IntRect& rect) const;
            WebCore::IntRect mapFromTransformed(const WebCore::IntRect&) const;
            bool transformedPointEqualsUntransformedPoint(const WebCore::IntPoint& transformedPoint, const WebCore::IntPoint& untransformedPoint);

            /* Notification methods that deliver changes to the real geometry of the device as specified above */
            void notifyTransformChanged();
            void notifyTransformedContentsSizeChanged();
            void notifyTransformedScrollChanged();

            bool moveToNextField(Olympia::Platform::ScrollDirection dir, int desiredScrollAmount);
            bool moveToNextField(WebCore::Frame* frame, Olympia::Platform::ScrollDirection dir, int desiredScrollAmount);
            Olympia::Platform::IntRect focusNodeRect();

            bool focusField(bool focus);

            WebCore::Node* bestNodeForZoomUnderPoint(const WebCore::IntPoint& point);
            WebCore::Node* bestChildNodeForClickRect(WebCore::Node* parentNode, const WebCore::IntRect& clickRect);
            WebCore::Node* nodeForZoomUnderPoint(const WebCore::IntPoint& point);
            WebCore::Node* adjustedBlockZoomNodeForZoomLimits(WebCore::Node* node);
            WebCore::IntRect rectForNode(WebCore::Node* node);
            WebCore::IntRect blockZoomRectForNode(WebCore::Node* node);
            WebCore::IntRect adjustRectOffsetForFrameOffset(const WebCore::IntRect& rect, const WebCore::Node* node);
            bool compareNodesForBlockZoom(WebCore::Node* n1, WebCore::Node* n2);
            double newScaleForBlockZoomRect(const WebCore::IntRect& rect, double oldScale, double margin);
            double maxBlockZoomScale() const;

            /* Zooming and animations */
            void renderBitmapZoomFrame(double factor, const WebCore::FloatPoint& anchor, WebCore::SurfaceOpenVG* tempSurface = 0);
            void animateZoomBounceBack(const WebCore::FloatPoint& lastBouncePoint);
            void animationZoomBounceBackFrameChanged(Olympia::WebKit::OlympiaAnimation<WebPagePrivate>*, double currentValue, double previousValue);
            void animateBlockZoom();
            void animationBlockZoomFrameChanged(Olympia::WebKit::OlympiaAnimation<WebPagePrivate>*, double currentValue, double previousValue);

            /* PopupMenu Methods */
            bool didNodeOpenPopup(WebCore::Node*);
            bool openLineInputPopup(WebCore::HTMLInputElement*);
            bool openSelectPopup(WebCore::HTMLSelectElement*);
            void setPopupListIndex(int index);
            void setPopupListIndexes(int size, bool* selecteds);
            void setDateTimeInput(const WebCore::String& value);

            /* Plugin Methods */
            void notifyPluginRectChanged(int id, const WebCore::IntRect& rectChanged);

            /* Context Methods */
            Context getContext();
            WebCore::Node* getContextNode() const;
            Context getContextForNode(WebCore::Node* node);
            void sendContextIfChanged(WebCore::Node* node);

#if ENABLE(VIEWPORT_REFLOW)
            void toggleTextReflowIfTextReflowEnabledOnlyForBlockZoom(bool shouldEnableTextReflow = false);
#endif

            void updateCursor();

            ViewMode viewMode() const { return m_viewMode; }
            bool setViewMode(ViewMode); // returns true if the change requires re-layout

            void setShouldUseFixedDesktopMode(bool b) { m_shouldUseFixedDesktopMode = b; }

            bool useFixedLayout() const;
            WebCore::IntSize fixedLayoutSize(bool snapToIncrement = false) const;

            bool didLayoutExceedMaximumIterations() const
            {
                // ZoomToFitOnLoad can lead to a large recursion depth in FrameView::layout() as we attempt
                // to determine the zoom scale factor so as to have the content of the page fit within the
                // area of the frame. From observation, we can bail out after a recursion depth of 10 and
                // still have reasonable results.
                return m_nestedLayoutFinishedCount > 10;
            }

            WebCore::String renderTreeDump() const;

            void clearFocusNode();
            WebCore::Frame* focusedOrMainFrame();
            WebCore::Frame* mainFrame();

            bool scrollNodeRecursively(WebCore::Node* originalNode, const WebCore::IntSize& delta);
            bool scrollRenderer(WebCore::RenderObject* renderer, const WebCore::IntSize& delta);
            void adjustScrollDelta(const WebCore::IntPoint& maxOffset, const WebCore::IntPoint& currentOffset, WebCore::IntSize& delta);

            void didReceiveViewportArguments(const WebCore::ViewportArguments&);
            WebCore::IntSize recomputeVirtualViewportFromViewportArguments();

            void resetBlockZoom();

            void zoomAboutPointTimerFired(WebCore::Timer<WebPagePrivate>*);
            void scrollEventTimerFired(WebCore::Timer<WebPagePrivate>*);
            void resizeEventTimerFired(WebCore::Timer<WebPagePrivate>*);

            // returns true if this url should be handled as a pattern, and puts the pattern in the "pattern" param
            // the pattern can be empty even if this returns true (for example, the url "pattern:")
            bool findPatternStringForUrl(const WebCore::KURL&, WebCore::String& pattern) const;

            // Scroll and/or zoom so that the webpage fits the new screen size and actual visible size.
            // This is only needed if we get a major change like screen rotation or out-of-order viewport events.
            void setScreenRotated(const WebCore::IntSize& screenSize, const WebCore::IntSize& defaultLayoutSize, const WebCore::IntSize& transformedActualVisibleSize);

            void scheduleDeferrableTimer(WebCore::Timer<WebPagePrivate>*, double timeOut);
            void willDeferLoading();
            void didResumeLoading();

            // Returns true if the escape key handler should zoom
            bool shouldZoomOnEscape() const;

            WebPage* m_webPage;
            WebPageClient* m_client;
            WebCore::Page* m_page;
            WebCore::Frame* m_mainFrame;
            WebCore::IntPoint m_lastMousePoint;
            RefPtr<WebCore::Node> m_currentContextNode;

            WebSettings* m_webSettings;

#if ENABLE(JAVASCRIPT_DEBUGGER)
            OwnPtr<WebCore::JavaScriptDebuggerOlympia> m_scriptDebugger;
#endif

            bool m_visible;
            bool m_shouldResetTilesWhenShown;
            static WebPage* g_lastVisibleWebPage;
            bool m_userScalable;
            bool m_userPerformedManualZoom;
            bool m_userPerformedManualScroll;
            bool m_contentsSizeChanged;
            bool m_overflowExceedsContentsSize;
            bool m_olympiaOriginatedScroll;
            bool m_resetVirtualViewportOnCommitted;
            bool m_shouldUseFixedDesktopMode;
            unsigned int m_nestedLayoutFinishedCount;
            WebCore::IntSize m_previousContentsSize;
            int m_x;
            int m_y;
            int m_width;
            int m_height;
            int m_actualVisibleWidth;
            int m_actualVisibleHeight;
            int m_virtualViewportWidth;
            int m_virtualViewportHeight;
            WebCore::IntSize m_defaultLayoutSize;
            WebCore::ViewportArguments m_viewportArguments; // We keep this around since we may need to re-evaluate the arguments on rotation.
            ViewMode m_viewMode;
            LoadState m_loadState;
            WebCore::TransformationMatrix* m_transformationMatrix;
            BackingStore* m_backingStore;
            InputHandler* m_inputHandler;
            SelectionHandler* m_selectionHandler;
            TouchEventHandler* m_touchEventHandler;
            WebCore::HTMLInputElement* m_dateTimeInput;
            WebCore::HTMLSelectElement* m_selectElement;

            Olympia::Platform::OlympiaCursor m_currentCursor;

            DumpRenderTree* m_dumpRenderTree;

            WebCore::FloatPoint m_lastBounceBackFocalPoint;
            WebCore::FloatPoint m_bounceBackFinalPoint;

            // Animation timelines
            Olympia::WebKit::OlympiaAnimation<WebPagePrivate>* m_zoomBounceBackAnimation;
            Olympia::WebKit::OlympiaAnimation<WebPagePrivate>* m_blockZoomAnimation;

            double m_bitmapScale;
            double m_initialScale;
            double m_minimumScale;
            double m_maximumScale;

            // Block zoom animation data.
            WebCore::FloatPoint m_finalBlockPoint;
            WebCore::FloatPoint m_finalBlockPointReflowOffset;
            double m_blockZoomInitialScale;
            double m_blockZoomFinalScale;
            RefPtr<WebCore::Node> m_currentPinchZoomNode;
            WebCore::VisiblePosition m_currentPinchZoomCaretPosition;
            RefPtr<WebCore::Node> m_currentBlockZoomNode;
            RefPtr<WebCore::Node> m_currentBlockZoomAdjustedNode;
            bool m_shouldReflowBlock;

            WebCore::SurfaceOpenVG* m_bitmapZoomBuffer;

            // Delayed zoomAboutPoint
            OwnPtr<WebCore::Timer<WebPagePrivate> > m_delayedZoomTimer;
            struct {
                double scale;
                WebCore::FloatPoint anchor;
                bool enforceScaleClamping;
                bool forceRendering;
            } m_delayedZoomArguments;

            double m_lastUserEventTimestamp; // Used to detect user scrolling

            OwnPtr<WebCore::Timer<WebPagePrivate> > m_deferredScrollEventTimer;
            OwnPtr<WebCore::Timer<WebPagePrivate> > m_deferredResizeEventTimer;
            Vector<WebCore::Timer<WebPagePrivate>* > m_deferrableTimers;
            Vector<WebCore::Timer<WebPagePrivate>* > m_deferredTimers;
        };
    }
}

#endif // WebPage_p_h
