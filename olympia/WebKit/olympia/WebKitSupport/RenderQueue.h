/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef RenderQueue_h
#define RenderQueue_h

#include "IntPoint.h"
#include "IntRect.h"
#include "IntRectRegion.h"
#include "IntSize.h"
#include "TileIndex.h"

#include <wtf/Vector.h>

namespace WebCore {
    typedef WTF::Vector<IntRect> IntRectList;
}

namespace Olympia {
    namespace WebKit {
        class BackingStorePrivate;

        enum SortDirection {
            LeftToRight,
            RightToLeft,
            TopToBottom,
            BottomToTop
        };

        class RenderRect : public WebCore::IntRect {
        public:
            RenderRect() { }
            RenderRect(const WebCore::IntPoint& location, const WebCore::IntSize& size, int splittingFactor);
            RenderRect(int x, int y, int width, int height, int splittingFactor);
            RenderRect(const WebCore::IntRect& rect);

            void split();

            WebCore::IntRect rectForRendering();

            bool isCompleted() const { return m_subRects.isEmpty(); }

            WebCore::IntRectList subRects() const { return m_subRects; }
            void updateSortDirection(SortDirection primary, SortDirection secondary);

        private:
            void quickSort(WebCore::IntRectList*);
            int m_splittingFactor;
            WebCore::IntRectList m_subRects;
            SortDirection m_primarySortDirection;
            SortDirection m_secondarySortDirection;
        };

        typedef WTF::Vector<RenderRect> RenderRectList;

        class RenderQueue {
        public:
            enum JobType { VisibleZoom, VisibleScroll, RegularRender, NonVisibleScroll };
            RenderQueue(BackingStorePrivate* parent);
            ~RenderQueue();

            int splittingFactor(const WebCore::IntRect&) const;
            RenderRect convertToRenderRect(const WebCore::IntRect& rect) const;
            WebCore::IntRectList convertToRenderRectList(const WebCore::IntRectList&) const;
            WebCore::IntRectList convertToRenderRectList(const WebCore::IntRect&) const;

            bool isEmpty(bool shouldPerformRegularRenderJobs = true) const;

            bool hasCurrentRegularRenderJob() const;
            bool hasCurrentVisibleZoomJob() const;
            bool hasCurrentVisibleScrollJob() const;
            bool isCurrentVisibleScrollJob(const WebCore::IntRect& rect) const;
            bool isCurrentVisibleScrollJobCompleted(const WebCore::IntRect& rect) const;
            bool isCurrentRegularRenderJob(const WebCore::IntRect& rect) const;

            bool currentRegularRenderJobBatchUnderPressure() const;
            void setCurrentRegularRenderJobBatchUnderPressure(bool b);

            void eventQueueCycled();

            void addToQueue(JobType type, const WebCore::IntRect&);
            void addToQueue(JobType type, const WebCore::IntRectList&);

            void updateSortDirection(int lastDeltaX, int lastDeltaY);
            void visibleContentChanged(const WebCore::IntRect&);
            void clear(const WebCore::IntRectRegion&, bool clearRegularRenderJobs);
            void clear(const WebCore::IntRect&, bool clearRegularRenderJobs);
            void clear();
            void clearVisibleZoom();
            bool regularRenderJobsPreviouslyAttemptedButNotRendered(const WebCore::IntRect& rect);
            WebCore::IntRectRegion regularRenderJobsNotRenderedRegion() const { return m_regularRenderJobsNotRenderedRegion; }

            void render(bool shouldPerformRegularRenderJobs = true);
            void renderAllCurrentRegularRenderJobs();

        private:
            void startRegularRenderJobBatchIfNeeded();

            /* Render an item from the queue */
            void renderVisibleZoomJob();
            void renderVisibleScrollJob();
            void renderRegularRenderJob();
            void renderNonVisibleScrollJob();

            /* Methods to handle a completed set of scroll jobs */
            void visibleScrollJobsCompleted(bool shouldBlit);
            void nonVisibleScrollJobsCompleted();

            /* Internal method to add to the various queues */
            void addToRegularQueue(const WebCore::IntRect&);
            void addToScrollZoomQueue(const RenderRect&, RenderRectList* queue);
            void quickSort(RenderRectList*);

            // The splitting factor for render rects
            int splittingFactor() const;

            BackingStorePrivate* m_parent;

            // The highest priority queue
            RenderRectList m_visibleZoomJobs;
            RenderRectList m_visibleScrollJobs;
            RenderRectList m_visibleScrollJobsCompleted;
            // The lowest priority queue
            RenderRectList m_nonVisibleScrollJobs;
            RenderRectList m_nonVisibleScrollJobsCompleted;
            // The regular render jobs are in the middle
            WebCore::IntRectRegion m_regularRenderJobsRegion;
            WebCore::IntRectList m_currentRegularRenderJobsBatch;
            WebCore::IntRectRegion m_currentRegularRenderJobsBatchRegion;
            bool m_rectsAddedToRegularRenderJobsInCurrentCycle;
            bool m_currentRegularRenderJobsBatchUnderPressure;

            // Holds the region of the page that we attempt to render, but the
            // backingstore was not in the right place at the time.  This will
            // be checked before we try to restore a tile to it's last rendered
            // place
            WebCore::IntRectRegion m_regularRenderJobsNotRenderedRegion;

            SortDirection m_primarySortDirection;
            SortDirection m_secondarySortDirection;
        };
    }
}

#endif // RenderQueue_h
