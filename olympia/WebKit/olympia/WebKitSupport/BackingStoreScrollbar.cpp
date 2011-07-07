/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "BackingStoreScrollbar.h"

#include "EGLDisplayOpenVG.h"
#include "EGLUtils.h"
#include "PainterOpenVG.h"
#include "SurfacePool.h"
#include "VGUtils.h"

#include <egl.h>
#include <openvg.h>

using namespace WebCore;

namespace Olympia {
namespace WebKit {

BackingStoreScrollbar::BackingStoreScrollbar(const WebCore::IntSize& size)
    : m_size(size)
{
    m_image = new unsigned char[size.height() * imageStride()];
    m_alphaImage = new unsigned char[size.height() * alphaImageStride()];
}

BackingStoreScrollbar::~BackingStoreScrollbar()
{
    delete[] m_image;
    delete[] m_alphaImage;
}

void BackingStoreScrollbar::repaint(const WebCore::IntRect& hotRect, bool vertical)
{
    SurfaceOpenVG* surface = SurfacePool::globalSurfacePool()->tileRenderingSurface();
    const IntSize surfaceSize(surface->width(), surface->height());
    PainterOpenVG painter(surface);

    // If the rendering surface is smaller than the scrollbar
    // (e.g. when the screen is rotated), paint it multiple times.
    int lastXTile = (m_size.width() - 1) / surfaceSize.width();
    int lastYTile = (m_size.height() - 1) / surfaceSize.height();

    FloatRect innerRect = hotRect;
    innerRect.move(0.5, 0.5); // draw it pixel-perfect with a 1.0 wide stroke
    int radius = vertical ? hotRect.width() / 2 : hotRect.height() / 2;
    if (vertical)
        innerRect.inflateY(-radius);
    else
        innerRect.inflateX(-radius);

    Path path;

    if (vertical) {
        path.moveTo(FloatPoint(innerRect.x(), innerRect.y()));
        path.addArc(FloatPoint(innerRect.x() + radius, innerRect.y()), radius, piFloat, 0, false);
        path.addLineTo(FloatPoint(innerRect.right(), innerRect.bottom()));
        path.addArc(FloatPoint(innerRect.x() + radius, innerRect.bottom()), radius, 0, piFloat, false);
        path.closeSubpath();
    } else {
        path.moveTo(FloatPoint(innerRect.right(), innerRect.y()));
        path.addArc(FloatPoint(innerRect.right(), innerRect.y() + radius), radius, piFloat + piFloat / 2, piFloat / 2, false);
        path.addLineTo(FloatPoint(innerRect.x(), innerRect.bottom()));
        path.addArc(FloatPoint(innerRect.x(), innerRect.y() + radius), radius, piFloat / 2, piFloat + piFloat / 2, false);
        path.closeSubpath();
    }

    for (int i = 0; i <= lastXTile; ++i) {
        for (int j = 0; j <= lastYTile; ++j) {
            const int dstX = i * surfaceSize.width();
            const int dstY = j * surfaceSize.height();
            const int dstRight = (i == lastXTile) ? m_size.width() : (i + 1) * surfaceSize.width();
            const int dstBottom = (j == lastYTile) ? m_size.height() : (j + 1) * surfaceSize.height();

            painter.translate(rect().x(), rect().y());

            if (dstX || dstY)
                painter.translate(-dstX, -dstY);

            // Paint the actual scrollbar.
            painter.setFillColor(Color(255, 255, 255));
            painter.drawRect(IntRect(IntPoint(0, 0), rect().size()), VG_FILL_PATH);
            painter.setFillColor(Color(173, 176, 178));
            painter.drawPath(path, VG_FILL_PATH, WebCore::RULE_NONZERO);

            surface->makeCurrent();
            unsigned char* dstBufferStart = m_image + (dstY * imageStride()) + (dstX * imageBytesPerPixel());
            vgReadPixels(dstBufferStart, imageStride(), VG_sRGB_565,
                0, 0, dstRight - dstX, dstBottom - dstY);
            ASSERT_VG_NO_ERROR();

            // Paint the alpha channel for the actual scrollbar.
            painter.setCompositeOperation(CompositeClear);
            painter.setFillColor(Color(0, 0, 0));
            painter.drawRect(IntRect(IntPoint(0, 0), rect().size()), VG_FILL_PATH);
            painter.setCompositeOperation(CompositeSourceOver);
            painter.setFillColor(Color(255, 255, 255));
            painter.drawPath(path, VG_FILL_PATH, WebCore::RULE_NONZERO);

            surface->makeCurrent();
            // FIXME: We need VG_A_4 until proper alpha blending is available.
            //   When we get 8-bit formats, make sure to adapt both the format
            //   and remove the division by two when determining the dstAlphaBufferStart.
            unsigned char* dstAlphaBufferStart = m_alphaImage + (dstY * alphaImageStride()) + dstX / 2;
            vgReadPixels(dstAlphaBufferStart, alphaImageStride(), VG_A_4,
                0, 0, dstRight - dstX, dstBottom - dstY);
            ASSERT_VG_NO_ERROR();

            if (dstX || dstY)
                painter.translate(dstX, dstY);
        }
    }
}

}
}
