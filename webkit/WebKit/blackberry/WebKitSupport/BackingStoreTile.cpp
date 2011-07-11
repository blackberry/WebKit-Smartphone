/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "BackingStoreTile.h"

#include "EGLDisplayOpenVG.h"
#include "EGLUtils.h"
#include "GraphicsContext.h"
#include "OlympiaPlatformGraphics.h"
#include "OlympiaPlatformPrimitives.h"
#include "SurfacePool.h"
#include "VGUtils.h"

using namespace WebCore;

namespace Olympia {
namespace WebKit {

BackingStoreTile::BackingStoreTile(const WebCore::IntSize& size, bool checkered)
    : m_size(size)
    , m_buffer(0)
    , m_checkered(checkered)
    , m_committed(false)
    , m_backgroundPainted(false)
    , m_horizontalShift(0)
    , m_verticalShift(0)
{
    if (m_checkered)
        paintBackground();
}

BackingStoreTile::~BackingStoreTile()
{
    Olympia::Platform::Graphics::destroyBuffer(m_buffer);
}

Olympia::Platform::Graphics::Buffer* BackingStoreTile::buffer() const
{
    if (!m_buffer)
        m_buffer = Olympia::Platform::Graphics::createBuffer(m_size, Olympia::Platform::Graphics::TileBuffer);

    return m_buffer;
}

void BackingStoreTile::reset()
{
    setCommitted(false);
    clearRenderedRegion();
    clearShift();
    paintBackground();
}

void BackingStoreTile::paintBackground()
{
    m_backgroundPainted = true;

    if (!m_checkered) {
        Olympia::Platform::Graphics::clearBufferWhite(buffer());
    } else {
        // Draw checkerboard pattern on the tile rendering surface
        // and copy it over to our buffer.
        SurfaceOpenVG* surface = SurfacePool::globalSurfacePool()->tileRenderingSurface();
        surface->makeCurrent();

        VGint scissoringEnabled = vgGeti(VG_SCISSORING);
        vgSeti(VG_SCISSORING, VG_FALSE);

        VGfloat currentClearColor[4];
        vgGetfv(VG_CLEAR_COLOR, 4, currentClearColor);
        ASSERT_VG_NO_ERROR();

        // checker specs are from UXD
        static const Color darkCheckerColor(0xe8, 0xee, 0xf7);
        static const Color lightCheckerColor(0xfb, 0xfd, 0xff);
        static VGfloat darkCheckerVG[4];
        static VGfloat lightCheckerVG[4];
        darkCheckerColor.getRGBA(darkCheckerVG[0], darkCheckerVG[1], darkCheckerVG[2], darkCheckerVG[3]);
        lightCheckerColor.getRGBA(lightCheckerVG[0], lightCheckerVG[1], lightCheckerVG[2], lightCheckerVG[3]);

        static const unsigned checkerSize = 20;

        vgSetfv(VG_CLEAR_COLOR, 4, lightCheckerVG);
        vgClear(checkerSize, 0, checkerSize, checkerSize);
        vgClear(0, checkerSize, checkerSize, checkerSize);

        vgSetfv(VG_CLEAR_COLOR, 4, darkCheckerVG);
        vgClear(0, 0, checkerSize, checkerSize);
        vgClear(checkerSize, checkerSize, checkerSize, checkerSize);
        ASSERT_VG_NO_ERROR();

        int width = checkerSize * 2;
        int height = width;

        while (width < m_size.width() || height < m_size.height()) {
            if (width < m_size.width()) {
                vgCopyPixels(width, 0, 0, 0, width, height);
                width *= 2;
            }
            if (height < m_size.height()) {
                vgCopyPixels(0, height, 0, 0, width, height);
                height *= 2;
            }
        }
        ASSERT_VG_NO_ERROR();

        vgSeti(VG_SCISSORING, scissoringEnabled);
        ASSERT_VG_NO_ERROR();

        Olympia::Platform::Graphics::blitToBufferFromDrawingSurface(
            buffer(), Olympia::Platform::IntPoint(0, 0),
            Olympia::Platform::IntRect(0, 0, m_size.width(), m_size.height()));
        ASSERT_VG_NO_ERROR();
    }
}

}
}
