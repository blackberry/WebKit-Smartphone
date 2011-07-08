/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef BackingStore_p_h
#define BackingStore_p_h

#include "BackingStoreScrollbar.h"
#include "BackingStoreTile.h"
#include "IntPoint.h"
#include "IntSize.h"
#include "RenderQueue.h"
#include "TileIndex.h"
#include "TileIndexHash.h"
#include "Timer.h"

#include <egl.h>
#include <wtf/HashMap.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

namespace WebCore {
    class IntRect;
    typedef WTF::Vector<IntRect> IntRectList;
    class TransformationMatrix;
}

namespace Olympia {
    namespace WebKit {

        class WebPage;

        class BackingStorePrivate {
        public:
            enum TileMatrixDirection { Horizontal, Vertical };
            enum ResumeUpdateOperation { None, Render, Blit, RenderAndBlit };
            BackingStorePrivate();
            ~BackingStorePrivate();

            // Suspends all screen updates so that 'blitToCanvas' is disabled.
            void suspendScreenUpdates();

            // Resumes all screen updates so that 'blitToCanvas' is enabled.
            void resumeScreenUpdates(ResumeUpdateOperation op);

            /* Called from outside WebKit and within WebKit via ChromeClientOlympia */
            void repaint(const WebCore::IntRect& windowRect,
                         bool contentChanged, bool immediate, bool repaintContentOnly);

            /* Called from within WebKit via ChromeClientOlympia */
            void slowScroll(const WebCore::IntSize& delta, const WebCore::IntRect& windowRect, bool immediate);

            /* Called from within WebKit via ChromeClientOlympia */
            void scroll(const WebCore::IntSize& delta,
                        const WebCore::IntRect& scrollViewRect,
                        const WebCore::IntRect& clipRect);
            void scrollingStartedHelper(const WebCore::IntSize& delta);

            bool shouldPerformRenderJobs() const;
            bool shouldPerformRegularRenderJobs() const;
            void startRenderTimer();
            void stopRenderTimer();
            void renderOnTimer(WebCore::Timer<BackingStorePrivate>*);
            void renderOnIdle();
            bool willFireTimer();

            /* Hides the scrollbars */
            void autoHideScrollbars(WebCore::Timer<BackingStorePrivate>*);

            /* Resumes the render queue */
            void resumeRenderQueue(WebCore::Timer<BackingStorePrivate>*);

            /* Set of helper methods for the scrollBackingStore() method */
            WebCore::IntRect contentsRect() const;
            WebCore::IntRect expandedContentsRect() const;
            WebCore::IntRect visibleContentsRect() const;
            WebCore::IntRect backingStoreRect() const;
            bool shouldMoveLeft(const WebCore::IntRect&) const;
            bool shouldMoveRight(const WebCore::IntRect&) const;
            bool shouldMoveUp(const WebCore::IntRect&) const;
            bool shouldMoveDown(const WebCore::IntRect&) const;
            bool canMoveX(const WebCore::IntRect&) const;
            bool canMoveY(const WebCore::IntRect&) const;
            bool canMoveLeft(const WebCore::IntRect&) const;
            bool canMoveRight(const WebCore::IntRect&) const;
            bool canMoveUp(const WebCore::IntRect&) const;
            bool canMoveDown(const WebCore::IntRect&) const;

            WebCore::IntRect backingStoreRectForScroll(int deltaX, int deltaY, const WebCore::IntRect&) const;
            void setBackingStoreRect(const WebCore::IntRect&);

            typedef WTF::Vector<TileIndex> TileIndexList;
            TileIndexList indexesForBackingStoreRect(const WebCore::IntRect&) const;

            WebCore::IntPoint originOfLastRenderForTile(const TileIndex& index,
                                                        BackingStoreTile* tile,
                                                        const WebCore::IntRect& backingStoreRect) const;

            TileIndex indexOfLastRenderForTile(const TileIndex& index, BackingStoreTile* tile) const;
            TileIndex indexOfTile(const WebCore::IntPoint& origin,
                                  const WebCore::IntRect& backingStoreRect) const;
            void clearAndUpdateTileOfNotRenderedRegion(const TileIndex& index, BackingStoreTile* tile,
                                                       const WebCore::IntRectRegion& tileNotRenderedRegion,
                                                       const WebCore::IntRect& backingStoreRect,
                                                       bool update = true);
            bool isCurrentVisibleJob(const TileIndex& index, BackingStoreTile* tile,
                                     const WebCore::IntRect& backingStoreRect) const;

            /* Responsible for scrolling the backingstore and updating the tile matrix geometry */
            void scrollBackingStore(int deltaX, int deltaY);

            /* Render the tiles dirty rects and blit to the screen */
            void render(const WebCore::IntRect& rect, bool renderContentOnly);
            void render(const WebCore::IntRectList& rectList, bool renderContentOnly);

            /* Called by the render queue to ensure that the queue is in a constant state before performing a render job. */
            void requestLayoutIfNeeded() const;

            /* Helper render methods */
            void renderVisibleContents(bool renderContentOnly);
            void renderBackingStore(bool renderContentOnly);
            WebCore::IntRect blitVisibleContents();
            typedef std::pair<TileIndex, WebCore::IntRect> TileRect;
            WebCore::IntRect blitTileRect(const TileRect&);
            void invalidateWindow(const WebCore::IntRect&);
            bool scrollsHorizontally() const;
            bool scrollsVertically() const;
            float horizontalScrollbarExtent() const;
            float verticalScrollbarExtent() const;
            void blitHorizontalScrollbar();
            void blitVerticalScrollbar();
            void blitScaledArbitraryRectFromBackingStoreToScreen(const WebCore::IntRect& srcRect, WebCore::IntRect dstRect, unsigned short* tempSurface, unsigned int tempSurfaceStride);

            /* Returns whether the tile index is currently visible or not */
            bool isTileVisible(const TileIndex&) const;
            bool isTileVisible(const WebCore::IntPoint&) const;

            /* Returns a rect that is the union of all tiles that are visible */
            WebCore::IntRect visibleTilesRect() const;

            /* Used to clip to the visible content for instance */
            WebCore::IntRect tileVisibleContentsRect(const TileIndex&) const;

            /* This is called by WebPage once load is committed to clear the render queue */
            void clearRenderQueue();
            /* This is called by FrameLoaderClient that explicitly paints on first visible layout */
            void clearVisibleZoom();

            /* This is called by WebPage once load is committed to reset all the tiles */
            void resetTiles(bool resetBackground);

            /* This is called by WebPage after load is complete to update all the tiles */
            void updateTiles(bool updateVisible, bool immediate);

            /* This is called during scroll and by the render queue */
            void updateTilesForScrollOrNotRenderedRegion();

            /* Reset an individual tile */
            void resetTile(const TileIndex&, BackingStoreTile*, bool resetBackground);

            /* Update an individual tile */
            void updateTile(const TileIndex&, bool immediate);
            void updateTile(const WebCore::IntPoint&, bool immediate);

            WebCore::IntRect mapFromTilesToTransformedContents(const TileRect&) const;

            typedef WTF::Vector<TileRect> TileRectList;
            TileRectList mapFromTransformedContentsToTiles(const WebCore::IntRect&) const;

            void updateTileMatrixIfNeeded();

            /* Called by WebPagePrivate::notifyTransformedContentsSizeChanged */
            void contentsSizeChanged(const WebCore::IntSize&);

            /* Called by WebPagePrivate::notifyTransformedScrollChanged */
            void scrollChanged(const WebCore::IntPoint&);

            /* Called by WebpagePrivate::notifyTransformChanged */
            void transformChanged();

            /* Called by WebpagePrivate::actualVisibleSizeChanged */
            void actualVisibleSizeChanged(const WebCore::IntSize&);

            /* Called by WebPagePrivate::setScreenRotated */
            void orientationChanged();

            /* Sets the geometry of the tile matrix */
            void setGeometryOfTileMatrix(int numberOfTilesWide, int numberOfTilesHigh);

            /* Create the surfaces of the backingstore */
            void createSurfaces();
            void createVisibleTileBuffer();
            void createScrollbars();

            /* Various calculations of quantities relevant to backingstore */
            WebCore::IntPoint originOfTile(const TileIndex&) const;
            WebCore::IntPoint originOfTile(const TileIndex&, const WebCore::IntRect&) const;
            int numberOfTilesWide() const;
            int numberOfTilesHigh() const;
            int minimumNumberOfTilesWide() const;
            int minimumNumberOfTilesHigh() const;
            int tileWidth() const;
            int tileHeight() const;
            WebCore::IntSize tileSize() const;
            WebCore::IntRect tileRect() const;
            WebCore::IntSize expandedContentsSize() const;
            WebCore::IntSize backingStoreSize() const;

            bool m_suspendScreenUpdates;
            bool m_suspendRenderJobs;
            bool m_suspendRegularRenderJobs;
            WebPage* m_webPage;
            RenderQueue* m_renderQueue;

            EGLDisplay m_eglDisplay;

            int m_numberOfTilesWide;
            int m_numberOfTilesHigh;
            TileMatrixDirection m_preferredTileMatrixDimension;

            WebCore::IntRect m_visibleTileBufferRect;

            // Hides any scrollbars when this times out
            WebCore::Timer<BackingStorePrivate>* m_autoHideScrollbarsTimer;

            // Resumes the render queue when this times out
            WebCore::Timer<BackingStorePrivate>* m_resumeRenderQueueTimer;

            // Last resort timer for rendering
            WebCore::Timer<BackingStorePrivate>* m_renderTimer;

            WebCore::IntPoint m_backingStoreOffset;
            typedef WTF::HashMap<TileIndex, WTF::RefPtr<BackingStoreTile> > TileMap;
            TileMap m_tileMap;
        };
    }
}

#endif // BackingStore_p_h
