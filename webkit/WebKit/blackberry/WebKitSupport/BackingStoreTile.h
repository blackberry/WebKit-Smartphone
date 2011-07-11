/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef BackingStoreTile_h
#define BackingStoreTile_h

#include "IntRect.h"
#include "IntRectRegion.h"
#include "IntSize.h"
#include "SurfaceOpenVG.h"

#include <egl.h>
#include <openvg.h>

#include <wtf/PassOwnPtr.h>
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>

#define ALIGN4(x) ((x + 3) & ~3)

namespace Olympia {
    namespace Platform {
    namespace Graphics {
    struct Buffer;
    }
    }

    namespace WebKit {
        class BackingStoreTile : public RefCounted<BackingStoreTile> {
        public:
            static PassRefPtr<BackingStoreTile> create(const WebCore::IntSize& size)
            {
                return adoptRef(new BackingStoreTile(size, false /*checkered*/ ));
            }
            static PassRefPtr<BackingStoreTile> createCheckered(const WebCore::IntSize& size)
            {
                return adoptRef(new BackingStoreTile(size, true /*checkered*/));
            }

            ~BackingStoreTile();

            WebCore::IntSize size() const { return m_size; }
            WebCore::IntRect rect() const { return WebCore::IntRect(WebCore::IntPoint(0,0), m_size); }
            Olympia::Platform::Graphics::Buffer* buffer() const;

            void reset();
            bool backgroundPainted() const { return m_backgroundPainted; }
            void paintBackground();

            bool isCommitted() const { return m_committed; }
            void setCommitted(bool committed) { m_committed = committed; }

            bool isRendered() const { return !m_renderedRegion.isEmpty(); }
            bool isCompletelyRendered() const { return m_renderedRegion.isRectInRegion(rect()) == WebCore::IntRectRegion::ContainedInRegion; }
            void clearRenderedRegion(const WebCore::IntRectRegion& region)
            {
                m_renderedRegion = WebCore::IntRectRegion::subtractRegions(m_renderedRegion, region);
            }
            void clearRenderedRegion() { m_renderedRegion = WebCore::IntRectRegion(); }
            void addRenderedRegion(const WebCore::IntRectRegion& region)
            {
                m_renderedRegion = WebCore::IntRectRegion::unionRegions(region, m_renderedRegion);
            }
            WebCore::IntRectRegion renderedRegion() const { return m_renderedRegion; }
            WebCore::IntRectRegion notRenderedRegion() const { return WebCore::IntRectRegion::subtractRegions(rect(), renderedRegion()); }

            void clearShift() { m_horizontalShift = 0; m_verticalShift = 0; }
            bool isShifted() const { return m_horizontalShift != 0 || m_verticalShift != 0; }
            void shiftLeft() { m_horizontalShift--; }
            void shiftRight() { m_horizontalShift++; }
            void shiftUp() { m_verticalShift--; }
            void shiftDown() { m_verticalShift++; }
            int horizontalShift() const { return m_horizontalShift; }
            void setHorizontalShift(int shift) { m_horizontalShift = shift; }
            int verticalShift() const { return m_verticalShift; }
            void setVerticalShift(int shift) { m_verticalShift = shift; }

        private:
            BackingStoreTile(const WebCore::IntSize&, bool checkered);

            WebCore::IntSize m_size;
            WebCore::IntRectRegion m_renderedRegion;
            mutable Olympia::Platform::Graphics::Buffer* m_buffer;
            bool m_checkered;
            bool m_committed;
            bool m_backgroundPainted;
            int m_horizontalShift;
            int m_verticalShift;
        };
    }
}

#endif // BackingStoreTile_h
