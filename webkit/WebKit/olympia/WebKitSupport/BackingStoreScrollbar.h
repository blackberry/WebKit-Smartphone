/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef BackingStoreScrollbar_h
#define BackingStoreScrollbar_h

#include "IntRect.h"
#include "IntSize.h"
#include "SurfaceOpenVG.h"

#include <wtf/PassOwnPtr.h>
#include <wtf/RefCounted.h>

#define ALIGN4(x) ((x + 3) & ~3)

namespace Olympia {
    namespace WebKit {
        class BackingStoreScrollbar : public RefCounted<BackingStoreScrollbar> {
        public:
            static PassOwnPtr<BackingStoreScrollbar> create(const WebCore::IntSize& size)
            {
                return new BackingStoreScrollbar(size);
            }
            ~BackingStoreScrollbar();

            WebCore::IntSize size() const { return m_size; }
            WebCore::IntRect rect() const { return WebCore::IntRect(WebCore::IntPoint(0,0), m_size); }
            WebCore::SurfaceOpenVG* surface() const { return m_surface; }
            unsigned char* image() const { return m_image; }
            int imageStride() const { return ALIGN4(m_size.width() * imageBytesPerPixel()); }
            int imageBytesPerPixel() const { return 2; }
            unsigned char* alphaImage() const { return m_alphaImage; }
            int alphaImageStride() const { return ALIGN4(m_size.width()); }

            /* Repaints the background. */
            void repaint(const WebCore::IntRect& hotRect, bool vertical);

        private:
            BackingStoreScrollbar(const WebCore::IntSize&);

            WebCore::IntSize m_size;
            WebCore::SurfaceOpenVG* m_surface;
            unsigned char* m_image;
            unsigned char* m_alphaImage;
        };
    }
}

#endif // BackingStoreScrollbar_h
