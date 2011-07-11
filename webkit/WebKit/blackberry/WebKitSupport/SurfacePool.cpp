/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "SurfacePool.h"

#include "EGLDisplayOpenVG.h"
#include "EGLUtils.h"
#include "OlympiaPlatformGraphics.h"
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

void SurfacePool::initialize(const WebCore::IntSize& tileSize)
{
    EGLDisplay display = EGLDisplayOpenVG::current()->display();
    m_tileRenderingSurface.set(SurfaceOpenVG::adoptExistingSurface(
        display, Olympia::Platform::Graphics::drawingSurface(), SurfaceOpenVG::DontTakeSurfaceOwnership,
        Olympia::Platform::Graphics::drawingSurfaceType()));

    m_tileRenderingSurface->setApplyFlipTransformationOnPainterCreation(
        Olympia::Platform::Graphics::isFlipTransformationRequiredForDrawingSurface());

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
