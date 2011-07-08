/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "Pattern.h"

#include "EGLDisplayOpenVG.h"
#include "FloatPoint.h"
#include "FloatRect.h"
#include "FloatSize.h"
#include "GraphicsContext.h"
#include "ImageBuffer.h"
#include "IntRect.h"
#include "IntSize.h"
#include "PainterOpenVG.h"
#include "PatternOpenVG.h"
#include "SurfaceOpenVG.h"
#include "TiledImageOpenVG.h"

#include <wtf/OwnPtr.h>

namespace WebCore {

PatternOpenVG::PatternOpenVG()
{
}

PatternOpenVG::PatternOpenVG(const PatternOpenVG& other)
    : m_image(other.m_image)
    , m_imageTile(other.m_imageTile)
    , m_transformation(other.m_transformation)
    , m_scaleX(other.m_scaleX)
    , m_scaleY(other.m_scaleY)
{
}

PatternOpenVG::PatternOpenVG(PassRefPtr<TiledImageOpenVG> tiledImage)
{
    setImage(tiledImage, FloatRect(FloatPoint(0, 0), FloatSize(tiledImage->size())));
}

PatternOpenVG::PatternOpenVG(PassRefPtr<TiledImageOpenVG> tiledImage, const FloatRect& srcRect)
{
    setImage(tiledImage, srcRect);
}

bool PatternOpenVG::isValid() const
{
    return static_cast<bool>(m_image.get());
}

void PatternOpenVG::setImage(PassRefPtr<TiledImageOpenVG> image, const FloatRect& srcRect)
{
    m_scaleX = 1.0;
    m_scaleY = 1.0;

    bool isIntRect = (fmod(srcRect.x() + 0.01, 1.0) < 0.02) && (fmod(srcRect.y() + 0.01, 1.0) < 0.02)
        && (fmod(srcRect.right() + 0.01, 1.0) < 0.02) && (fmod(srcRect.bottom() + 0.01, 1.0) < 0.02);

    if (isIntRect) {
        // Try to reuse existing VGImages if possible.
        const IntRect roundedSrc(srcRect.x() + 0.01, srcRect.y() + 0.01,
            srcRect.width() + 0.02, srcRect.height() + 0.02);

        if (image->numTiles() == 1 && IntRect(IntPoint(0, 0), image->size()) == roundedSrc) {
            // Common case #1.
            m_image = image;
            m_imageTile = IntPoint(0, 0);
            return;
        }
        const IntRect tilesInRect = image->tilesInRect(FloatRect(roundedSrc));

        if (tilesInRect.width() == 1 && tilesInRect.height() == 1
            && image->tileRect(tilesInRect.x(), tilesInRect.y()) == roundedSrc) {
            m_image = image;
            m_imageTile = tilesInRect.location();
            return;
        }

        // Image can't be reused directly, copy pixels into a new image instead.
#if PLATFORM(EGL)
        EGLDisplayOpenVG::current()->sharedPlatformSurface()->makeResourceCreationContextCurrent();
#endif
        const IntSize vgMaxImageSize(vgGeti(VG_MAX_IMAGE_WIDTH), vgGeti(VG_MAX_IMAGE_HEIGHT));
        ASSERT_VG_NO_ERROR();

        if (vgMaxImageSize.width() <= roundedSrc.width() && vgMaxImageSize.height() <= roundedSrc.height()) {
            VGImageFormat format = (VGImageFormat) vgGetParameteri(image->tile(0, 0), VG_IMAGE_FORMAT);
            ASSERT_VG_NO_ERROR();

            VGImage srcImage = vgCreateImage(format,
                roundedSrc.width(), roundedSrc.height(), VG_IMAGE_QUALITY_FASTER);
            ASSERT_VG_NO_ERROR();

            if (tilesInRect.width() == 1 && tilesInRect.height() == 1) {
                // Common case #2.
                VGImage tile = image->tile(tilesInRect.x(), tilesInRect.y());
                IntRect tileRect = image->tileRect(tilesInRect.x(), tilesInRect.y());

                vgCopyImage(srcImage, 0, 0, tile,
                    roundedSrc.x() - tileRect.x(), roundedSrc.y() - tileRect.y(),
                    roundedSrc.width(), roundedSrc.height(), VG_FALSE /*dither*/);
                ASSERT_VG_NO_ERROR();
            } else {
                for (int yIndex = tilesInRect.y(); yIndex < tilesInRect.bottom(); ++yIndex) {
                    for (int xIndex = tilesInRect.x(); xIndex < tilesInRect.right(); ++xIndex) {
                        VGImage tile = image->tile(xIndex, yIndex);
                        IntRect tileRect = image->tileRect(xIndex, yIndex);
                        IntRect extractedRect = intersection(roundedSrc, tileRect);

                        vgCopyImage(srcImage,
                            extractedRect.x() - roundedSrc.x(), extractedRect.x() - roundedSrc.x(),
                            tile, roundedSrc.x() - tileRect.x(), roundedSrc.y() - tileRect.y(),
                            extractedRect.width(), extractedRect.height(), VG_FALSE /*dither*/);
                        ASSERT_VG_NO_ERROR();
                    }
                }
            }

            m_image = adoptRef(new TiledImageOpenVG(roundedSrc.size(), vgMaxImageSize));
            m_image->setTile(0, 0, srcImage);
            m_imageTile = IntPoint(0, 0);
            return;
        }
    }

    // Existing image/pixels could not be reused. Use (down)scaling to make it work.
#if PLATFORM(EGL)
    EGLDisplayOpenVG::current()->sharedPlatformSurface()->makeResourceCreationContextCurrent();
#endif
    const IntSize vgMaxImageSize(vgGeti(VG_MAX_IMAGE_WIDTH), vgGeti(VG_MAX_IMAGE_HEIGHT));
    ASSERT_VG_NO_ERROR();

    const IntSize enclosedSrcSize(ceilf(srcRect.width()), ceilf(srcRect.height()));
    const IntSize adaptedSrcSize = enclosedSrcSize.shrunkTo(vgMaxImageSize);

    m_scaleX = srcRect.width() / (float) adaptedSrcSize.width();
    m_scaleY = srcRect.height() / (float) adaptedSrcSize.height();

    OwnPtr<ImageBuffer> imageBuffer = ImageBuffer::create(adaptedSrcSize);

    // PainterOpenVG::drawImage() can do the downscaling for us.
    if (imageBuffer) {
        GraphicsContext* bufferContext = imageBuffer->context();
        PainterOpenVG* bufferPainter = bufferContext->platformContext()->activePainter();

        bufferPainter->drawImage(image.get(),
            FloatRect(FloatPoint(0, 0), adaptedSrcSize), srcRect);

        m_image = bufferPainter->asNewNativeImage(
            IntRect(IntPoint(0, 0), adaptedSrcSize), VG_sARGB_8888_PRE);
        m_imageTile = IntPoint(0, 0);
    }
}

AffineTransform PatternOpenVG::effectiveTransformation() const
{
    if (m_scaleX == 1.0 && m_scaleY == 1.0)
        return m_transformation;

    AffineTransform scaledTransformation(m_transformation);
    scaledTransformation.scaleNonUniform(m_scaleX, m_scaleY);
    return scaledTransformation;
}

VGImage PatternOpenVG::vgImage() const
{
    return m_image->tile(m_imageTile.x(), m_imageTile.y());
}

PatternOpenVG* Pattern::createPlatformPattern(const AffineTransform&) const
{
    RefPtr<TiledImageOpenVG> patternImage = tileImage()->nativeImageForCurrentFrame();
    if (!patternImage.get())
        return new PatternOpenVG;

    PatternOpenVG* pattern = new PatternOpenVG(patternImage);
    pattern->setTransformation(m_patternSpaceTransformation);
    return pattern;
}

}
