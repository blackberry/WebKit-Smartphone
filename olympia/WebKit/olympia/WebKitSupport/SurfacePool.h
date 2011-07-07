/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef SurfacePool_h
#define SurfacePool_h

#include "BackingStoreTile.h"
#include "BackingStoreScrollbar.h"
#include "IntSize.h"
#include "SurfaceOpenVG.h"

#include <wtf/Vector.h>

namespace Olympia {
    namespace WebKit {
        class SurfacePool {
        public:
            static SurfacePool* globalSurfacePool();

            void initialize(const WebCore::IntSize&);

            int isEmpty() const { return m_tilePool.isEmpty(); }
            int size() const { return m_tilePool.size(); }

            typedef WTF::Vector<WTF::RefPtr<BackingStoreTile> > TileList;
            const TileList tileList() const { return m_tilePool; }

            WebCore::SurfaceOpenVG* tileRenderingSurface() const { return m_tileRenderingSurface.get(); }
            BackingStoreTile* checkeredTile() const { return m_checkeredTile.get(); }
            BackingStoreTile* visibleTileBuffer() const { return m_visibleTileBuffer.get(); }

            BackingStoreScrollbar* horizontalScrollbar() const { return m_horizontalScrollbar.get(); }
            BackingStoreScrollbar* verticalScrollbar() const { return m_verticalScrollbar.get(); }

            void initializeVisibleTileBuffer(const WebCore::IntSize&);
            void initializeScrollbars(const WebCore::IntSize&, const WebCore::IntSize&);

        private:
            SurfacePool();
            ~SurfacePool();

            TileList m_tilePool;
            OwnPtr<WebCore::SurfaceOpenVG> m_tileRenderingSurface;
            OwnPtr<BackingStoreTile> m_checkeredTile;
            OwnPtr<BackingStoreTile> m_visibleTileBuffer;
            OwnPtr<BackingStoreScrollbar> m_horizontalScrollbar;
            OwnPtr<BackingStoreScrollbar> m_verticalScrollbar;
        };
    }
}

#endif // SurfacePool_h
