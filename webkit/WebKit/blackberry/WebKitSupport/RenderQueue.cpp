/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "RenderQueue.h"

#include "BackingStore_p.h"
#include "CString.h"
#include "OlympiaPlatformMisc.h"
#include "PlatformString.h"
#include "WebPage.h"
#include "WebPage_p.h"

#include <wtf/CurrentTime.h>
#include <wtf/NotFound.h>

#include <math.h>

// The maximum amount of time that any render scroll operation should take in milliseconds
#define MAXIMUM_TILE_RENDER_THRESHOLD 100
// The maximum number of times we can split a render rect
#define MAXIMUM_SPLITTING_FACTOR 4

#define DEBUG_RENDER_QUEUE 0
#define DEBUG_RENDER_QUEUE_SORT 0

using WTF::String;
using namespace WebCore;

namespace Olympia {
namespace WebKit {

void splitRectInHalfAndAddToList(const WebCore::IntRect rect, bool vertical, WebCore::IntRectList& renderRectList)
{
    if (vertical) {
        int width1 = static_cast<int>(ceilf(static_cast<float>(rect.width()) / 2.0));
        int width2 = static_cast<int>(floorf(static_cast<float>(rect.width()) / 2.0));
        renderRectList.append(IntRect(rect.x(), rect.y(), width1, rect.height()));
        renderRectList.append(IntRect(rect.x() + width1, rect.y(), width2, rect.height()));
    } else {
        int height1 = static_cast<int>(ceilf(static_cast<float>(rect.height()) / 2.0));
        int height2 = static_cast<int>(floorf(static_cast<float>(rect.height()) / 2.0));
        renderRectList.append(IntRect(rect.x(), rect.y(), rect.width(), height1));
        renderRectList.append(IntRect(rect.x(), rect.y() + height1, rect.width(), height2));
    }
}

template<SortDirection sortDirection>
static inline int compareRectOneDirection(const WebCore::IntRect& r1, const WebCore::IntRect& r2)
{
    switch (sortDirection) {
    case LeftToRight:
        return r1.x() - r2.x();
    case RightToLeft:
        return r2.x() - r1.x();
    case TopToBottom:
        return r1.y() - r2.y();
    case BottomToTop:
        return r2.y() - r1.y();
    default:
        break;
    }
    ASSERT_NOT_REACHED();
    return 0;
}

template<SortDirection primarySortDirection, SortDirection secondarySortDirection>
static bool rectIsLessThan(const WebCore::IntRect& r1, const WebCore::IntRect& r2)
{                    
    int primaryResult = compareRectOneDirection<primarySortDirection>(r1, r2);

    if (primaryResult || secondarySortDirection == primarySortDirection)
        return primaryResult < 0;

    return compareRectOneDirection<secondarySortDirection>(r1, r2) < 0;
}

typedef bool (*FuncRectLessThan)(const WebCore::IntRect& r1, const WebCore::IntRect& r2);

static FuncRectLessThan rectLessThanFunction(SortDirection primary, SortDirection secondary)
{
    static FuncRectLessThan s_rectLessThanFunctions[NumSortDirections][NumSortDirections] = { 0 };
    static bool s_initialized = false;
    if (!s_initialized) {
#define ADD_COMPARE_FUNCTION(_primary, _secondary) \
        s_rectLessThanFunctions[_primary][_secondary] = rectIsLessThan<_primary, _secondary>

        ADD_COMPARE_FUNCTION(LeftToRight, LeftToRight);
        ADD_COMPARE_FUNCTION(LeftToRight, RightToLeft);
        ADD_COMPARE_FUNCTION(LeftToRight, TopToBottom);
        ADD_COMPARE_FUNCTION(LeftToRight, BottomToTop);

        ADD_COMPARE_FUNCTION(RightToLeft, LeftToRight);
        ADD_COMPARE_FUNCTION(RightToLeft, RightToLeft);
        ADD_COMPARE_FUNCTION(RightToLeft, TopToBottom);
        ADD_COMPARE_FUNCTION(RightToLeft, BottomToTop);

        ADD_COMPARE_FUNCTION(TopToBottom, LeftToRight);
        ADD_COMPARE_FUNCTION(TopToBottom, RightToLeft);
        ADD_COMPARE_FUNCTION(TopToBottom, TopToBottom);
        ADD_COMPARE_FUNCTION(TopToBottom, BottomToTop);

        ADD_COMPARE_FUNCTION(BottomToTop, LeftToRight);
        ADD_COMPARE_FUNCTION(BottomToTop, RightToLeft);
        ADD_COMPARE_FUNCTION(BottomToTop, TopToBottom);
        ADD_COMPARE_FUNCTION(BottomToTop, BottomToTop);
#undef ADD_COMPARE_FUNCTION

        s_initialized = true;
    }

    return s_rectLessThanFunctions[primary][secondary];
}

class RectLessThan {
public:
    RectLessThan(SortDirection primarySortDirection, SortDirection secondarySortDirection)
        : m_rectIsLessThan(rectLessThanFunction(primarySortDirection, secondarySortDirection))
    {
    }

    bool operator()(const WebCore::IntRect& r1, const WebCore::IntRect& r2)
    {              
        return m_rectIsLessThan(r1, r2);
    }

private:
    FuncRectLessThan m_rectIsLessThan;
};

class RenderRectLessThan {
public:
    RenderRectLessThan(SortDirection primarySortDirection, SortDirection secondarySortDirection)
        : m_rectIsLessThan(rectLessThanFunction(primarySortDirection, secondarySortDirection))
    {
    }

    bool operator()(const RenderRect& r1, const RenderRect& r2)
    {              
        return m_rectIsLessThan(r1.subRects().first(), r2.subRects().first());
    }

private:
    FuncRectLessThan m_rectIsLessThan;
};

RenderRect::RenderRect(const WebCore::IntPoint& location, const WebCore::IntSize& size, int splittingFactor)
    : WebCore::IntRect(location, size)
    , m_splittingFactor(0)
    , m_primarySortDirection(LeftToRight)
    , m_secondarySortDirection(TopToBottom)
{
    initialize(splittingFactor);
}

RenderRect::RenderRect(int x, int y, int width, int height, int splittingFactor)
    : WebCore::IntRect(x, y, width, height)
    , m_splittingFactor(0)
    , m_primarySortDirection(LeftToRight)
    , m_secondarySortDirection(TopToBottom)
{
    initialize(splittingFactor);
}

void RenderRect::initialize(int splittingFactor)
{
    m_subRects.append(*this);
    for (int i = 0; i < splittingFactor; ++i)
        split();
    quickSort();
}

void RenderRect::split()
{
    m_splittingFactor++;

    bool vertical = !(m_splittingFactor % 2);

    IntRectList subRects;
    for (size_t i = 0; i < m_subRects.size(); ++i)
        splitRectInHalfAndAddToList(m_subRects.at(i), vertical, subRects);
    m_subRects.swap(subRects);
}

WebCore::IntRect RenderRect::rectForRendering()
{
    ASSERT(!m_subRects.isEmpty());
    IntRect rect = m_subRects.first();
    m_subRects.remove(0);
    return rect;
}

void RenderRect::updateSortDirection(SortDirection primary, SortDirection secondary)
{
    if (primary == m_primarySortDirection && secondary == m_secondarySortDirection)
        return;

    m_primarySortDirection = primary;
    m_secondarySortDirection = secondary;

    quickSort();
}

void RenderRect::quickSort()
{
    std::sort(m_subRects.begin(), m_subRects.begin(), RectLessThan(m_primarySortDirection, m_secondarySortDirection));
}

RenderQueue::RenderQueue(BackingStorePrivate* parent)
    : m_parent(parent)
    , m_rectsAddedToRegularRenderJobsInCurrentCycle(false)
    , m_currentRegularRenderJobsBatchUnderPressure(false)
    , m_primarySortDirection(LeftToRight)
    , m_secondarySortDirection(TopToBottom)
{
}

RenderQueue::~RenderQueue()
{
}

int RenderQueue::splittingFactor(const WebCore::IntRect& rect) const
{
    // This method is used to split up regular render rect jobs and we want it to
    // to be zoom invariant with respect to WebCore.  In other words, if WebCore sends
    // us a rect of viewport size to invalidate at zoom 1.0 then we split that up
    // in the exact same way we would at zoom 2.0.  The amount of content that is
    // rendered in any one pass should stay fixed with regard to the zoom level.
    IntRect untransformedRect = m_parent->m_webPage->d->mapFromTransformed(rect);
    double rectArea = untransformedRect.width() * untransformedRect.height();
    double maxArea = DEFAULT_MAX_LAYOUT_WIDTH * DEFAULT_MAX_LAYOUT_HEIGHT; // defined in WebPage_p.h
    double renderRectArea = maxArea / pow(2.0, 2 /*default split factor*/);
    return ceil(log(rectArea / renderRectArea) / log(2.0));
}

RenderRect RenderQueue::convertToRenderRect(const WebCore::IntRect& rect) const
{
    return RenderRect(rect.location(), rect.size(), splittingFactor(rect));
}

WebCore::IntRectList RenderQueue::convertToRenderRectList(const WebCore::IntRectList& rectList) const
{
    IntRectList renderRectList;
    for (size_t i = 0; i < rectList.size(); ++i)
        renderRectList.append(convertToRenderRectList(rectList.at(i)));
    return renderRectList;
}

WebCore::IntRectList RenderQueue::convertToRenderRectList(const WebCore::IntRect& rect) const
{
    return convertToRenderRect(rect).subRects();
}

bool RenderQueue::isEmpty(bool shouldPerformRegularRenderJobs) const
{
    return m_visibleZoomJobs.isEmpty()
            && m_visibleScrollJobs.isEmpty()
            && (!shouldPerformRegularRenderJobs || m_currentRegularRenderJobsBatch.isEmpty())
            && (!shouldPerformRegularRenderJobs || m_regularRenderJobsRegion.isEmpty())
            && m_nonVisibleScrollJobs.isEmpty();
}

bool RenderQueue::hasCurrentRegularRenderJob() const
{
    return !m_currentRegularRenderJobsBatch.isEmpty() || !m_regularRenderJobsRegion.isEmpty();
}

bool RenderQueue::hasCurrentVisibleZoomJob() const
{
    return !m_visibleZoomJobs.isEmpty();
}

bool RenderQueue::hasCurrentVisibleScrollJob() const
{
    return !m_visibleScrollJobs.isEmpty();
}

bool RenderQueue::isCurrentVisibleScrollJob(const WebCore::IntRect& rect) const
{
    return m_visibleScrollJobs.find(rect) != WTF::notFound;
}

bool RenderQueue::isCurrentVisibleScrollJobCompleted(const WebCore::IntRect& rect) const
{
    return m_visibleScrollJobsCompleted.find(rect) != WTF::notFound;
}

bool RenderQueue::isCurrentRegularRenderJob(const WebCore::IntRect& rect) const
{
    return m_regularRenderJobsRegion.isRectInRegion(rect) == IntRectRegion::ContainedInRegion
        || m_currentRegularRenderJobsBatchRegion.isRectInRegion(rect) == IntRectRegion::ContainedInRegion;
}

bool RenderQueue::currentRegularRenderJobBatchUnderPressure() const
{
    return m_currentRegularRenderJobsBatchUnderPressure;
}

void RenderQueue::setCurrentRegularRenderJobBatchUnderPressure(bool b)
{
    m_currentRegularRenderJobsBatchUnderPressure = b;
}

void RenderQueue::eventQueueCycled()
{
    // Called by the backingstore when the event queue has cycled to allow the
    // render queue to determine if the regular render jobs are under pressure
    if (m_rectsAddedToRegularRenderJobsInCurrentCycle && m_currentRegularRenderJobsBatchRegion.isEmpty())
        m_currentRegularRenderJobsBatchUnderPressure = true;

    m_rectsAddedToRegularRenderJobsInCurrentCycle = false;
}

void RenderQueue::addToQueue(JobType type, const WebCore::IntRectList& rectList)
{
    for (size_t i = 0; i < rectList.size(); ++i)
        addToQueue(type, rectList.at(i));
}

void RenderQueue::addToQueue(JobType type, const WebCore::IntRect& rect)
{
    switch (type) {
    case VisibleZoom:
        addToScrollZoomQueue(convertToRenderRect(rect), &m_visibleZoomJobs);
        break;
    case VisibleScroll:
        addToScrollZoomQueue(convertToRenderRect(rect), &m_visibleScrollJobs);
        break;
    case RegularRender:
        {
            // Flag that we added rects in the current event queue cycle
            m_rectsAddedToRegularRenderJobsInCurrentCycle = true;

            // We try and detect if this newly added rect intersects or is contained in the currently running
            // batch of render jobs.  If so, then we have to start the batch over since we decompose individual
            // rects into subrects and might have already rendered one of them.  If the webpage's content has
            // changed state then this can lead to artifacts.  We mark this by noting the batch is now under pressure
            // and the backingstore will attempt to clear it at the next available opportunity.
            IntRectRegion::IntersectionState state = m_currentRegularRenderJobsBatchRegion.isRectInRegion(rect);
            if (state == IntRectRegion::ContainedInRegion || state == IntRectRegion::PartiallyContainedInRegion) {
                m_regularRenderJobsRegion = IntRectRegion::unionRegions(m_regularRenderJobsRegion, m_currentRegularRenderJobsBatchRegion);
                m_currentRegularRenderJobsBatch.clear();
                m_currentRegularRenderJobsBatchRegion = IntRectRegion();
                m_currentRegularRenderJobsBatchUnderPressure = true;
            }
            addToRegularQueue(rect);
        }
        break;
    case NonVisibleScroll:
        // Check if the rect is in higher priority queue and return if so
        if (m_visibleScrollJobs.find(rect) != WTF::notFound)
            return;
        addToScrollZoomQueue(convertToRenderRect(rect), &m_nonVisibleScrollJobs);
        break;
    default:
        ASSERT_NOT_REACHED();
    }
}

void RenderQueue::addToRegularQueue(const WebCore::IntRect& rect)
{
#if DEBUG_RENDER_QUEUE
    if (m_regularRenderJobsRegion.isRectInRegion(rect) != IntRectRegion::ContainedInRegion)
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::addToRegularQueue %d,%d %dx%d",
                               rect.x(), rect.y(), rect.width(), rect.height());
#endif
    m_regularRenderJobsRegion = IntRectRegion::unionRegions(m_regularRenderJobsRegion, rect);
}

void RenderQueue::addToScrollZoomQueue(const RenderRect& rect, RenderRectList* rectList)
{
    if (rectList->find(rect) != WTF::notFound)
        return;
#if DEBUG_RENDER_QUEUE
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::addToScrollZoomQueue %d,%d %dx%d",
                           rect.x(), rect.y(), rect.width(), rect.height());
#endif
    rectList->append(rect);

    if (hasCurrentVisibleZoomJob() || hasCurrentVisibleScrollJob())
        m_parent->startRenderTimer(); // Start the render timer since we know we could have some checkerboard here...
}

void RenderQueue::quickSort(RenderRectList* queue)
{
    if (queue->isEmpty())
        return;

    for (size_t i = 0; i < queue->size(); ++i)
        queue->at(i).updateSortDirection(m_primarySortDirection, m_secondarySortDirection);

    return std::sort(queue->begin(), queue->end(), RenderRectLessThan(m_primarySortDirection, m_secondarySortDirection));
}

void RenderQueue::updateSortDirection(int lastDeltaX, int lastDeltaY)
{
    bool primaryIsHorizontal = abs(lastDeltaX) >= abs(lastDeltaY);

    if (primaryIsHorizontal)
        m_primarySortDirection = lastDeltaX <= 0 ? LeftToRight : RightToLeft;
    else
        m_primarySortDirection = lastDeltaY <= 0 ? TopToBottom : BottomToTop;

    if (!primaryIsHorizontal)
        m_secondarySortDirection = lastDeltaX <= 0 ? LeftToRight : RightToLeft;
    else
        m_secondarySortDirection = lastDeltaY <= 0 ? TopToBottom : BottomToTop;
}

void RenderQueue::visibleContentChanged(const WebCore::IntRect& visibleContent)
{
    if (m_visibleScrollJobs.isEmpty() && m_nonVisibleScrollJobs.isEmpty()) {
        ASSERT(m_visibleScrollJobsCompleted.isEmpty() && m_nonVisibleScrollJobsCompleted.isEmpty());
        return;
    }

    // Move visibleScrollJobs to nonVisibleScrollJobs if they do not intersect
    // the visible content rect
    for (size_t i = 0; i < m_visibleScrollJobs.size(); ++i) {
        RenderRect rect = m_visibleScrollJobs.at(i);
        if (!rect.intersects(visibleContent)) {
            m_visibleScrollJobs.remove(i);
            addToScrollZoomQueue(rect, &m_nonVisibleScrollJobs);
            --i;
        }
    }

    // Do the same for the completed list
    for (size_t i = 0; i < m_visibleScrollJobsCompleted.size(); ++i) {
        RenderRect rect = m_visibleScrollJobsCompleted.at(i);
        if (!rect.intersects(visibleContent)) {
            m_visibleScrollJobsCompleted.remove(i);
            addToScrollZoomQueue(rect, &m_nonVisibleScrollJobsCompleted);
            --i;
        }
    }

    // Move nonVisibleScrollJobs to visibleScrollJobs if they do intersect
    // the visible content rect
    for (size_t i = 0; i < m_nonVisibleScrollJobs.size(); ++i) {
        RenderRect rect = m_nonVisibleScrollJobs.at(i);
        if (rect.intersects(visibleContent)) {
            m_nonVisibleScrollJobs.remove(i);
            addToScrollZoomQueue(rect, &m_visibleScrollJobs);
            --i;
        }
    }

    // Do the same for the completed list
    for (size_t i = 0; i < m_nonVisibleScrollJobsCompleted.size(); ++i) {
        RenderRect rect = m_nonVisibleScrollJobsCompleted.at(i);
        if (rect.intersects(visibleContent)) {
            m_nonVisibleScrollJobsCompleted.remove(i);
            addToScrollZoomQueue(rect, &m_visibleScrollJobsCompleted);
            --i;
        }
    }

    if (m_visibleScrollJobs.isEmpty() && !m_visibleScrollJobsCompleted.isEmpty())
        visibleScrollJobsCompleted(false /*shouldBlit*/);

    if (m_nonVisibleScrollJobs.isEmpty() && !m_nonVisibleScrollJobsCompleted.isEmpty())
        nonVisibleScrollJobsCompleted();

    if (!hasCurrentVisibleZoomJob() && !hasCurrentVisibleScrollJob())
         m_parent->stopRenderTimer();

    // We shouldn't be empty because the early return above and the fact that this
    // method just shuffles rects from queue to queue hence the total number of
    // rects in the various queues should be conserved
    ASSERT(!isEmpty());
}

void RenderQueue::clear(const WebCore::IntRectRegion& region, bool clearRegularRenderJobs)
{
    IntRectList rects = region.rects();
    for (size_t i = 0; i < rects.size(); ++i)
        clear(rects.at(i), clearRegularRenderJobs);
}

void RenderQueue::clear(const WebCore::IntRect& rect, bool clearRegularRenderJobs)
{
    if (m_visibleScrollJobs.isEmpty() && m_nonVisibleScrollJobs.isEmpty())
        ASSERT(m_visibleScrollJobsCompleted.isEmpty() && m_nonVisibleScrollJobsCompleted.isEmpty());

    // Remove all rects from all queues that are contained by this rect
    for (size_t i = 0; i < m_visibleScrollJobs.size(); ++i)
        if (rect.contains(m_visibleScrollJobs.at(i))) {
            m_visibleScrollJobs.remove(i);
            --i;
        }

    for (size_t i = 0; i < m_visibleScrollJobsCompleted.size(); ++i)
        if (rect.contains(m_visibleScrollJobsCompleted.at(i))) {
            m_visibleScrollJobsCompleted.remove(i);
            --i;
        }

    for (size_t i = 0; i < m_nonVisibleScrollJobs.size(); ++i)
        if (rect.contains(m_nonVisibleScrollJobs.at(i))) {
            m_nonVisibleScrollJobs.remove(i);
            --i;
        }

    for (size_t i = 0; i < m_nonVisibleScrollJobsCompleted.size(); ++i)
        if (rect.contains(m_nonVisibleScrollJobsCompleted.at(i))) {
            m_nonVisibleScrollJobsCompleted.remove(i);
            --i;
        }

    // Only clear the regular render jobs if the flag has been set...
    if (clearRegularRenderJobs) {
        for (size_t i = 0; i < m_currentRegularRenderJobsBatch.size(); ++i)
            if (rect.contains(m_currentRegularRenderJobsBatch.at(i))) {
                m_currentRegularRenderJobsBatch.remove(i);
                --i;
            }

        m_regularRenderJobsRegion = IntRectRegion::subtractRegions(m_regularRenderJobsRegion, rect);
        m_currentRegularRenderJobsBatchRegion = IntRectRegion::subtractRegions(m_currentRegularRenderJobsBatchRegion, rect);
        m_regularRenderJobsNotRenderedRegion = IntRectRegion::subtractRegions(m_regularRenderJobsNotRenderedRegion, rect);
    }

    if (m_visibleScrollJobs.isEmpty() && !m_visibleScrollJobsCompleted.isEmpty())
        visibleScrollJobsCompleted(false /*shouldBlit*/);

    if (m_nonVisibleScrollJobs.isEmpty() && !m_nonVisibleScrollJobsCompleted.isEmpty())
        nonVisibleScrollJobsCompleted();

    if (!hasCurrentVisibleZoomJob() && !hasCurrentVisibleScrollJob())
         m_parent->stopRenderTimer();
}

void RenderQueue::clear()
{
    m_visibleZoomJobs.clear();
    m_visibleScrollJobs.clear();
    m_visibleScrollJobsCompleted.clear();
    m_nonVisibleScrollJobs.clear();
    m_nonVisibleScrollJobsCompleted.clear();
    m_regularRenderJobsRegion = IntRectRegion();
    m_currentRegularRenderJobsBatch.clear();
    m_currentRegularRenderJobsBatchRegion = IntRectRegion();
    m_currentRegularRenderJobsBatchUnderPressure = false;
    m_regularRenderJobsNotRenderedRegion = IntRectRegion();
    m_parent->stopRenderTimer();
    ASSERT(isEmpty());
}

void RenderQueue::clearVisibleZoom()
{
    m_visibleZoomJobs.clear();

    if (!hasCurrentVisibleZoomJob() && !hasCurrentVisibleScrollJob())
         m_parent->stopRenderTimer();
}

bool RenderQueue::regularRenderJobsPreviouslyAttemptedButNotRendered(const WebCore::IntRect& rect)
{
    return m_regularRenderJobsNotRenderedRegion.isRectInRegion(rect) != IntRectRegion::NotInRegion;
}

void RenderQueue::render(bool shouldPerformRegularRenderJobs)
{
    // We request a layout here to ensure that we're executing jobs in the correct
    // order.  If we didn't request a layout here then the jobs below could result
    // in a layout and that layout can alter this queue.  So request layout if needed
    // to ensure that the queues below are in constant state before performing the
    // next rendering job.

#if DEBUG_RENDER_QUEUE
    // Start the time measurement...
    double time = WTF::currentTime();
#endif

    m_parent->requestLayoutIfNeeded();

#if DEBUG_RENDER_QUEUE
    double elapsed = WTF::currentTime() - time;
    if (elapsed)
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::render layout elapsed=%f", elapsed);
#endif

    // Empty the queues in a precise order of priority
    if (!m_visibleZoomJobs.isEmpty())
        renderVisibleZoomJob();
    else if (!m_visibleScrollJobs.isEmpty())
        renderVisibleScrollJob();
    else if (shouldPerformRegularRenderJobs && (!m_currentRegularRenderJobsBatch.isEmpty() || !m_regularRenderJobsRegion.isEmpty()))
        if (currentRegularRenderJobBatchUnderPressure())
            renderAllCurrentRegularRenderJobs();
        else
            renderRegularRenderJob();
    else if (!m_nonVisibleScrollJobs.isEmpty())
        renderNonVisibleScrollJob();
    else
        ASSERT_NOT_REACHED();
}

void RenderQueue::renderAllCurrentRegularRenderJobs()
{
#if DEBUG_RENDER_QUEUE
    // Start the time measurement...
    double time = WTF::currentTime();
#endif

    // Request layout first
    m_parent->requestLayoutIfNeeded();

#if DEBUG_RENDER_QUEUE
    double elapsed = WTF::currentTime() - time;
    if (elapsed)
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::renderAllCurrentRegularRenderJobs layout elapsed=%f", elapsed);
#endif

    ASSERT(!m_currentRegularRenderJobsBatchRegion.isEmpty() || !m_regularRenderJobsRegion.isEmpty());

    // If there is no current batch of jobs, then create one
    if (m_currentRegularRenderJobsBatchRegion.isEmpty()) {

        // Create a current region object from our regular render region
        m_currentRegularRenderJobsBatchRegion = m_regularRenderJobsRegion;

        // Clear this since we're about to render everything
        m_regularRenderJobsRegion = IntRectRegion();
    }

    // Record any part of the region that doesn't intersect the current visible contents rect
    IntRectRegion regionNotRendered = IntRectRegion::subtractRegions(m_currentRegularRenderJobsBatchRegion, m_parent->visibleContentsRect());
    m_regularRenderJobsNotRenderedRegion = IntRectRegion::unionRegions(m_regularRenderJobsNotRenderedRegion, regionNotRendered);

#if DEBUG_RENDER_QUEUE
    if (!regionNotRendered.isEmpty())
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::renderAllCurrentRegularRenderJobs region not completely rendered!");
#endif

    // Clip to the visible contents so we'll be faster
    m_currentRegularRenderJobsBatchRegion = IntRectRegion::intersectRegions(m_currentRegularRenderJobsBatchRegion, m_parent->visibleContentsRect());

    if (!m_currentRegularRenderJobsBatchRegion.isEmpty())
        m_parent->render(m_currentRegularRenderJobsBatchRegion.rects(), true /*Always content only*/);

#if DEBUG_RENDER_QUEUE
    // Stop the time measurement
    elapsed = WTF::currentTime() - time;
    IntRect extents = m_currentRegularRenderJobsBatchRegion.extents();
    int numberOfRects = m_currentRegularRenderJobsBatchRegion.rects().size();
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::renderAllCurrentRegularRenderJobs extents=(%d,%d %dx%d) numberOfRects=%d elapsed=%f",
                           extents.x(), extents.y(), extents.width(), extents.height(), numberOfRects, elapsed);
#endif

    // Clear the region and blit since this batch is now complete
    m_currentRegularRenderJobsBatch.clear();
    m_currentRegularRenderJobsBatchRegion = IntRectRegion();
    m_currentRegularRenderJobsBatchUnderPressure = false;
    IntRect updateRect = m_parent->blitVisibleContents();
    m_parent->invalidateWindow(updateRect);

    if (!regionNotRendered.isEmpty())
        m_parent->updateTilesForScrollOrNotRenderedRegion();
}

void RenderQueue::startRegularRenderJobBatchIfNeeded()
{
    if (!m_currentRegularRenderJobsBatch.isEmpty())
        return;

    // Decompose the current regular render job region into render rect pieces
    IntRectList regularRenderJobs = convertToRenderRectList(m_regularRenderJobsRegion.rects());

    // The current batch...
    m_currentRegularRenderJobsBatch = regularRenderJobs;

    // Create a region object that will be checked when adding new rects before
    // this batch has been completed
    m_currentRegularRenderJobsBatchRegion = m_regularRenderJobsRegion;

    // Clear the former region since it is now part of this batch
    m_regularRenderJobsRegion = IntRectRegion();

#if DEBUG_RENDER_QUEUE
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::startRegularRenderJobBatchIfNeeded batch size is %d!", m_currentRegularRenderJobsBatch.size());
#endif
}

void RenderQueue::renderVisibleZoomJob()
{
    ASSERT(!m_visibleZoomJobs.isEmpty());

#if DEBUG_RENDER_QUEUE
    // Start the time measurement...
    double time = WTF::currentTime();
#endif

    RenderRect* rect = &m_visibleZoomJobs.first();
    ASSERT(!rect->isCompleted());
    IntRect subRect = rect->rectForRendering();
    if (rect->isCompleted())
        m_visibleZoomJobs.remove(0);

    m_parent->render(subRect, true /*Always content only*/);

    // Record that it has now been rendered via a different type of job...
    m_regularRenderJobsNotRenderedRegion = IntRectRegion::subtractRegions(m_regularRenderJobsNotRenderedRegion, subRect);

#if DEBUG_RENDER_QUEUE
    // Stop the time measurement
    double elapsed = WTF::currentTime() - time;
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::renderVisibleZoomJob rect=(%d,%d %dx%d) elapsed=%f",
                           subRect.x(), subRect.y(), subRect.width(), subRect.height(), elapsed);
#endif

    if (!hasCurrentVisibleZoomJob() && !hasCurrentVisibleScrollJob())
        m_parent->stopRenderTimer();
}

void RenderQueue::renderVisibleScrollJob()
{
    ASSERT(!m_visibleScrollJobs.isEmpty());

#if DEBUG_RENDER_QUEUE || DEBUG_RENDER_QUEUE_SORT
    // Start the time measurement...
    double time = WTF::currentTime();
#endif

    quickSort(&m_visibleScrollJobs);

#if DEBUG_RENDER_QUEUE_SORT
    // Stop the time measurement
    double elapsed = WTF::currentTime() - time;
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::renderVisibleScrollJob sort elapsed=%f", elapsed);
#endif

    RenderRect rect = m_visibleScrollJobs.first();
    m_visibleScrollJobs.remove(0);

    ASSERT(!rect.isCompleted());
    IntRect subRect = rect.rectForRendering();
    if (rect.isCompleted()) {
        m_visibleScrollJobsCompleted.append(rect);
    } else {
        m_visibleScrollJobs.insert(0, rect);
    }

    m_parent->render(subRect, true /*Always content only*/);

    // Record that it has now been rendered via a different type of job...
    m_regularRenderJobsNotRenderedRegion = IntRectRegion::subtractRegions(m_regularRenderJobsNotRenderedRegion, subRect);

#if DEBUG_RENDER_QUEUE
    // Stop the time measurement
    double elapsed = WTF::currentTime() - time;
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::renderVisibleScrollJob rect=(%d,%d %dx%d) elapsed=%f",
                           subRect.x(), subRect.y(), subRect.width(), subRect.height(), elapsed);
#endif

    if (m_visibleScrollJobs.isEmpty())
        visibleScrollJobsCompleted(true /*shouldBlit*/);

    if (!hasCurrentVisibleZoomJob() && !hasCurrentVisibleScrollJob())
        m_parent->stopRenderTimer();
}

void RenderQueue::renderRegularRenderJob()
{
#if DEBUG_RENDER_QUEUE
    // Start the time measurement...
    double time = WTF::currentTime();
#endif

    ASSERT(!m_currentRegularRenderJobsBatch.isEmpty() || !m_regularRenderJobsRegion.isEmpty());

    startRegularRenderJobBatchIfNeeded();

    // Take the first job from the regular render job queue
    IntRect rect = m_currentRegularRenderJobsBatch.first();
    m_currentRegularRenderJobsBatch.remove(0);

    // Record any part of the region that doesn't intersect the current visible tiles rect
    IntRectRegion regionNotRendered = IntRectRegion::subtractRegions(rect, m_parent->visibleContentsRect());
    m_regularRenderJobsNotRenderedRegion = IntRectRegion::unionRegions(m_regularRenderJobsNotRenderedRegion, regionNotRendered);
#if DEBUG_RENDER_QUEUE
    if (!regionNotRendered.isEmpty())
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::renderRegularRenderJob rect (%d,%d %dx%d) not completely rendered!",
                               rect.x(), rect.y(), rect.width(), rect.height());
#endif

    // Clip to the visible tiles so we'll be faster
    rect.intersect(m_parent->visibleContentsRect());

    if (!rect.isEmpty())
        m_parent->render(rect, true /*Always content only*/);

#if DEBUG_RENDER_QUEUE
    // Stop the time measurement
    double elapsed = WTF::currentTime() - time;
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::renderRegularRenderJob rect=(%d,%d %dx%d) elapsed=%f",
                           rect.x(), rect.y(), rect.width(), rect.height(), elapsed);
#endif

    if (m_currentRegularRenderJobsBatch.isEmpty()) {
        // Clear the region and the and blit since this batch is now complete
        m_currentRegularRenderJobsBatchRegion = IntRectRegion();
        m_currentRegularRenderJobsBatchUnderPressure = false;
        IntRect updateRect = m_parent->blitVisibleContents();
        m_parent->invalidateWindow(updateRect);
    }

    // Make sure we didn't alter state of the queues that should have been empty
    // before this method was called
    ASSERT(m_visibleScrollJobs.isEmpty());

    if (!regionNotRendered.isEmpty())
        m_parent->updateTilesForScrollOrNotRenderedRegion();
}

void RenderQueue::renderNonVisibleScrollJob()
{
    ASSERT(!m_nonVisibleScrollJobs.isEmpty());

#if DEBUG_RENDER_QUEUE || DEBUG_RENDER_QUEUE_SORT
    // Start the time measurement...
    double time = WTF::currentTime();
#endif

    quickSort(&m_nonVisibleScrollJobs);

#if DEBUG_RENDER_QUEUE_SORT
    // Stop the time measurement
    double elapsed = WTF::currentTime() - time;
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::renderNonVisibleScrollJob sort elapsed=%f", elapsed);
#endif

    RenderRect rect = m_nonVisibleScrollJobs.first();
    m_nonVisibleScrollJobs.remove(0);

    ASSERT(!rect.isCompleted());
    IntRect subRect = rect.rectForRendering();
    if (rect.isCompleted()) {
        m_nonVisibleScrollJobsCompleted.append(rect);
    } else {
        m_nonVisibleScrollJobs.insert(0, rect);
    }

    m_parent->render(subRect, true /*Always content only*/);

    // Record that it has now been rendered via a different type of job...
    m_regularRenderJobsNotRenderedRegion = IntRectRegion::subtractRegions(m_regularRenderJobsNotRenderedRegion, subRect);

    // Make sure we didn't alter state of the queues that should have been empty
    // before this method was called
    ASSERT(m_visibleScrollJobs.isEmpty());

#if DEBUG_RENDER_QUEUE
    // Stop the time measurement
    double elapsed = WTF::currentTime() - time;
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "RenderQueue::renderNonVisibleScrollJob rect=(%d,%d %dx%d) elapsed=%f",
                           subRect.x(), subRect.y(), subRect.width(), subRect.height(), elapsed);
#endif

    if (m_nonVisibleScrollJobs.isEmpty())
        nonVisibleScrollJobsCompleted();
}

void RenderQueue::visibleScrollJobsCompleted(bool shouldBlit)
{
    // Now blit to the screen if we are done and get rid of the completed list!
    ASSERT(m_visibleScrollJobs.isEmpty());
    m_visibleScrollJobsCompleted.clear();
    if (shouldBlit) {
        IntRect updateRect = m_parent->blitVisibleContents();
        m_parent->invalidateWindow(updateRect);
    }
}

void RenderQueue::nonVisibleScrollJobsCompleted()
{
    // Get rid of the completed list!
    ASSERT(m_nonVisibleScrollJobs.isEmpty());
    m_nonVisibleScrollJobsCompleted.clear();
}

}
}
