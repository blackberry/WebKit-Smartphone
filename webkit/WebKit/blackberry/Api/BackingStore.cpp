/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "BackingStore.h"
#include "BackingStore_p.h"

#include "CString.h"
#include "EGLDisplayOpenVG.h"
#include "EGLUtils.h"
#include "FloatRect.h"
#include "IntRect.h"
#include "IntRectRegion.h"
#include "IntSize.h"
#include "PainterOpenVG.h"
#include "PlatformString.h"
#include "SurfaceOpenVG.h"
#include "SurfacePool.h"
#include "TiledImageOpenVG.h"
#include "TransformationMatrix.h"
#include "WebPage.h"
#include "WebPage_p.h"
#include "WebPageClient.h"
#include "WebSettings.h"

#include <math.h>
#include <wtf/CurrentTime.h>
#include <wtf/ListHashSet.h>
#include <wtf/MathExtras.h>
#include <wtf/NotFound.h>

#define ENABLE_SCROLLBARS 1
#define ENABLE_REPAINTONSCROLL 1
#define DEBUG_BACKINGSTORE 0
#define DEBUG_TILEMATRIX 0

using std::min;
using std::max;
using namespace WebCore;

namespace Olympia {
namespace WebKit {

/* Compute divisor pairs */
typedef std::pair<int, int> Divisor;
typedef Vector<Divisor> DivisorList;
// TODO: cache this and/or use a smarter algorithm
static DivisorList divisors(int n)
{
    DivisorList divisors;
    for (unsigned int i = 1; i <= n; ++i)
        if (!(n % i))
            divisors.append(std::make_pair(i, n / i));
    return divisors;
}

/* Compute best divisor given the ratio determined by size */
static Divisor bestDivisor(WebCore::IntSize size, int tileWidth, int tileHeight,
                           int minimumNumberOfTilesWide, int minimumNumberOfTilesHigh,
                           BackingStorePrivate::TileMatrixDirection direction)
{
    // The point of this function is to determine the number of tiles in each
    // dimension.  We do this by looking to match the tile matrix width/height
    // ratio as closely as possible with the width/height ratio of the contents.
    // We also look at the direction passed to give preference to one dimension
    // over another. This method could probably be made faster, but it gets the
    // job done.

    SurfacePool* surfacePool = SurfacePool::globalSurfacePool();
    ASSERT(!surfacePool->isEmpty());

    // Store a static list of possible divisors
    static DivisorList divisorList = divisors(surfacePool->size());

    // The ratio we're looking to best imitate
    float ratio = static_cast<float>(size.width()) / static_cast<float>(size.height());

    Divisor bestDivisor;
    for (size_t i = 0; i < divisorList.size(); ++i) {
        Divisor divisor = divisorList[i];

        bool divisorWidthIsPerfect = size.width() <= divisor.first * tileWidth && abs(size.width() - divisor.first * tileWidth) < tileWidth;
        bool divisorHeightIsPerfect = size.height() <= divisor.second * tileHeight && abs(size.height() - divisor.second * tileHeight) < tileHeight;
        bool divisorWidthIsValid = divisor.first >= minimumNumberOfTilesWide || divisorWidthIsPerfect;
        bool divisorHeightIsValid = divisor.second >= minimumNumberOfTilesHigh || divisorHeightIsPerfect;
        if (!divisorWidthIsValid || !divisorHeightIsValid)
            continue;

        if (divisor.first > divisor.second && direction == BackingStorePrivate::Vertical && !divisorHeightIsPerfect)
            continue;

        if (divisor.second > divisor.first && direction == BackingStorePrivate::Horizontal && !divisorWidthIsPerfect)
            continue;

        if (divisorWidthIsPerfect || divisorHeightIsPerfect) {
            bestDivisor = divisor; // Found a perfect fit!
#if DEBUG_TILEMATRIX
            Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "bestDivisor found perfect size widthPerfect=%s heightPerfect=%s",
                                   divisorWidthIsPerfect ? "true" : "false",
                                   divisorHeightIsPerfect ? "true" : "false");
#endif
            break;
        }

        // Store basis of comparison
        if (!bestDivisor.first && !bestDivisor.second) {
            bestDivisor = divisor;
            continue;
        }

        // Compare ratios
        float diff1 = fabs((static_cast<float>(divisor.first) / static_cast<float>(divisor.second)) - ratio);
        float diff2 = fabs((static_cast<float>(bestDivisor.first) / static_cast<float>(bestDivisor.second)) - ratio);
        if (diff1 < diff2)
            bestDivisor = divisor;
    }

    return bestDivisor;
}

BackingStorePrivate::BackingStorePrivate()
    : m_suspendScreenUpdates(false)
    , m_suspendRenderJobs(false)
    , m_suspendRegularRenderJobs(false)
    , m_webPage(0)
    , m_renderQueue(new RenderQueue(this))
    , m_eglDisplay(EGL_NO_DISPLAY)
    , m_numberOfTilesWide(0)
    , m_numberOfTilesHigh(0)
    , m_preferredTileMatrixDimension(Vertical)
{
#if ENABLE_SCROLLBARS
    m_autoHideScrollbarsTimer = new Timer<BackingStorePrivate>(this, &BackingStorePrivate::autoHideScrollbars);
#endif
    m_resumeRenderQueueTimer = new Timer<BackingStorePrivate>(this, &BackingStorePrivate::resumeRenderQueue);
    m_renderTimer = new Timer<BackingStorePrivate>(this, &BackingStorePrivate::renderOnTimer);
}

BackingStorePrivate::~BackingStorePrivate()
{
    delete m_renderQueue;
    m_renderQueue = 0;
#if ENABLE_SCROLLBARS
    delete m_autoHideScrollbarsTimer;
    m_autoHideScrollbarsTimer = 0;
#endif
    delete m_resumeRenderQueueTimer;
    m_resumeRenderQueueTimer = 0;
    delete m_renderTimer;
    m_renderTimer = 0;
}

void BackingStorePrivate::suspendScreenUpdates()
{
    m_suspendScreenUpdates = true;
}

void BackingStorePrivate::resumeScreenUpdates(ResumeUpdateOperation op)
{
    m_suspendScreenUpdates = false;
    switch (op) {
    case Render:
        renderVisibleContents(true /*renderContentOnly*/);
        break;
    case Blit:
        {
            IntRect updateRect = blitVisibleContents();
            invalidateWindow(updateRect);
            break;
        }
    case RenderAndBlit:
        renderVisibleContents(false /*renderContentOnly*/);
        break;
    case None:
    default:
        break;
    }
}

void BackingStorePrivate::repaint(const WebCore::IntRect& windowRect,
                                  bool contentChanged, bool immediate, bool repaintContentOnly)
{
    ASSERT(m_eglDisplay != EGL_NO_DISPLAY);

    if (m_suspendScreenUpdates)
        return;

    /*
     * If immediate is true, then we're being asked to perform synchronously
     * If repaintContentOnly, then we're being asked to just render to the backingstore, but not to update the screen
     *
     * NOTE: WebCore::ScrollView will call this method with immediate:true and contentChanged:false.
     * This is a special case introduced specifically for the Apple's windows port and can be safely ignored I believe.
     *
     * Now this method will be called from WebPagePrivate::repaint()
     */

    if (contentChanged && !windowRect.isEmpty()) {
        // This windowRect is in untransformed coordinates relative to the viewport, but
        // it needs to be transformed coordinates relative to the transformed contents.
        IntRect rect = m_webPage->d->mapToTransformed(m_webPage->d->mapFromViewportToContents(windowRect));
        rect.inflate(1); // account for anti-aliasing of previous rendering runs
        m_webPage->d->clipToTransformedContentsRect(rect);

        if (rect.isEmpty())
            return;

        if (immediate)
            render(rect, repaintContentOnly);
        else
            m_renderQueue->addToQueue(RenderQueue::RegularRender, rect);
    }
}

void BackingStorePrivate::slowScroll(const WebCore::IntSize& delta, const WebCore::IntRect& windowRect, bool immediate)
{
#if DEBUG_BACKINGSTORE
    // Start the time measurement...
    double time = WTF::currentTime();
#endif

    scrollingStartedHelper(delta);

    // This windowRect is in untransformed coordinates relative to the viewport, but
    // it needs to be transformed coordinates relative to the transformed contents.
    IntRect rect = m_webPage->d->mapToTransformed(m_webPage->d->mapFromViewportToContents(windowRect));
    m_webPage->d->clipToTransformedContentsRect(rect);

    if (immediate)
        render(rect, false /*renderContentOnly*/);
    else {
        m_renderQueue->addToQueue(RenderQueue::VisibleScroll, rect);
        // Blit the current contents to the canvas
        blitVisibleContents();
    }

    // FIXME: Need to invalidate the window if *and only if* the scroll did not
    // originate with a window event!

#if DEBUG_BACKINGSTORE
    // Stop the time measurement
    double elapsed = WTF::currentTime() - time;
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::slowScroll elapsed=%f", elapsed);
#endif
}

void BackingStorePrivate::scroll(const WebCore::IntSize& delta,
                                 const WebCore::IntRect& scrollViewRect,
                                 const WebCore::IntRect& clipRect)
{
#if DEBUG_BACKINGSTORE
    // Start the time measurement...
    double time = WTF::currentTime();
#endif

    scrollingStartedHelper(delta);

    // Blit the current contents to the canvas
    blitVisibleContents();

    // FIXME: Need to invalidate the window if *and only if* the scroll did not
    // originate with a window event!

#if DEBUG_BACKINGSTORE
    // Stop the time measurement
    double elapsed = WTF::currentTime() - time;
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::scroll dx=%d, dy=%d elapsed=%f", delta.width(), delta.height(), elapsed);
#endif
}

void BackingStorePrivate::scrollingStartedHelper(const WebCore::IntSize& delta)
{
    if (!m_suspendScreenUpdates) {
#if ENABLE_SCROLLBARS
        if (m_webPage->settings()->isScrollbarsEnabled()) {
            // Restart the auto hide scrollbars timer
            m_autoHideScrollbarsTimer->stop();
            m_autoHideScrollbarsTimer->startOneShot(0.25); // 250 msecs
        }
#endif

        // Restart the resume render queue timer
        m_resumeRenderQueueTimer->stop();
        m_resumeRenderQueueTimer->startOneShot(0.25); // 250 msecs

#if !ENABLE_REPAINTONSCROLL
        m_suspendRenderJobs = true; // Suspend the rendering of everything
#endif
        if (!m_webPage->settings()->shouldRenderAnimationsOnScroll())
            m_suspendRegularRenderJobs = true; // Suspend the rendering of animations

        // Clear this flag since we don't care if the render queue is under pressure
        // or not since we are scrolling and it is more important to not lag than
        // it is to ensure animations achieve better framerates!
        m_renderQueue->setCurrentRegularRenderJobBatchUnderPressure(false);
    }

    // Notify the render queue so that it can shuffle accordingly
    m_renderQueue->updateSortDirection(delta.width(), delta.height());
    m_renderQueue->visibleContentChanged(visibleContentsRect());

    // Scroll the actual backingstore
    scrollBackingStore(delta.width(), delta.height());

    // Add any newly visible tiles that have not been previously rendered to the queue
    // and check if the tile was previously rendered by regular render job
    updateTilesForScrollOrNotRenderedRegion();
}

bool BackingStorePrivate::shouldPerformRenderJobs() const
{
    return (m_webPage->isVisible() || m_webPage->settings()->isDirectRenderingToCanvasEnabled()) && !m_suspendRenderJobs && !m_suspendScreenUpdates && !m_renderQueue->isEmpty(!m_suspendRegularRenderJobs);
}

bool BackingStorePrivate::shouldPerformRegularRenderJobs() const
{
    return shouldPerformRenderJobs() && !m_suspendRegularRenderJobs;
}

void BackingStorePrivate::startRenderTimer()
{
    // Called when render queue has a new job added
    if (m_renderTimer->isActive() || m_renderQueue->isEmpty(!m_suspendRegularRenderJobs))
        return;
#if DEBUG_BACKINGSTORE
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::startRenderTimer time=%f", WTF::currentTime());
#endif
    m_renderTimer->startOneShot(5.0); // five seconds since this is a catch-all fallback
}

void BackingStorePrivate::stopRenderTimer()
{
    if (!m_renderTimer->isActive())
        return;

    // Called when we render something to restart
#if DEBUG_BACKINGSTORE
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::stopRenderTimer time=%f", WTF::currentTime());
#endif
    m_renderTimer->stop();
}

void BackingStorePrivate::renderOnTimer(WebCore::Timer<BackingStorePrivate>*)
{
    // This timer is a third method of starting a render operation that is a catch-all.  If more
    // than two seconds elapses with no rendering taking place and render jobs in the queue, then
    // renderOnTimer will be called which will actually render.
    if (!shouldPerformRenderJobs())
        return;

#if DEBUG_BACKINGSTORE
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::renderOnTimer time=%f", WTF::currentTime());
#endif
    while (m_renderQueue->hasCurrentVisibleZoomJob() || m_renderQueue->hasCurrentVisibleScrollJob())
        m_renderQueue->render(!m_suspendRegularRenderJobs);
}

void BackingStorePrivate::renderOnIdle()
{
    ASSERT(shouldPerformRenderJobs());

    // Let the render queue know that we entered a new event queue cycle
    // so it can determine if it is under pressure
    m_renderQueue->eventQueueCycled();

#if DEBUG_BACKINGSTORE
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::renderOnIdle");
#endif

    m_renderQueue->render(!m_suspendRegularRenderJobs);
}

bool BackingStorePrivate::willFireTimer()
{
    // Let the render queue know that we entered a new event queue cycle
    // so it can determine if it is under pressure
    m_renderQueue->eventQueueCycled();

    if (!shouldPerformRegularRenderJobs() || !m_renderQueue->hasCurrentRegularRenderJob() || !m_renderQueue->currentRegularRenderJobBatchUnderPressure())
        return true;

#if DEBUG_BACKINGSTORE
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::willFireTimer");
#endif

    // We've detected that the regular render jobs are coming under pressure likely
    // due to timers firing producing invalidation jobs and our efforts to break them
    // up into bite size pieces has produced a situation where we can not complete
    // a batch of them before receiving more that intersect them which causes us
    // to start the batch over.  To mitigate this we have to empty the current batch
    // when this is detected.
    m_renderQueue->renderAllCurrentRegularRenderJobs();

    // Let the caller yield and reschedule the timer.
    return false;
}

void BackingStorePrivate::autoHideScrollbars(WebCore::Timer<BackingStorePrivate>*)
{
    // Blit the current contents to the canvas
    IntRect updateRect = blitVisibleContents();
    invalidateWindow(updateRect);
}

void BackingStorePrivate::resumeRenderQueue(WebCore::Timer<BackingStorePrivate>*)
{
#if !ENABLE_REPAINTONSCROLL
    m_suspendRenderJobs = false;
#endif
    m_suspendRegularRenderJobs = false;
}

WebCore::IntRect BackingStorePrivate::expandedContentsRect() const
{
    return IntRect(IntPoint(0,0), expandedContentsSize());
}

WebCore::IntRect BackingStorePrivate::visibleContentsRect() const
{
    return m_webPage->d->transformedVisibleContentsRect();
}

WebCore::IntRect BackingStorePrivate::backingStoreRect() const
{
    return IntRect(m_backingStoreOffset, backingStoreSize());
}

bool BackingStorePrivate::shouldMoveLeft(const WebCore::IntRect& backingStoreRect) const
{
    return canMoveX(backingStoreRect)
            && backingStoreRect.x() > visibleContentsRect().x()
            && backingStoreRect.x() > expandedContentsRect().x();
}

bool BackingStorePrivate::shouldMoveRight(const WebCore::IntRect& backingStoreRect) const
{
    return canMoveX(backingStoreRect)
            && backingStoreRect.right() < visibleContentsRect().right()
            && backingStoreRect.right() < expandedContentsRect().right();
}

bool BackingStorePrivate::shouldMoveUp(const WebCore::IntRect& backingStoreRect) const
{
    return canMoveY(backingStoreRect)
            && backingStoreRect.y() > visibleContentsRect().y()
            && backingStoreRect.y() > expandedContentsRect().y();
}

bool BackingStorePrivate::shouldMoveDown(const WebCore::IntRect& backingStoreRect) const
{
    return canMoveY(backingStoreRect)
            && backingStoreRect.bottom() < visibleContentsRect().bottom()
            && backingStoreRect.bottom() < expandedContentsRect().bottom();
}

bool BackingStorePrivate::canMoveX(const WebCore::IntRect& backingStoreRect) const
{
    return backingStoreRect.width() > visibleContentsRect().width();
}

bool BackingStorePrivate::canMoveY(const WebCore::IntRect& backingStoreRect) const
{
    return backingStoreRect.height() > visibleContentsRect().height();
}

bool BackingStorePrivate::canMoveLeft(const WebCore::IntRect& rect) const
{
    IntRect backingStoreRect = rect;
    IntRect visibleContentsRect = this->visibleContentsRect();
    IntRect contentsRect = this->expandedContentsRect();
    backingStoreRect.move(-tileWidth(), 0);
    return backingStoreRect.right() >= visibleContentsRect.right()
            && backingStoreRect.x() >= contentsRect.x();
}

bool BackingStorePrivate::canMoveRight(const WebCore::IntRect& rect) const
{
    IntRect backingStoreRect = rect;
    IntRect visibleContentsRect = this->visibleContentsRect();
    IntRect contentsRect = this->expandedContentsRect();
    backingStoreRect.move(tileWidth(), 0);
    return backingStoreRect.x() <= visibleContentsRect.x()
            && (backingStoreRect.right() <= contentsRect.right()
            || (backingStoreRect.right() - contentsRect.right()) < tileWidth());
}

bool BackingStorePrivate::canMoveUp(const WebCore::IntRect& rect) const
{
    IntRect backingStoreRect = rect;
    IntRect visibleContentsRect = this->visibleContentsRect();
    IntRect contentsRect = this->expandedContentsRect();
    backingStoreRect.move(0, -tileHeight());
    return backingStoreRect.bottom() >= visibleContentsRect.bottom()
            && backingStoreRect.y() >= contentsRect.y();
}

bool BackingStorePrivate::canMoveDown(const WebCore::IntRect& rect) const
{
    IntRect backingStoreRect = rect;
    IntRect visibleContentsRect = this->visibleContentsRect();
    IntRect contentsRect = this->expandedContentsRect();
    backingStoreRect.move(0, tileHeight());
    return backingStoreRect.y() <= visibleContentsRect.y()
            && (backingStoreRect.bottom() <= contentsRect.bottom()
            || (backingStoreRect.bottom() - contentsRect.bottom()) < tileHeight());
}

IntRect BackingStorePrivate::backingStoreRectForScroll(int deltaX, int deltaY, const IntRect& rect) const
{
    // The current rect
    IntRect backingStoreRect = rect;

    // Return to origin if need be
    if (!canMoveX(backingStoreRect) && backingStoreRect.x())
        backingStoreRect.setX(0);

    if (!canMoveY(backingStoreRect) && backingStoreRect.y())
        backingStoreRect.setY(0);

    // Move the rect left
    while (shouldMoveLeft(backingStoreRect) || (deltaX > 0 && canMoveLeft(backingStoreRect)))
        backingStoreRect.move(-tileWidth(), 0);

    // Move the rect right
    while (shouldMoveRight(backingStoreRect) || (deltaX < 0 && canMoveRight(backingStoreRect)))
        backingStoreRect.move(tileWidth(), 0);

    // Move the rect up
    while (shouldMoveUp(backingStoreRect) || (deltaY > 0 && canMoveUp(backingStoreRect)))
        backingStoreRect.move(0, -tileHeight());

    // Move the rect down
    while (shouldMoveDown(backingStoreRect) || (deltaY < 0 && canMoveDown(backingStoreRect)))
        backingStoreRect.move(0, tileHeight());

    return backingStoreRect;
}

void BackingStorePrivate::setBackingStoreRect(const WebCore::IntRect& backingStoreRect)
{
    if (!m_webPage->isVisible())
        return;

    // Record the current backingstore rect
    IntRect currentBackingStoreRect = this->backingStoreRect();

    if (backingStoreRect == currentBackingStoreRect)
        return;

#if DEBUG_TILEMATRIX
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::setBackingStoreRect changed from (%d,%d %dx%d) to (%d,%d %dx%d)",
                           currentBackingStoreRect.x(),
                           currentBackingStoreRect.y(),
                           currentBackingStoreRect.width(),
                           currentBackingStoreRect.height(),
                           backingStoreRect.x(),
                           backingStoreRect.y(),
                           backingStoreRect.width(),
                           backingStoreRect.height());
#endif

    // The list of indexes we need to fill
    TileIndexList indexesToFill = indexesForBackingStoreRect(backingStoreRect);

    ASSERT(indexesToFill.size() == m_tileMap.size());

    TileMap tiles; // our new tile map
    TileMap leftOverTiles; // tiles that are left over

    // Iterate through our current tile map and add tiles that are rendered with
    // our new backingstore rect
    TileMap::const_iterator tileMapEnd = m_tileMap.end();
    for (TileMap::const_iterator it = m_tileMap.begin(); it != tileMapEnd; ++it) {
        TileIndex oldIndex = it->first;
        RefPtr<BackingStoreTile> tile = it->second;

        // Reset the old index
        resetTile(oldIndex, tile.get(), false /*resetBackground*/);

        // Origin of last committed render for tile in transformed content coordinates
        IntPoint origin = originOfLastRenderForTile(oldIndex, tile.get(), currentBackingStoreRect);

        // If the new backing store rect contains this origin, then insert the tile there
        // and mark it as no longer shifted.  Note: IntRect::contains checks for a 1x1 rect
        // below and to the right of the origin so it is correct usage here.
        if (backingStoreRect.contains(origin)) {
            TileIndex newIndex = indexOfTile(origin, backingStoreRect);
            IntRect rect(origin, tileSize());
            if (m_renderQueue->regularRenderJobsPreviouslyAttemptedButNotRendered(rect)) {
                // If the render queue previously tried to render this tile, but the
                // backingstore wasn't in the correct place or the tile wasn't visible
                // at the time then we can't simply restore the tile since the content
                // is now invalid as far as WebKit is concerned.  Instead, we clear
                // the tile here of the region and then put the tile in the render
                // queue again.

                // Intersect the tile with the not rendered region to get the areas
                // of the tile that we need to clear
                IntRectRegion tileNotRenderedRegion = IntRectRegion::intersectRegions(m_renderQueue->regularRenderJobsNotRenderedRegion(), rect);
                clearAndUpdateTileOfNotRenderedRegion(newIndex, tile.get(), tileNotRenderedRegion, backingStoreRect);
#if DEBUG_BACKINGSTORE
                IntRect extents = tileNotRenderedRegion.extents();
                Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::setBackingStoreRect did clear tile %d,%d %dx%d",
                                       extents.x(), extents.y(), extents.width(), extents.height());
#endif
            } else if (!tile->isCompletelyRendered() && !isCurrentVisibleJob(newIndex, tile.get(), backingStoreRect)) {
                // Mark as needing update
                updateTile(origin, false /*immediate*/);
            }

            // Do some bookkeeping with shifting tiles...
            tile->clearShift();
            tile->setCommitted(true);

            size_t i = indexesToFill.find(newIndex);
            ASSERT(i != WTF::notFound);
            indexesToFill.remove(i);
            tiles.add(newIndex, tile);
        } else {
            // Store this tile and index so we can add it to the remaining left over spots...
            leftOverTiles.add(oldIndex, tile);
        }
    }

    ASSERT(indexesToFill.size() == leftOverTiles.size());
    size_t i = 0;
    TileMap::const_iterator leftOverEnd = leftOverTiles.end();
    for (TileMap::const_iterator it = leftOverTiles.begin(); it != leftOverEnd; ++it) {
        TileIndex oldIndex = it->first;
        RefPtr<BackingStoreTile> tile = it->second;
        
        if (!(i < indexesToFill.size())) {
            ASSERT_NOT_REACHED();
            break;
        }

        TileIndex newIndex = indexesToFill.at(i);

        // Origin of last committed render for tile in transformed content coordinates
        IntPoint originOfOld = originOfLastRenderForTile(oldIndex, tile.get(), currentBackingStoreRect);
        // Origin of the new index for the new backingstore rect
        IntPoint originOfNew = originOfTile(newIndex, backingStoreRect);

        // Mark as needing update
        updateTile(originOfNew, false /*immediate*/);

        // Do some bookkeeping with shifting tiles...
        tile->clearShift();
        tile->setCommitted(false);
        tile->setHorizontalShift((originOfOld.x() - originOfNew.x()) / tileWidth());
        tile->setVerticalShift((originOfOld.y() - originOfNew.y()) / tileHeight());

        tiles.add(newIndex, tile);

        ++i;
    }

    // Actually set state
    m_numberOfTilesWide = backingStoreRect.width() / tileWidth();
    m_numberOfTilesHigh = backingStoreRect.height() / tileHeight();
    m_backingStoreOffset = backingStoreRect.location();
    ASSERT(m_tileMap.size() == tiles.size());
    m_tileMap = tiles;
}

BackingStorePrivate::TileIndexList BackingStorePrivate::indexesForBackingStoreRect(const WebCore::IntRect& backingStoreRect) const
{
    TileIndexList indexes;
    int numberOfTilesWide = backingStoreRect.width() / tileWidth();
    int numberOfTilesHigh = backingStoreRect.height() / tileHeight();
    for (int y = 0; y < numberOfTilesHigh; ++y) {
        for (int x = 0; x < numberOfTilesWide; ++x) {
            TileIndex index(x, y);
            indexes.append(index);
        }
    }
    return indexes;
}

WebCore::IntPoint BackingStorePrivate::originOfLastRenderForTile(const TileIndex& index,
                                                                 BackingStoreTile* tile,
                                                                 const WebCore::IntRect& backingStoreRect) const
{
    return originOfTile(indexOfLastRenderForTile(index, tile), backingStoreRect);
}

TileIndex BackingStorePrivate::indexOfLastRenderForTile(const TileIndex& index, BackingStoreTile* tile) const
{
    return TileIndex(index.i() + tile->horizontalShift(), index.j() + tile->verticalShift());
}

TileIndex BackingStorePrivate::indexOfTile(const WebCore::IntPoint& origin,
                                           const WebCore::IntRect& backingStoreRect) const
{
    int offsetX = origin.x() - backingStoreRect.x();
    int offsetY = origin.y() - backingStoreRect.y();
    if (offsetX)
        offsetX = offsetX / tileWidth();
    if (offsetY)
        offsetY = offsetY / tileHeight();
    return TileIndex(offsetX, offsetY);
}

void BackingStorePrivate::clearAndUpdateTileOfNotRenderedRegion(const TileIndex& index, BackingStoreTile* tile,
                                                                const WebCore::IntRectRegion& tileNotRenderedRegion,
                                                                const WebCore::IntRect& backingStoreRect,
                                                                bool update)
{
    // Intersect the tile with the not rendered region to get the areas
    // of the tile that we need to clear
    IntRectList tileNotRenderedRegionRects = tileNotRenderedRegion.rects();
    for (size_t i = 0; i < tileNotRenderedRegionRects.size(); ++i) {
        IntRect tileNotRenderedRegionRect = tileNotRenderedRegionRects.at(i);
        // Clear the render queue of this rect
        m_renderQueue->clear(tileNotRenderedRegionRect, true /*clearRegularRenderJobs*/);

        if (update) {
            // Add it again as a regular render job
            m_renderQueue->addToQueue(RenderQueue::RegularRender, tileNotRenderedRegionRect);
        }

        // Find the origin of this tile
        IntPoint origin = originOfTile(index, backingStoreRect);

        // Map to tile coordinates
        tileNotRenderedRegionRect.move(-origin.x(), -origin.y());

        // Clear the tile of this region
        tile->clearRenderedRegion(tileNotRenderedRegionRect);
    }
}

bool BackingStorePrivate::isCurrentVisibleJob(const TileIndex& index, BackingStoreTile* tile, const WebCore::IntRect& backingStoreRect) const
{
    // First check if the whole rect is in the queue
    IntRect wholeRect = IntRect(originOfTile(index, backingStoreRect), tileSize());
    if (m_renderQueue->isCurrentVisibleScrollJob(wholeRect) || m_renderQueue->isCurrentVisibleScrollJobCompleted(wholeRect))
        return true;

    // Second check if the individual parts of the non-rendered region are in the regular queue
    bool isCurrent = true; // it is true until it isn't :)

    IntRectList tileNotRenderedRegionRects = tile->notRenderedRegion().rects();
    for (size_t i = 0; i < tileNotRenderedRegionRects.size(); ++i) {
        IntRect tileNotRenderedRegionRect = tileNotRenderedRegionRects.at(i);
        // Find the origin of this tile
        IntPoint origin = originOfTile(index, backingStoreRect);

        // Map to transformed contents coordinates
        tileNotRenderedRegionRect.move(origin.x(), origin.y());

        // Check if it is current
        isCurrent = m_renderQueue->isCurrentRegularRenderJob(tileNotRenderedRegionRect) ? isCurrent : false;
    }

    return isCurrent;
}

void BackingStorePrivate::scrollBackingStore(int deltaX, int deltaY)
{
    if (!m_webPage->isVisible())
        return;

    // Calculate our new preferred matrix dimension
    if (deltaX || deltaY)
        m_preferredTileMatrixDimension = abs(deltaX) > abs(deltaY) ? Horizontal : Vertical;

    // Calculate our preferred matrix geometry
    Divisor divisor = bestDivisor(expandedContentsSize(),
                                  tileWidth(), tileHeight(),
                                  minimumNumberOfTilesWide(), minimumNumberOfTilesHigh(),
                                  m_preferredTileMatrixDimension);

#if DEBUG_TILEMATRIX
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::scrollBackingStore divisor %dx%d",
                           divisor.first,
                           divisor.second);
#endif

    // Initialize a rect with that new geometry
    IntRect backingStoreRect(0, 0, divisor.first * tileWidth(), divisor.second * tileHeight());

    // Scroll that rect so that it fits our contents and viewport and scroll delta
    backingStoreRect = backingStoreRectForScroll(deltaX, deltaY, backingStoreRect);

    ASSERT(!backingStoreRect.isEmpty());

    // Set the state
    setBackingStoreRect(backingStoreRect);
}

void BackingStorePrivate::render(const WebCore::IntRectList& rectList, bool renderContentOnly)
{
    // FIXME: We cycle through one by one and only blit the contents at the end.
    // This can be improved upon if we had a mapFromTransformedContentsToTiles that
    // took a set of rects and decomposed them appropriately
    for (size_t i = 0; i < rectList.size(); ++i)
        render(rectList.at(i), true);

    if (!renderContentOnly) {
        IntRect updateRect = blitVisibleContents();
        invalidateWindow(updateRect);
    }
}

void BackingStorePrivate::renderDirectToCanvas(const WebCore::IntRect& rect, bool renderContentOnly)
{
    // Request a layout now which might change the contents rect
    requestLayoutIfNeeded();

    IntRect dirtyRect = rect;
    dirtyRect.intersect(visibleContentsRect());

    if (dirtyRect.isEmpty())
        return;

    SurfaceOpenVG* renderingSurface = SurfacePool::globalSurfacePool()->tileRenderingSurface();
    m_webPage->d->renderContents(renderingSurface, 0 /*not needed since we're direct rendering to canvas*/, visibleContentsRect().location(), dirtyRect);

    bool shouldInvalidate = !m_suspendScreenUpdates && !renderContentOnly;
    if (!shouldInvalidate)
        return;

    IntRect screenRect = m_webPage->d->mapFromTransformedContentsToTransformedViewport(dirtyRect);
    m_webPage->d->invalidateWindow(screenRect);
}

void BackingStorePrivate::render(const WebCore::IntRect& rect, bool renderContentOnly)
{
    if (m_webPage->settings()->isDirectRenderingToCanvasEnabled()) {
        renderDirectToCanvas(rect, renderContentOnly);
        return;
    }

    if (!m_webPage->isVisible())
        return;

    // Request a layout now which might change the contents rect
    requestLayoutIfNeeded();

    TileRectList tileRectList = mapFromTransformedContentsToTiles(rect);
    if (tileRectList.isEmpty())
        return;

    IntRect screenRect;
    bool shouldBlit = !m_suspendScreenUpdates && !renderContentOnly;
    const IntRect contentsRect = expandedContentsRect();
    const IntRect viewportRect = IntRect(IntPoint(0, 0), m_webPage->d->transformedViewportSize());
    SurfaceOpenVG* renderingSurface = SurfacePool::globalSurfacePool()->tileRenderingSurface();

    for (size_t i = 0; i < tileRectList.size(); ++i) {
        TileRect tileRect = tileRectList[i];
        TileIndex index = tileRect.first;
        IntRect dirtyTileRect = tileRect.second;
        RefPtr<BackingStoreTile> tile = m_tileMap.get(index);

        // This dirty tile rect is in tile coordinates, but it needs to be in transformed contents coordinates
        IntRect dirtyRect = mapFromTilesToTransformedContents(tileRect);

        // If we're not yet committed, then commit now by clearing the rendered region
        // and setting the committed flag as well as clearing the shift
        if (!tile->isCommitted()) {
            tile->setCommitted(true);
            tile->clearRenderedRegion();
            tile->clearShift();
        }

        // If the tile has been created, but this is the first time we are painting on it
        // then it hasn't been given a default background yet so that we can save time during
        // startup.  That's why we are doing it here instead...
        if (!tile->backgroundPainted())
            tile->paintBackground();

        // Add the newly rendered region to the tile so it can keep track for blits
        tile->addRenderedRegion(dirtyTileRect);

        // Paint default background if contents rect is empty
        if (!contentsRect.isEmpty()) {
            // Otherwise we should clip the contents size and render the content
            dirtyRect.intersect(contentsRect);

            // We probably have extra tiles since the contents size is so small.  Save some cycles here...
            if (dirtyRect.isEmpty())
                continue;
        }

        // FIXME: modify render to take a Vector<IntRect> parameter so we're not recreating
        // GraphicsContext on the stack each time
        m_webPage->d->renderContents(renderingSurface, tile.get(), originOfTile(index), dirtyRect);

        // Now, this dirty rect is in transformed coordinates relative to the transformed contents, but
        // ultimately it needs to be transformed coordinates relative to the viewport.
        dirtyRect = m_webPage->d->mapFromTransformedContentsToTransformedViewport(dirtyRect);

        if (!shouldBlit)
            continue;

        IntRect dstRect = dirtyRect;
        IntRect srcRect = dirtyTileRect;

        // Clip to the viewport
        dstRect.intersect(viewportRect);

        // Clip to the visible content
        srcRect.intersect(tileVisibleContentsRect(index));

        m_webPage->client()->lockCanvas();
        m_webPage->d->blitToCanvas(dstRect, tile->buffer(), srcRect);
        m_webPage->client()->unlockCanvas();
        screenRect.unite(dstRect);
    }

    if (!shouldBlit)
        return;

    m_webPage->d->invalidateWindow(screenRect);
}

void BackingStorePrivate::requestLayoutIfNeeded() const
{
    m_webPage->d->requestLayoutIfNeeded();
}

void BackingStorePrivate::renderVisibleContents(bool renderContentOnly)
{
    render(visibleTilesRect(), renderContentOnly);
}

void BackingStorePrivate::renderBackingStore(bool renderContentOnly)
{
    render(backingStoreRect(), renderContentOnly);
}

WebCore::IntRect BackingStorePrivate::blitVisibleContents()
{
    if (m_webPage->settings()->isDirectRenderingToCanvasEnabled()) {
#if DEBUG_BACKINGSTORE
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::blitVisibleContents direct render to canvas");
#endif
        return IntRect(IntPoint(0, 0), m_webPage->d->transformedViewportSize());
    }

    if (m_suspendScreenUpdates)
        return IntRect();

    if (!m_webPage->isVisible())
        return IntRect();

#if DEBUG_BACKINGSTORE
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::blitVisibleContents");
#endif

    // Get the list of tile rects that makeup the visible content
    const IntRect contentsRect = IntRect(IntPoint(0, 0), m_webPage->d->transformedContentsSize());
    const IntRect viewportRect = IntRect(IntPoint(0, 0), m_webPage->d->transformedViewportSize());

    m_webPage->client()->lockCanvas();
    IntRect screenRect;
    TileRectList tileRectList = mapFromTransformedContentsToTiles(visibleContentsRect());
    for (size_t i = 0; i < tileRectList.size(); ++i) {
        TileRect tileRect = tileRectList[i];
        TileIndex index = tileRect.first;
        IntRect dirtyTileRect = tileRect.second;
        RefPtr<BackingStoreTile> tile = m_tileMap.get(index);

        // This dirty rect is in tile coordinates, but it needs to be in transformed contents coordinates
        IntRect dirtyRect = mapFromTilesToTransformedContents(tileRect);

        // Don't clip to contents if it is empty so we can still paint default background
        if (!contentsRect.isEmpty()) {
            // Otherwise we should clip the contents size and blit
            dirtyRect.intersect(contentsRect);

            // We probably have extra tiles since the contents size is so small.  Save some cycles here...
            if (dirtyRect.isEmpty())
                continue;
        }

        // Now, this dirty rect is in transformed coordinates relative to the transformed contents, but
        // ultimately it needs to be transformed coordinates relative to the viewport.
        dirtyRect = m_webPage->d->mapFromTransformedContentsToTransformedViewport(dirtyRect);

        // Clip to the viewport
        dirtyRect.intersect(viewportRect);

        // Clip to the visible content
        dirtyTileRect.intersect(tileVisibleContentsRect(index));

        // Save some cycles here...
        if (dirtyRect.isEmpty() || dirtyTileRect.isEmpty())
            continue;

        TileRect wholeTileRect;
        wholeTileRect.first = index;
        wholeTileRect.second = this->tileRect();
        IntRect wholeRect = mapFromTilesToTransformedContents(wholeTileRect);

        if (!tile->isCommitted() || !tile->isCompletelyRendered()) {
            // Blit the checkered pattern here
            RefPtr<BackingStoreTile> checkeredTile = SurfacePool::globalSurfacePool()->checkeredTile();
            m_webPage->d->blitToCanvas(dirtyRect, checkeredTile->buffer(), dirtyTileRect);
        }

        // Blit the visible buffer here if we have visible zoom jobs
        if (m_renderQueue->hasCurrentVisibleZoomJob()) {

            // Needs to be in same coordinate system as dirtyRect
            IntRect visibleTileBufferRect = m_webPage->d->mapFromTransformedContentsToTransformedViewport(m_visibleTileBufferRect);

            // Clip to the viewport
            visibleTileBufferRect.intersect(viewportRect);

            // Clip to the visibleTileBufferRect
            dirtyRect.intersect(visibleTileBufferRect);

            // Clip to the dirtyRect
            visibleTileBufferRect.intersect(dirtyRect);

            if (!dirtyRect.isEmpty() && !visibleTileBufferRect.isEmpty()) {
                BackingStoreTile* visibleTileBuffer = SurfacePool::globalSurfacePool()->visibleTileBuffer();
                ASSERT(visibleTileBuffer->size() == visibleContentsRect().size());

                // The offset of the current viewport with the visble tile buffer
                IntSize offset = visibleContentsRect().location() - m_visibleTileBufferRect.location();

                // Map to the visibleTileBuffer coordinates
                IntRect dirtyTileRect = visibleTileBufferRect;
                dirtyTileRect.move(offset);

                m_webPage->d->blitToCanvas(dirtyRect, visibleTileBuffer->buffer(), dirtyTileRect);
            }
        } else if (tile->isCommitted()) {
            // Intersect the rendered region
            IntRectRegion renderedRegion = tile->renderedRegion();
            Vector<IntRect> dirtyRenderedRects = renderedRegion.rects();
            for (size_t i = 0; i < dirtyRenderedRects.size(); ++i) {
                TileRect tileRect;
                tileRect.first = index;
                tileRect.second = intersection(dirtyTileRect, dirtyRenderedRects.at(i));
                if (tileRect.second.isEmpty())
                    continue;
                // Blit the rendered parts
                screenRect.unite(blitTileRect(tileRect));
            }
        } else {
            // Either the visual tiles for scroll are outdated or this tile should have been committed.
            bool isCurrentVisibleJob = this->isCurrentVisibleJob(index, tile.get(), backingStoreRect());
            ASSERT(isCurrentVisibleJob);
            if (!isCurrentVisibleJob) {
                Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "WARNING! Displaying checker, but no current visible job found for (%d,%d %dx%d)!",
                                       wholeRect.x(), wholeRect.y(), wholeRect.width(), wholeRect.height());
            }
        }
    }

#if ENABLE_SCROLLBARS
    if (m_autoHideScrollbarsTimer->isActive()) {
        if (scrollsHorizontally())
            blitHorizontalScrollbar();
        if (scrollsVertically())
            blitVerticalScrollbar();
    }
#endif

    m_webPage->client()->unlockCanvas();

    return screenRect;
}

WebCore::IntRect BackingStorePrivate::blitTileRect(const TileRect& tileRect)
{
    if (!m_webPage->isVisible())
        return IntRect();

    TileIndex index = tileRect.first;
    IntRect dirtyTileRect = tileRect.second;
    RefPtr<BackingStoreTile> tile = m_tileMap.get(index);

    // This dirty rect is in tile coordinates, but it needs to be in transformed contents coordinates
    IntRect dirtyRect = mapFromTilesToTransformedContents(tileRect);

    // Now, this dirty rect is in transformed coordinates relative to the transformed contents, but
    // ultimately it needs to be transformed coordinates relative to the viewport.
    dirtyRect = m_webPage->d->mapFromTransformedContentsToTransformedViewport(dirtyRect);

    ASSERT(!dirtyRect.isEmpty());
    ASSERT(!dirtyTileRect.isEmpty());
    if (dirtyRect.isEmpty() || dirtyTileRect.isEmpty())
        return IntRect();

    m_webPage->d->blitToCanvas(dirtyRect, tile->buffer(), dirtyTileRect);
    return dirtyRect;
}

void BackingStorePrivate::invalidateWindow(const WebCore::IntRect& screenRect)
{
    m_webPage->d->invalidateWindow(screenRect);
}

bool BackingStorePrivate::scrollsHorizontally() const
{
    return m_webPage->d->transformedActualVisibleSize().width() < m_webPage->d->transformedContentsSize().width();
}

bool BackingStorePrivate::scrollsVertically() const
{
    return m_webPage->d->transformedActualVisibleSize().height() < m_webPage->d->transformedContentsSize().height();
}

// The extent of the top of the scrollbar to the bottom of the screen
float BackingStorePrivate::horizontalScrollbarExtent() const
{
    if (!m_webPage->isVisible()) {
        ASSERT_NOT_REACHED();
        return 0.0;
    }

    if (!scrollsHorizontally() || !SurfacePool::globalSurfacePool()->horizontalScrollbar())
        return 0.0;

    FloatRect scrollbarRect = SurfacePool::globalSurfacePool()->horizontalScrollbar()->rect();
    scrollbarRect.inflateY(-1.5);
    ASSERT(scrollbarRect.height() >= 0);

    return scrollbarRect.height();
}

// The extent of the left of the scrollbar to the right of the screen
float BackingStorePrivate::verticalScrollbarExtent() const
{
    if (!m_webPage->isVisible()) {
        ASSERT_NOT_REACHED();
        return 0.0;
    }

    if (!scrollsVertically() || !SurfacePool::globalSurfacePool()->verticalScrollbar())
        return 0.0;

    FloatRect scrollbarRect = SurfacePool::globalSurfacePool()->verticalScrollbar()->rect();
    scrollbarRect.inflateX(-1.5);
    return scrollbarRect.width();
}

void BackingStorePrivate::blitHorizontalScrollbar()
{
    if (!m_webPage->isVisible())
        return;

    ASSERT(scrollsHorizontally());
    BackingStoreScrollbar* scrollbar = SurfacePool::globalSurfacePool()->horizontalScrollbar();

    if (!scrollbar)
        return;

    IntRect scrollbarRect = scrollbar->rect();
    IntRect canvasRect = scrollbarRect;
    IntSize visibleSize = m_webPage->d->transformedActualVisibleSize();
    canvasRect.move(0, visibleSize.height() - scrollbarRect.height());

    float verticalScrollbarExtent = this->verticalScrollbarExtent();

    float widthScale = static_cast<float>(visibleSize.width() - verticalScrollbarExtent)
                            / static_cast<float>(m_webPage->d->transformedContentsSize().width());
    float xScale = static_cast<float>(m_webPage->d->transformedScrollPosition().x())
                       / static_cast<float>(m_webPage->d->transformedMaximumScrollPosition().x());
    float width = static_cast<float>(scrollbarRect.width() - verticalScrollbarExtent) * widthScale;
    float x = (static_cast<float>(scrollbarRect.width() - verticalScrollbarExtent) - width) * xScale;

    IntRect hotRect = IntRect(roundf(x), 0, roundf(width), scrollbarRect.height());

    ASSERT(hotRect.width() >= 0 && hotRect.height() >= 0);

    hotRect.inflateY(-1);
    hotRect.inflateX(-1);

    // Don't draw scrollbars if they'll be 0-dimensional anyhow.
    if (hotRect.width() <= 0 || hotRect.height() <= 0)
        return;

    scrollbar->repaint(hotRect, false /*vertical*/);
    m_webPage->d->blendOntoCanvas(canvasRect, scrollbar->buffer(), scrollbar->alphaBuffer(), scrollbarRect, 255);
}

void BackingStorePrivate::blitVerticalScrollbar()
{
    if (!m_webPage->isVisible())
        return;

    ASSERT(scrollsVertically());
    BackingStoreScrollbar* scrollbar = SurfacePool::globalSurfacePool()->verticalScrollbar();

    if (!scrollbar)
        return;

    IntRect scrollbarRect = scrollbar->rect();
    IntRect canvasRect = scrollbarRect;
    IntSize visibleSize = m_webPage->d->transformedActualVisibleSize();
    canvasRect.move(visibleSize.width() - scrollbarRect.width(), 0);

    float horizontalScrollbarExtent = this->horizontalScrollbarExtent();

    float heightScale = static_cast<float>(visibleSize.height() - horizontalScrollbarExtent)
                            / static_cast<float>(m_webPage->d->transformedContentsSize().height());
    float yScale = static_cast<float>(m_webPage->d->transformedScrollPosition().y())
                       / static_cast<float>(m_webPage->d->transformedMaximumScrollPosition().y());
    float height = static_cast<float>(scrollbarRect.height() - horizontalScrollbarExtent) * heightScale;
    float y = (static_cast<float>(scrollbarRect.height() - horizontalScrollbarExtent) - height) * yScale;

    IntRect hotRect = IntRect(0, roundf(y), scrollbarRect.width(), roundf(height));

    ASSERT(hotRect.width() >= 0 && hotRect.height() >= 0);

    hotRect.inflateX(-1);
    hotRect.inflateY(-1);

    // Don't draw scrollbars if they'll be 0-dimensional anyhow.
    if (hotRect.width() <= 0 || hotRect.height() <= 0)
        return;

    scrollbar->repaint(hotRect, true /*vertical*/);
    m_webPage->d->blendOntoCanvas(canvasRect, scrollbar->buffer(), scrollbar->alphaBuffer(), scrollbarRect, 255);
}

void BackingStorePrivate::blitScaledArbitraryRectFromBackingStoreToScreen(const WebCore::IntRect& srcRect, WebCore::IntRect dstRect, Olympia::Platform::Graphics::Buffer* tempBuffer)
{
    ASSERT(tempBuffer);

    if (!m_webPage->isVisible())
        return;

    const IntRect contentsRect = IntRect(IntPoint(0, 0), m_webPage->d->transformedContentsSize());

    TransformationMatrix transformation = TransformationMatrix::rectToRect(
        FloatRect(FloatPoint(0.0, 0.0), srcRect.size()), dstRect);
    transformation.translate(-srcRect.x(), -srcRect.y());

    const IntRect viewportRect = IntRect(IntPoint(0, 0), m_webPage->d->transformedViewportSize());

    // Paint overZoomColor background for non-page area.
    // color is expected to be ARGB32
    Color color(m_webPage->settings()->overZoomColor());
    Olympia::Platform::Graphics::clearBuffer(tempBuffer, Olympia::Platform::IntRect(0, 0, dstRect.width(), dstRect.height()), color.red(), color.green(), color.blue());

    // Paint checkerboard for non-drawn page areas.
    IntRect checkerRect = srcRect;
    checkerRect.intersect(contentsRect);
    checkerRect = transformation.mapRect(checkerRect);
    checkerRect.intersect(viewportRect);

    if (!checkerRect.isEmpty()) {
        // Tile the checker tile into the image to ensure that we have full
        // coverage. In the case where tiles are portrait screen size, this
        // isn't too bad. It results in only one extra copy for landscape mode.
        // But this should be replaced with something more efficient in the
        // future in case backingstore tiles change size.
        BackingStoreTile* checkeredTile = SurfacePool::globalSurfacePool()->checkeredTile();
        IntSize checkerTileSize = checkeredTile->size();

        int firstXTile = checkerRect.x() / checkerTileSize.width();
        int firstYTile = checkerRect.y() / checkerTileSize.height();
        int lastXTile = (checkerRect.right() - 1) / checkerTileSize.width();
        int lastYTile = (checkerRect.bottom() - 1) / checkerTileSize.height();

        for (int i = firstXTile; i <= lastXTile; ++i) {
            for (int j = firstYTile; j <= lastYTile; ++j) {
                const int dstX = (i == firstXTile) ? checkerRect.x() : i * checkerTileSize.width();
                const int dstY = (j == firstYTile) ? checkerRect.y() : j * checkerTileSize.height();
                const int dstRight = (i == lastXTile) ? checkerRect.right() : (i + 1) * checkerTileSize.width();
                const int dstBottom = (j == lastYTile) ? checkerRect.bottom() : (j + 1) * checkerTileSize.height();
                const int srcX = dstX % checkerTileSize.width();
                const int srcY = dstY % checkerTileSize.height();

                const IntRect srcRect(srcX, srcY, dstRight - dstX, dstBottom - dstY);
                const IntPoint dstPoint(dstX, dstY);

                Olympia::Platform::Graphics::blitToBuffer(tempBuffer, dstPoint, checkeredTile->buffer(), srcRect);
            }
        }
        ASSERT_VG_NO_ERROR();
    }

    TileRectList tileRectList = mapFromTransformedContentsToTiles(srcRect);

    for (size_t i = 0; i < tileRectList.size(); ++i) {
        const TileRect& dirtyTile = tileRectList[i];
        const TileIndex& index = dirtyTile.first;
        const IntRect& dirtyTileRect = dirtyTile.second;
        RefPtr<BackingStoreTile> tile = m_tileMap.get(index);

        if (!tile->isCompletelyRendered())
            continue;

        // Compose part of temp surface represented by this dirty rect.
        IntRect rect = intersection(dirtyTileRect, IntRect(0, 0, tile->size().width(), tile->size().height()));

        // Clip the src tile rect to the actually rendered portion of the tile
        IntPoint originOfTile = this->originOfTile(index);
        IntPoint oldOrigin = rect.topLeft();
        rect.move(originOfTile.x(), originOfTile.y());
        rect.intersect(contentsRect);
        rect.setLocation(oldOrigin);

        if (rect.isEmpty())
            continue;

        // Map the image onto the canvas.
        IntRect dstDirtyTileRect = mapFromTilesToTransformedContents(dirtyTile);
        dstDirtyTileRect.intersect(contentsRect);
        dstDirtyTileRect = transformation.mapRect(dstDirtyTileRect);
        dstDirtyTileRect.intersect(viewportRect);

        if (!dstDirtyTileRect.isEmpty())
            Olympia::Platform::Graphics::stretchBlitToBuffer(tempBuffer, dstDirtyTileRect, tile->buffer(), rect);
    }

    dstRect.intersect(viewportRect);

    // blit temp surface to canvas
    m_webPage->client()->lockCanvas();
    m_webPage->d->blitToCanvas(dstRect, tempBuffer, IntRect(IntPoint(0, 0), dstRect.size()));
    m_webPage->client()->unlockCanvas();
    m_webPage->d->invalidateWindow(dstRect);
}


bool BackingStorePrivate::isTileVisible(const TileIndex& index) const
{
    TileRect tileRect;
    tileRect.first = index;
    tileRect.second = this->tileRect();
    return mapFromTilesToTransformedContents(tileRect).intersects(visibleContentsRect());
}

bool BackingStorePrivate::isTileVisible(const WebCore::IntPoint& origin) const
{
    return IntRect(origin, tileSize()).intersects(visibleContentsRect());
}

WebCore::IntRect BackingStorePrivate::visibleTilesRect() const
{
    IntRect rect;
    TileMap::const_iterator end = m_tileMap.end();
    for (TileMap::const_iterator it = m_tileMap.begin(); it != end; ++it) {
        TileRect tileRect;
        tileRect.first = it->first;
        tileRect.second = this->tileRect();
        IntRect tile = mapFromTilesToTransformedContents(tileRect);
        if (tile.intersects(visibleContentsRect()))
            rect.unite(tile);
    }
    return rect;
}

WebCore::IntRect BackingStorePrivate::tileVisibleContentsRect(const TileIndex& index) const
{
    if (!isTileVisible(index))
        return IntRect();

    TileRectList tileRectList = mapFromTransformedContentsToTiles(visibleContentsRect());
    for (size_t i = 0; i < tileRectList.size(); ++i) {
        TileRect tileRect = tileRectList[i];
        if (index == tileRect.first)
            return tileRect.second;
    }
    return IntRect();
}

void BackingStorePrivate::clearRenderQueue()
{
    m_renderQueue->clear();
}

void BackingStorePrivate::clearVisibleZoom()
{
    m_renderQueue->clearVisibleZoom();
}

void BackingStorePrivate::resetTiles(bool resetBackground)
{
    TileMap::const_iterator end = m_tileMap.end();
    for (TileMap::const_iterator it = m_tileMap.begin(); it != end; ++it)
        resetTile(it->first, it->second.get(), resetBackground);
}

void BackingStorePrivate::updateTiles(bool updateVisible, bool immediate)
{
    TileMap::const_iterator end = m_tileMap.end();
    for (TileMap::const_iterator it = m_tileMap.begin(); it != end; ++it) {
        bool isVisible = isTileVisible(it->first);
        if (!updateVisible && isVisible)
            continue;
        updateTile(it->first, immediate);
    }
}

void BackingStorePrivate::updateTilesForScrollOrNotRenderedRegion()
{
    // This method looks at all the tiles and if they are visible, but not completely
    // rendered or we are loading, then it updates them. For all tiles, visible and
    // non-visible, if a previous attempt was made to render them during a regular
    // render job, but they were not visible at the time, then update them and if
    // they are currently visible, reset them.

    bool isLoading = m_webPage->d->loadState() == WebPagePrivate::Committed;
    TileMap::const_iterator end = m_tileMap.end();
    for (TileMap::const_iterator it = m_tileMap.begin(); it != end; ++it) {
        TileIndex index = it->first;
        RefPtr<BackingStoreTile> tile = it->second;
        bool isVisible = isTileVisible(index);
        // The rect in transformed contents coordinates
        IntRect rect(originOfTile(index), tileSize());
        if (tile->isCommitted() && m_renderQueue->regularRenderJobsPreviouslyAttemptedButNotRendered(rect)) {
            // If the render queue previously tried to render this tile, but the
            // tile wasn't visible at the time we can't simply restore the tile
            // since the content is now invalid as far as WebKit is concerned.
            // Instead, we clear that part of the tile if it is visible and then
            // put the tile in the render queue again.
            if (isVisible) {
                // Intersect the tile with the not rendered region to get the areas
                // of the tile that we need to clear
                IntRectRegion tileNotRenderedRegion = IntRectRegion::intersectRegions(m_renderQueue->regularRenderJobsNotRenderedRegion(), rect);
                clearAndUpdateTileOfNotRenderedRegion(index, tile.get(), tileNotRenderedRegion, backingStoreRect(), false /*update*/);
#if DEBUG_BACKINGSTORE
                IntRect extents = tileNotRenderedRegion.extents();
                Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "BackingStorePrivate::updateTilesForScroll did clear tile %d,%d %dx%d",
                                       extents.x(), extents.y(), extents.width(), extents.height());
#endif
            }
            updateTile(index, false /*immediate*/);
        } else if (isVisible && (isLoading || !tile->isCompletelyRendered()) && !isCurrentVisibleJob(index, tile.get(), backingStoreRect()))
            updateTile(index, false /*immediate*/);
    }
}

void BackingStorePrivate::resetTile(const TileIndex& index, BackingStoreTile* tile, bool resetBackground)
{
    if (!m_webPage->isVisible())
        return;

    TileRect tileRect;
    tileRect.first = index;
    tileRect.second = this->tileRect();
    // Only clear regular render jobs if we're clearing the background too
    m_renderQueue->clear(mapFromTilesToTransformedContents(tileRect), resetBackground /*clearRegularRenderJobs*/);
    if (resetBackground)
        tile->reset();
}

void BackingStorePrivate::updateTile(const TileIndex& index, bool immediate)
{
    TileRect tileRect;
    tileRect.first = index;
    tileRect.second = this->tileRect();
    IntRect updateRect = mapFromTilesToTransformedContents(tileRect);
    RenderQueue::JobType jobType = isTileVisible(index) ? RenderQueue::VisibleScroll : RenderQueue::NonVisibleScroll;
    if (immediate)
        render(updateRect, true /*renderContentOnly*/);
    else
        m_renderQueue->addToQueue(jobType, updateRect);
}

void BackingStorePrivate::updateTile(const WebCore::IntPoint& origin, bool immediate)
{
    IntRect updateRect = IntRect(origin, tileSize());
    RenderQueue::JobType jobType = isTileVisible(origin) ? RenderQueue::VisibleScroll : RenderQueue::NonVisibleScroll;
    if (immediate)
        render(updateRect, true /*renderContentOnly*/);
    else
        m_renderQueue->addToQueue(jobType, updateRect);
}

WebCore::IntRect BackingStorePrivate::mapFromTilesToTransformedContents(const BackingStorePrivate::TileRect& tileRect) const
{
    TileIndex index = tileRect.first;
    IntRect rect = tileRect.second;
    // The origin of the tile including the backing store offset
    const IntPoint originOfTile = this->originOfTile(index);
    rect.move(originOfTile.x(), originOfTile.y());
    return rect;
}

BackingStorePrivate::TileRectList BackingStorePrivate::mapFromTransformedContentsToTiles(const WebCore::IntRect& rect) const
{
    if (!m_webPage->isVisible()) {
        ASSERT_NOT_REACHED();
        return TileRectList();
    }

    TileRectList tileRectList;
    TileMap::const_iterator end = m_tileMap.end();
    for (TileMap::const_iterator it = m_tileMap.begin(); it != end; ++it) {
        TileIndex index = it->first;
        RefPtr<BackingStoreTile> tile = it->second;

        // Need to map the rect to tile coordinates
        IntRect r = rect;

        // The origin of the tile including the backing store offset
        const IntPoint originOfTile = this->originOfTile(index);

        r.move(-(originOfTile.x()), -(originOfTile.y()));

        // Do we intersect the current tile or no?
        r.intersect(tile->rect());
        if (r.isEmpty())
            continue;

        // If we do append to list and Voila!
        TileRect tileRect;
        tileRect.first = index;
        tileRect.second = r;
        tileRectList.append(tileRect);
    }
    return tileRectList;
}

void BackingStorePrivate::updateTileMatrixIfNeeded()
{
    // This will update the tile matrix
    scrollBackingStore(0, 0);
}

void BackingStorePrivate::contentsSizeChanged(const WebCore::IntSize&)
{
    updateTileMatrixIfNeeded();
}

void BackingStorePrivate::scrollChanged(const IntPoint&)
{
    // FIXME: Need to do anything here?
}

void BackingStorePrivate::transformChanged()
{
    if (!m_webPage->isVisible())
        return;

    bool hasCurrentVisibleZoomJob = m_renderQueue->hasCurrentVisibleZoomJob();
    bool isLoading = m_webPage->d->loadState() == WebPagePrivate::Provisional || m_webPage->d->loadState() == WebPagePrivate::Committed;
    if (isLoading) {
        if (!hasCurrentVisibleZoomJob)
            m_visibleTileBufferRect = visibleContentsRect(); // cache this for blitVisibleContents

        // Add the currently visible tiles to the render queue as visible zoom jobs
        TileRectList tileRectList = mapFromTransformedContentsToTiles(visibleContentsRect());
        for (size_t i = 0; i < tileRectList.size(); ++i) {
            TileRect tileRect = tileRectList[i];
            TileIndex index = tileRect.first;
            IntRect dirtyTileRect = tileRect.second;
            RefPtr<BackingStoreTile> tile = m_tileMap.get(index);

            // Invalidate the whole rect
            tileRect.second = this->tileRect();
            IntRect wholeRect = mapFromTilesToTransformedContents(tileRect);
            m_renderQueue->addToQueue(RenderQueue::VisibleZoom, wholeRect);

            // Copy the visible contents into the visibleTileBuffer if we don't have any current visible zoom jobs
            if (!hasCurrentVisibleZoomJob) {
                // Map to the destination's coordinate system
                IntSize offset = this->originOfTile(index) - m_visibleTileBufferRect.location();
                IntRect dirtyRect = dirtyTileRect;
                dirtyRect.move(offset);

                BackingStoreTile* visibleTileBuffer = SurfacePool::globalSurfacePool()->visibleTileBuffer();
                ASSERT(visibleTileBuffer->size() == visibleContentsRect().size());
                Olympia::Platform::Graphics::blitToBuffer(visibleTileBuffer->buffer(), dirtyRect.location(), tile->buffer(), dirtyTileRect);
            }
        }
    }

    m_renderQueue->clear();
    resetTiles(true /*resetBackground*/);
}

void BackingStorePrivate::orientationChanged()
{
    updateTileMatrixIfNeeded();
    createVisibleTileBuffer();
}

void BackingStorePrivate::actualVisibleSizeChanged(const WebCore::IntSize& size)
{
#if ENABLE_SCROLLBARS
    if (!SurfacePool::globalSurfacePool()->horizontalScrollbar())
        return;

    if (!SurfacePool::globalSurfacePool()->verticalScrollbar())
        return;

    createScrollbars();

    if (m_webPage->settings()->isScrollbarsEnabled()) {
        // Restart the auto hide scrollbars timer
        m_autoHideScrollbarsTimer->stop();
        m_autoHideScrollbarsTimer->startOneShot(0.25);
    }

    // This only occurs with a window event, thus the window does not need to be
    // invalidated, rather just copy the visible contents to the current canvas
    // in order to show the scrollbars.
    blitVisibleContents();
#endif
}

static void createVisibleTileBufferForWebPage(WebPagePrivate* page)
{
    ASSERT(page);
    SurfacePool* surfacePool = SurfacePool::globalSurfacePool();
    surfacePool->initializeVisibleTileBuffer(page->transformedViewportSize());
}

static void createScrollbarsForWebPage(WebPagePrivate* page)
{
    ASSERT(page);
    IntSize horizontalSize = IntSize(page->transformedActualVisibleSize().width(), 6);
    IntSize verticalSize = IntSize(6, page->transformedActualVisibleSize().height());

    ASSERT(horizontalSize.width() <= page->m_webPage->width());
    ASSERT(verticalSize.height() <= page->m_webPage->height());

    SurfacePool* surfacePool = SurfacePool::globalSurfacePool();
    surfacePool->initializeScrollbars(horizontalSize, verticalSize);
}

void BackingStorePrivate::createSurfaces()
{
    ASSERT(m_tileMap.isEmpty());

    if (m_webPage->isVisible()) {
        // This method is only to be called as part of setting up a new web page instance and
        // before said instance is made visible so as to ensure a consistent definition of web
        // page visibility. That is, a web page is said to be visible when explicitly made visible.
        ASSERT_NOT_REACHED();
        return;
    }

    SurfacePool* surfacePool = SurfacePool::globalSurfacePool();
    if (surfacePool->isEmpty())
        surfacePool->initialize(tileSize());

    const Divisor divisor = bestDivisor(expandedContentsSize(), tileWidth(), tileHeight(), minimumNumberOfTilesWide(), minimumNumberOfTilesHigh(), m_preferredTileMatrixDimension);
    m_numberOfTilesWide = divisor.first;
    m_numberOfTilesHigh = divisor.second;

    const SurfacePool::TileList tileList = surfacePool->tileList();
    ASSERT(tileList.size() >= (m_numberOfTilesWide * m_numberOfTilesHigh));

    for (int y = 0; y < m_numberOfTilesHigh; ++y) {
        for (int x = 0; x < m_numberOfTilesWide; ++x) {
            TileIndex index(x, y);
            m_tileMap.add(index, tileList.at(x + y * m_numberOfTilesWide));
        }
    }

    createVisibleTileBufferForWebPage(m_webPage->d);
#if ENABLE_SCROLLBARS
    createScrollbarsForWebPage(m_webPage->d);
#endif
}

void BackingStorePrivate::createVisibleTileBuffer()
{
    if (!m_webPage->isVisible())
        return;

    createVisibleTileBufferForWebPage(m_webPage->d);
}

void BackingStorePrivate::createScrollbars()
{
    if (!m_webPage->isVisible())
        return;

    createScrollbarsForWebPage(m_webPage->d);
}

WebCore::IntPoint BackingStorePrivate::originOfTile(const TileIndex& index) const
{
    return originOfTile(index, backingStoreRect());
}

WebCore::IntPoint BackingStorePrivate::originOfTile(const TileIndex& index, const WebCore::IntRect& backingStoreRect) const
{
    return IntPoint(backingStoreRect.x() + (index.i() * tileWidth()),
                    backingStoreRect.y() + (index.j() * tileHeight()));
}

int BackingStorePrivate::numberOfTilesWide() const
{
    return m_numberOfTilesWide;
}

int BackingStorePrivate::numberOfTilesHigh() const
{
    return m_numberOfTilesHigh;
}

int BackingStorePrivate::minimumNumberOfTilesWide() const
{
    // The minimum number of tiles wide required to fill the viewport + 1 tile extra to allow scrolling
    return static_cast<int>(ceilf(m_webPage->d->transformedViewportSize().width() / static_cast<float>(tileWidth()))) + 1;
}

int BackingStorePrivate::minimumNumberOfTilesHigh() const
{
    // The minimum number of tiles high required to fill the viewport + 1 tile extra to allow scrolling
    return static_cast<int>(ceilf(m_webPage->d->transformedViewportSize().height() / static_cast<float>(tileHeight()))) + 1;
}

int BackingStorePrivate::tileWidth() const
{
    static int tw = m_webPage->width();
    return tw;
}

int BackingStorePrivate::tileHeight() const
{
    static int th = m_webPage->height();
    return th;
}

WebCore::IntSize BackingStorePrivate::tileSize() const
{
    return IntSize(tileWidth(), tileHeight());
}

WebCore::IntRect BackingStorePrivate::tileRect() const
{
    return IntRect(0, 0, tileWidth(), tileHeight());
}

WebCore::IntSize BackingStorePrivate::expandedContentsSize() const
{
    return m_webPage->d->transformedContentsSize().expandedTo(m_webPage->d->transformedViewportSize());
}

WebCore::IntSize BackingStorePrivate::backingStoreSize() const
{
    return IntSize(numberOfTilesWide() * tileWidth(), numberOfTilesHigh() * tileHeight());
}

BackingStore::BackingStore(WebPage* webPage)
    : d(new BackingStorePrivate)
{
    d->m_webPage = webPage;
}

BackingStore::~BackingStore()
{
    delete d;
    d = 0;
}

void BackingStore::createSurface()
{
    static bool initialized = false;
    if (!initialized) {
        Olympia::Platform::Graphics::initialize();
        initialized = true;
    }

    d->m_eglDisplay = Olympia::Platform::Graphics::display();

    // Make sure we are using OpenVG
    eglBindAPI(EGL_OPENVG_API);
    ASSERT_EGL_NO_ERROR();

    EGLDisplayOpenVG::setCurrentDisplay(d->m_eglDisplay);

    // Triggers creation of surfaces in backingstore
    d->createSurfaces();

    EGLDisplayOpenVG::current()->sharedPlatformSurface()->makeCurrent();

    // Focusing the WebPage triggers a repaint, so while we want it to be
    // focused initially this has to happen after creation of the surface.
    d->m_webPage->setFocused(true);
}

EGLDisplay BackingStore::display() const
{
    return d->m_eglDisplay;
}

void BackingStore::repaint(int x, int y, int width, int height,
                           bool contentChanged, bool immediate, bool repaintContentOnly)
{
    d->repaint(IntRect(x, y, width, height), contentChanged, immediate, repaintContentOnly);
}

bool BackingStore::hasIdleJobs() const
{
    return d->shouldPerformRenderJobs();
}

void BackingStore::renderOnIdle()
{
    d->renderOnIdle();
}

}
}
