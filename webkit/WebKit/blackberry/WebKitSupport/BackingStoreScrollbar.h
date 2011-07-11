/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef BackingStoreScrollbar_h
#define BackingStoreScrollbar_h

#include "IntRect.h"
#include "IntSize.h"
#include "SurfaceOpenVG.h"

#include <wtf/PassOwnPtr.h>

#define ALIGN4(x) ((x + 3) & ~3)

namespace Olympia {
    namespace Platform {
    namespace Graphics {
    struct Buffer;
    }
    }

    namespace WebKit {
        class BackingStoreScrollbar {
        public:
            static PassOwnPtr<BackingStoreScrollbar> create(const WebCore::IntSize& size)
            {
                return new BackingStoreScrollbar(size);
            }
            ~BackingStoreScrollbar();

            WebCore::IntSize size() const { return m_size; }
            WebCore::IntRect rect() const { return WebCore::IntRect(WebCore::IntPoint(0,0), m_size); }
            Olympia::Platform::Graphics::Buffer* buffer() const { return m_buffer; }
            Olympia::Platform::Graphics::Buffer* alphaBuffer() const { return m_alphaBuffer; }

            /* Repaints the background. */
            void repaint(const WebCore::IntRect& hotRect, bool vertical);

        private:
            BackingStoreScrollbar(const WebCore::IntSize&);

            WebCore::IntSize m_size;
            mutable Olympia::Platform::Graphics::Buffer* m_buffer;
            mutable Olympia::Platform::Graphics::Buffer* m_alphaBuffer;
        };
    }
}

#endif // BackingStoreScrollbar_h
