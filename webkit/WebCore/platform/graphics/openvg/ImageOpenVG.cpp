/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
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
#include "Image.h"

#include "AffineTransform.h"
#include "BitmapImage.h"
#include "FloatRect.h"
#include "GraphicsContext.h"
#include "ImageBuffer.h"
#include "ImageDecoder.h"
#include "ImageObserver.h"
#include "IntSize.h"
#include "NotImplemented.h"
#include "PainterOpenVG.h"
#include "PatternOpenVG.h"
#include "PlatformString.h"
#include "SurfaceOpenVG.h"
#include "TiledImageOpenVG.h"
#include "VGUtils.h"

#include <wtf/MathExtras.h>

namespace WebCore {

bool FrameData::clear(bool clearMetadata)
{
    if (clearMetadata)
        m_haveMetadata = false;

    if (m_frame) {
        m_frame.clear();
        return true;
    }
    return false;
}

BitmapImage::BitmapImage(NativeImagePtr tiledImage, ImageObserver* observer)
    : Image(observer)
    , m_size(tiledImage->size())
    , m_currentFrame(0)
    , m_frames(1)
    , m_frameTimer(0)
    , m_repetitionCount(cAnimationNone)
    , m_repetitionCountStatus(Unknown)
    , m_repetitionsComplete(0)
    , m_desiredFrameStartTime(0)
    , m_isSolidColor(false)
    , m_checkedForSolidColor(false)
    , m_animationFinished(false)
    , m_allDataReceived(false)
    , m_haveSize(true)
    , m_sizeAvailable(true)
    , m_hasUniformFrameSize(true)
    , m_haveFrameCount(true)
    , m_frameCount(1)
{
    initPlatformData();

    ASSERT(m_size.width() > 0);
    ASSERT(m_size.height() > 0);

    m_decodedSize = m_size.width() * m_size.height() * 4;

    m_frames[0].m_frame = tiledImage;
    m_frames[0].m_hasAlpha = true;
    m_frames[0].m_isComplete = true;
    m_frames[0].m_haveMetadata = true;
    checkForSolidColor();
}

void BitmapImage::checkForSolidColor()
{
    RefPtr<TiledImageOpenVG> tiledImage = 0;

    if (m_frameCount == 1 && m_size.width() == 1 && m_size.height() == 1)
        tiledImage = nativeImageForCurrentFrame();

    if (tiledImage) {
        m_isSolidColor = true;
        RGBA32 pixel;
        vgGetImageSubData(tiledImage->tile(0, 0), &pixel, 0, VG_sARGB_8888, 0, 0, 1, 1);
        ASSERT_VG_NO_ERROR();
        m_solidColor.setRGB(pixel);
    } else
        m_isSolidColor = false;

    m_checkedForSolidColor = true;
}

void BitmapImage::initPlatformData()
{
}

void BitmapImage::invalidatePlatformData()
{
}

#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
static void adjustSourceRectForDownSampling(FloatRect& srcRect, const IntSize& origSize, const IntSize& scaledSize)
{
    // We assume down-sampling zoom rates in X direction and in Y direction are same.
    if (origSize.width() == scaledSize.width())
        return;

    // Image has been down sampled.
    double rate = static_cast<double>(scaledSize.width()) / origSize.width();
    double temp = srcRect.right() * rate;
    srcRect.setX(srcRect.x() * rate);
    srcRect.setWidth(temp - srcRect.x());
    temp = srcRect.bottom() * rate;
    srcRect.setY(srcRect.y() * rate);
    srcRect.setHeight(temp - srcRect.y());
}
#endif

void BitmapImage::draw(GraphicsContext* context, const FloatRect& dst, const FloatRect& src, ColorSpace styleColorSpace, CompositeOperator op)
{
    if (dst.isEmpty() || src.isEmpty())
        return;

    RefPtr<TiledImageOpenVG> image = nativeImageForCurrentFrame();
    if (!image)
        return;

    startAnimation();

    if (mayFillWithSolidColor()) {
        fillWithSolidColor(context, dst, solidColor(), styleColorSpace, op);
        return;
    }

    context->save();

    // Set the compositing operation.
    if (op == CompositeSourceOver && !frameHasAlphaAtIndex(m_currentFrame))
        context->setCompositeOperation(CompositeCopy);
    else
        context->setCompositeOperation(op);

    FloatRect srcRectLocal(src);
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
    adjustSourceRectForDownSampling(srcRectLocal, size(), image->size());
#endif

    context->platformContext()->activePainter()->drawImage(image.get(), dst, srcRectLocal);
    context->restore();

    if (imageObserver())
        imageObserver()->didDraw(this);
}

void Image::drawPattern(GraphicsContext* context, const FloatRect& src,
                        const AffineTransform& patternTransformation,
                        const FloatPoint& phase, ColorSpace styleColorSpace,
                        CompositeOperator op, const FloatRect& dst)
{
    if (dst.isEmpty() || src.isEmpty())
        return;

    RefPtr<TiledImageOpenVG> image = nativeImageForCurrentFrame();
    if (!image)
        return;

    startAnimation();

    if (mayFillWithSolidColor()) {
        fillWithSolidColor(context, dst, solidColor(), styleColorSpace, op);
        return;
    }

    PainterOpenVG* painter = context->platformContext()->activePainter();
    painter->save();
    painter->setCompositeOperation(op);

    FloatRect srcRectLocal(src);
#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
    adjustSourceRectForDownSampling(srcRectLocal, size(), image->size());
#endif

    AffineTransform phasedPatternTransform;
    phasedPatternTransform.translate(phase.x(), phase.y());
    phasedPatternTransform.multLeft(patternTransformation);

    PatternOpenVG pattern(image, srcRectLocal);
    pattern.setTransformation(phasedPatternTransform);

    painter->setFillPattern(pattern);
    painter->drawRect(dst, VG_FILL_PATH);

    painter->restore();

    if (imageObserver())
        imageObserver()->didDraw(this);
}

PassRefPtr<Image> Image::loadPlatformResource(char const* name)
{
    //FIXME: Either load a "proper" icon instead of drawing one, or get this
    //       upstream as cross-platform default implementation (or both).
    OwnPtr<ImageBuffer> imageBuffer = ImageBuffer::create(IntSize(16, 16));
    if (!imageBuffer)
        return 0;

    String iconName(name); // a.k.a. "enable string comparisons with =="

    if (iconName == "missingImage") {
        // Draw a fancy red x-shaped icon.
        GraphicsContext* context = imageBuffer->context();
        context->save();

        context->fillRect(FloatRect(0, 0, 16, 16), Color::white, DeviceColorSpace);

        context->setStrokeStyle(SolidStroke);
        context->setStrokeThickness(1.7);
        context->setStrokeColor(Color(48, 48, 48), DeviceColorSpace);
        context->setFillColor(Color(200, 30, 0), DeviceColorSpace);

        context->translate(8, 8);

        context->rotate(piDouble / 4.0);
        context->strokeRect(FloatRect(-1.5, -7, 3, 14));

        context->rotate(-piDouble / 2.0);
        context->strokeRect(FloatRect(-1.5, -7, 3, 14));
        context->fillRect(FloatRect(-1.5, -7, 3, 14));

        context->rotate(piDouble / 2.0);
        context->fillRect(FloatRect(-1.5, -7, 3, 14));

        context->restore();
    } else if (iconName == "searchCancel" || iconName == "searchCancelPressed") {
        // Draw a more subtle, gray x-shaped icon.
        GraphicsContext* context = imageBuffer->context();
        context->save();

        context->fillRect(FloatRect(0, 0, 16, 16), Color::white, DeviceColorSpace);

        if (iconName.length() == sizeof("searchCancel") - 1)
            context->setFillColor(Color(128, 128, 128), DeviceColorSpace);
        else
            context->setFillColor(Color(64, 64, 64), DeviceColorSpace);

        context->translate(8, 8);

        context->rotate(piDouble / 4.0);
        context->fillRect(FloatRect(-1, -7, 2, 14));

        context->rotate(-piDouble / 2.0);
        context->fillRect(FloatRect(-1, -7, 2, 14));

        context->restore();
    }

    return imageBuffer->copyImage();
}

}
