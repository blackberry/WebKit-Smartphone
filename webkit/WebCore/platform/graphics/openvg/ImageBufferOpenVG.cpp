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
#include "ImageBuffer.h"

#include "Base64.h"
#include "BitmapImage.h"
#include "GraphicsContext.h"
#include "ImageData.h"
#include "ImageDecoder.h"
#include "JPEGImageEncoder.h"
#include "NotImplemented.h"
#include "PainterOpenVG.h"
#include "PNGImageEncoder.h"
#include "SurfaceOpenVG.h"
#include "TiledImageOpenVG.h"

#if PLATFORM(EGL)
#include "EGLDisplayOpenVG.h"
#endif

#include <openvg.h>
#include <wtf/Assertions.h>

// ImageData is RGBA on big endian and ABGR on little endian.
// VGImageFormat always works on the same bit order no matter the endianness.
// Therefore, we have to take endianness into account when converting from
// and to VGImage pixel data.
#define IMAGEDATA_VG_UNMULTIPLIED_FORMAT VGUtils::endianAwareImageFormat(VG_sRGBA_8888)
#define IMAGEDATA_VG_PREMULTIPLIED_FORMAT VGUtils::endianAwareImageFormat(VG_sRGBA_8888_PRE)
#define IMAGEBUFFER_VG_EXCHANGE_FORMAT VG_sARGB_8888

#ifdef WTF_PLATFORM_BIG_ENDIAN
#define IMAGEBUFFER_A 0
#define IMAGEBUFFER_R 1
#define IMAGEBUFFER_G 2
#define IMAGEBUFFER_B 3
#else
#define IMAGEBUFFER_A 3
#define IMAGEBUFFER_R 2
#define IMAGEBUFFER_G 1
#define IMAGEBUFFER_B 0
#endif

namespace WebCore {

ImageBufferData::ImageBufferData(const IntSize& size, int colorSpace, bool& success)
    : m_surface(0)
{
    ASSERT(size.width() > 0);
    ASSERT(size.height() > 0);
    success = false;

    switch ((ImageColorSpace) colorSpace) {
    case DeviceRGB:
        m_format = VG_sARGB_8888; break;
    case GrayScale:
        m_format = VG_sL_8; break;
    case LinearRGB:
        m_format = VG_lARGB_8888; break;
    }

#if PLATFORM(EGL)
    EGLint errorCode;
    m_surface = new SurfaceOpenVG(size, EGLDisplayOpenVG::current()->display(), 0, &errorCode);
#else
    ASSERT_NOT_REACHED();
    return;
#endif

    if (!m_surface->isValid()) {
        delete m_surface;
        m_surface = 0;
        return;
    }
    m_surface->makeCurrent();

    // Clear the ImageBuffer from whatever color was set before to transparent.
    const VGfloat transparent[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    vgSeti(VG_SCISSORING, VG_FALSE);
    vgSetfv(VG_CLEAR_COLOR, 4, transparent);
    vgClear(0, 0, size.width(), size.height());

    success = true;
}

ImageBufferData::~ImageBufferData()
{
    delete m_surface;
}

PassRefPtr<ImageData> ImageBufferData::getImageData(const IntRect& rect, const IntSize& size, VGImageFormat format) const
{
    ASSERT(m_surface);

    PassRefPtr<ImageData> result = ImageData::create(rect.width(), rect.height());
    unsigned char* data = result->data()->data()->data();

    // If this copy operation won't fill all of the pixels in the target image,
    // paint it black in order to avoid random uninitialized pixel garbage.
    if (rect.x() < 0 || rect.y() < 0 || (rect.right()) > size.width() || (rect.bottom()) > size.height())
        memset(data, 0, result->data()->length());

    m_surface->makeCurrent();

    // OpenVG ignores pixels that are out of bounds, so we can just
    // call vgReadPixels() without any further safety assurances.
    vgReadPixels(data, rect.width() * 4, format,
        rect.x(), rect.y(), rect.width(), rect.height());
    ASSERT_VG_NO_ERROR();

    return result;
}

void ImageBufferData::putImageData(ImageData*& source, const IntRect& sourceRect, const IntPoint& destPoint, const IntSize& size, VGImageFormat format)
{
    ASSERT(m_surface);

    ASSERT(sourceRect.width() > 0);
    ASSERT(sourceRect.height() > 0);

    // We expect the sourceRect to be a subset of the given source image.
    ASSERT(sourceRect.x() >= 0);
    ASSERT(sourceRect.y() >= 0);
    ASSERT(sourceRect.right() <= source->width());
    ASSERT(sourceRect.bottom() <= source->height());

    // The target origin point is the combined offset of sourceRect.location()
    // and destPoint.
    int destx = destPoint.x() + sourceRect.x();
    int desty = destPoint.y() + sourceRect.y();
    ASSERT(destx >= 0);
    ASSERT(desty >= 0);
    ASSERT(destx + sourceRect.width() <= size.width());
    ASSERT(desty + sourceRect.height() <= size.height());

    unsigned const char* data = source->data()->data()->data();
    int dataOffset = (sourceRect.y() * source->width() * 4) + (sourceRect.x() * 4);

    m_surface->makeCurrent();

    vgWritePixels(data + dataOffset, source->width() * 4, format,
        destx, desty, sourceRect.width(), sourceRect.height());
    ASSERT_VG_NO_ERROR();
}

void ImageBufferData::transformColorSpace(const Vector<int>& lookUpTable)
{
    ASSERT(m_surface);

    VGint width = m_surface->width();
    VGint height = m_surface->height();

    VGubyte* data = new VGubyte[width * height * 4];
    VGubyte* currentPixel = data;

    m_surface->makeCurrent();

    vgReadPixels(data, width * 4, IMAGEBUFFER_VG_EXCHANGE_FORMAT, 0, 0, width, height);
    ASSERT_VG_NO_ERROR();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; currentPixel += 4, x++) {
            currentPixel[IMAGEBUFFER_A] = lookUpTable[currentPixel[IMAGEBUFFER_A]];
            currentPixel[IMAGEBUFFER_R] = lookUpTable[currentPixel[IMAGEBUFFER_R]];
            currentPixel[IMAGEBUFFER_G] = lookUpTable[currentPixel[IMAGEBUFFER_G]];
            currentPixel[IMAGEBUFFER_B] = lookUpTable[currentPixel[IMAGEBUFFER_B]];
        }
    }

    vgWritePixels(data, width * 4, IMAGEBUFFER_VG_EXCHANGE_FORMAT, 0, 0, width, height);
    ASSERT_VG_NO_ERROR();

    delete[] data;
}

ImageBuffer::ImageBuffer(const IntSize& size, ImageColorSpace colorSpace, bool& success)
    : m_data(size, colorSpace, success)
    , m_size(size)
{
    if (!success) // success has already been set by m_data's constructor.
        return;

    m_context.set(new GraphicsContext(m_data.m_surface));
}

ImageBuffer::~ImageBuffer()
{
}

GraphicsContext* ImageBuffer::context() const
{
    return m_context.get();
}

Image* ImageBuffer::image() const
{
    if (!m_image) {
        // It's assumed that if image() is called, the actual rendering to the
        // GraphicsContext must be done.
        ASSERT(context());

        // As the original VGImage might be modified in the future, we copy its
        // contents into a new image and return that one.
        NativeImagePtr image = context()->platformContext()->activePainter()->asNewNativeImage(
            IntRect(IntPoint(0, 0), m_size), m_data.m_format);

        m_image = BitmapImage::create(image);
    }
    return m_image.get();
}

void ImageBuffer::platformTransformColorSpace(const Vector<int>& lookUpTable)
{
    ASSERT(context());
    context()->platformContext()->activePainter()->blitToSurface();
    m_data.transformColorSpace(lookUpTable);
}

PassRefPtr<ImageData> ImageBuffer::getUnmultipliedImageData(const IntRect& rect) const
{
    ASSERT(context());
    context()->platformContext()->activePainter()->blitToSurface();
    return m_data.getImageData(rect, m_size, IMAGEDATA_VG_UNMULTIPLIED_FORMAT);
}

PassRefPtr<ImageData> ImageBuffer::getPremultipliedImageData(const IntRect& rect) const
{
    ASSERT(context());
    context()->platformContext()->activePainter()->blitToSurface();
    return m_data.getImageData(rect, m_size, IMAGEDATA_VG_PREMULTIPLIED_FORMAT);
}

void ImageBuffer::putUnmultipliedImageData(ImageData* source, const IntRect& sourceRect, const IntPoint& destPoint)
{
    m_data.putImageData(source, sourceRect, destPoint, m_size, IMAGEDATA_VG_UNMULTIPLIED_FORMAT);
}

void ImageBuffer::putPremultipliedImageData(ImageData* source, const IntRect& sourceRect, const IntPoint& destPoint)
{
    m_data.putImageData(source, sourceRect, destPoint, m_size, IMAGEDATA_VG_PREMULTIPLIED_FORMAT);
}

String ImageBuffer::toDataURL(const String& mimeType) const
{
    if (m_size.isEmpty())
        return "data:,";

    enum {
        EncodeJPEG,
        EncodePNG,
    } encodeType = mimeType.lower() == "image/png" ? EncodePNG : EncodeJPEG;

    // According to http://www.w3.org/TR/html5/the-canvas-element.html,
    // "For image types that do not support an alpha channel, the image must be"
    // "composited onto a solid black background using the source-over operator,"
    // "and the resulting image must be the one used to create the data: URL."
    // JPEG doesn't have alpha channel, so we need premultiplied data.
    RefPtr<ImageData> imageData = encodeType == EncodePNG
        ? getUnmultipliedImageData(IntRect(IntPoint(0, 0), m_size))
        : getPremultipliedImageData(IntRect(IntPoint(0, 0), m_size));

    ASSERT(imageData && imageData->width() == m_size.width() && imageData->height() == m_size.height());

    Vector<char> output;
    const char* header;
    if (encodeType == EncodePNG) {
        if (!compressRGBABigEndianToPNG(imageData->data()->data()->data(), m_size, output))
            return "data:,";
        header = "data:image/png;base64,";
    } else {
        if (!compressRGBABigEndianToJPEG(imageData->data()->data()->data(), m_size, output))
            return "data:,";
        header = "data:image/jpeg;base64,";
    }

    Vector<char> base64;
    base64Encode(output, base64);

    output.clear();

    Vector<char> url;
    url.append(header, strlen(header));
    url.append(base64);

    return String(url.data(), url.size());
}

}
