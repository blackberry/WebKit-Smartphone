/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "SurfacePool.h"

#include "EGLDisplayOpenVG.h"
#include "EGLUtils.h"
#include "OlympiaPlatformMisc.h"
#include "OlympiaPlatformSettings.h"

using namespace WebCore;

namespace Olympia {
namespace WebKit {

SurfacePool* SurfacePool::globalSurfacePool()
{
    static SurfacePool* s_instance = 0;
    if (!s_instance)
        s_instance = new SurfacePool;
    return s_instance;
}

SurfacePool::SurfacePool()
{
}

SurfacePool::~SurfacePool()
{
}

void SurfacePool::initialize(const WebCore::IntSize& tileSize)
{
    EGLint      numConfigs;
    EGLConfig   config;

    EGLint attribListConfig[] = {
        EGL_RED_SIZE,           8,
        EGL_GREEN_SIZE,         8,
        EGL_BLUE_SIZE,          8,
        EGL_ALPHA_SIZE,         8,
        EGL_ALPHA_MASK_SIZE,    1,
        EGL_SURFACE_TYPE,       EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE,    EGL_OPENVG_BIT,
        EGL_NONE
    };
    eglChooseConfig(EGLDisplayOpenVG::current()->display(), attribListConfig, &config, 1, &numConfigs);
    ASSERT_EGL_NO_ERROR();
    ASSERT(numConfigs == 1);

    m_tileRenderingSurface.set(new SurfaceOpenVG(tileSize, EGLDisplayOpenVG::current()->display(), &config));

    unsigned tileNumber = Olympia::Platform::Settings::get()->numberOfBackingStoreTiles();

    for (size_t i = 0; i < tileNumber; ++i) {
        RefPtr<BackingStoreTile> tile = BackingStoreTile::create(tileSize);
        m_tilePool.append(tile);
    }

    m_checkeredTile = BackingStoreTile::createCheckered(tileSize);
}

void SurfacePool::initializeVisibleTileBuffer(const WebCore::IntSize& visibleSize)
{
    if (!m_visibleTileBuffer || m_visibleTileBuffer->size() != visibleSize) {
        m_visibleTileBuffer.clear();
        m_visibleTileBuffer = BackingStoreTile::createCheckered(visibleSize);
    }
}

void SurfacePool::initializeScrollbars(const WebCore::IntSize& horizontalSize, const WebCore::IntSize& verticalSize)
{
    if (!m_horizontalScrollbar || m_horizontalScrollbar->size() != horizontalSize) {
        m_horizontalScrollbar.clear();

        if (!horizontalSize.isEmpty() && horizontalSize.width() > 6)
            m_horizontalScrollbar = BackingStoreScrollbar::create(horizontalSize);
    }

    if (!m_verticalScrollbar || m_verticalScrollbar->size() != verticalSize) {
        m_verticalScrollbar.clear();

        if (!verticalSize.isEmpty() && verticalSize.height() > 6)
            m_verticalScrollbar = BackingStoreScrollbar::create(verticalSize);
    }
}

}
}
