/*
 * Copyright (C) 2008-2009 Torch Mobile, Inc.
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
#include "ImageDecoder.h"

#include "IntRect.h"
#include "IntSize.h"
#include "SurfaceOpenVG.h"
#include "TiledImageOpenVG.h"
#include "VGUtils.h"

#if PLATFORM(EGL)
#include "EGLDisplayOpenVG.h"
#endif

#include <openvg.h>
#include <wtf/PassRefPtr.h>

namespace WebCore {

RGBA32Buffer::RGBA32Buffer()
    : m_hasAlpha(false)
    , m_status(FrameEmpty)
    , m_duration(0)
    , m_disposalMethod(DisposeNotSpecified)
{
}

void RGBA32Buffer::clear()
{
    m_image.clear(); // dereference (possibly delete) the TiledImageOpenVG
    m_bytes.clear();
    m_status = FrameEmpty;
    // NOTE: Do not reset other members here; clearFrameBufferCache()
    // calls this to free the bitmap data, but other functions like
    // initFrameBuffer() and frameComplete() may still need to read
    // other metadata out of this frame later.
}

void RGBA32Buffer::zeroFill()
{
    m_image.clear();
    m_bytes.fill(0);
    m_hasAlpha = true;
}

void RGBA32Buffer::copyBitmapData(const RGBA32Buffer& other)
{
    if (this == &other)
        return;

    m_size = other.m_size;
    m_image.clear();

    // This will usually be called from a completed frame. Depending on whether
    // or not we have already replaced the original buffer (m_bytes) with the
    // native image pointer (m_image), we either copy m_bytes directly or
    // need to extract the pixel data from the image tiles.
    if (TiledImageOpenVG* image = other.m_image.get()) {
        m_bytes.resize(m_size.width() * m_size.height());

        static const VGImageFormat bufferFormat = VG_sARGB_8888_PRE;

        const int numColumns = image->numColumns();
        const int numRows = image->numRows();

        image->makeCompatibleContextCurrent();

        for (int yIndex = 0; yIndex < numRows; ++yIndex) {
            for (int xIndex = 0; xIndex < numColumns; ++xIndex) {
                IntRect tileRect = image->tileRect(xIndex, yIndex);
                VGImage tile = image->tile(xIndex, yIndex);

                PixelData* pixelData = m_bytes.data();
                pixelData += (tileRect.y() * width()) + tileRect.x();

                vgGetImageSubData(tile, reinterpret_cast<unsigned char*>(pixelData),
                    tileRect.width() * sizeof(PixelData), bufferFormat,
                    0, 0, tileRect.width(), tileRect.height());
                ASSERT_VG_NO_ERROR();
            }
        }
    } else
        m_bytes = other.m_bytes;

    setHasAlpha(other.m_hasAlpha);
}

bool RGBA32Buffer::setSize(int newWidth, int newHeight)
{
    // NOTE: This has no way to check for allocation failure if the
    // requested size was too big...
    m_bytes.resize(newWidth * newHeight);
    m_size = IntSize(newWidth, newHeight);

    // Zero the image.
    zeroFill();

    return true;
}

NativeImagePtr RGBA32Buffer::asNewNativeImage() const
{
    if (!m_image) {
        static const VGImageFormat bufferFormat = VG_sARGB_8888_PRE;
        // Save memory by using 16-bit images for fully opaque images.
        const VGImageFormat imageFormat = hasAlpha() ? bufferFormat : VG_sRGB_565;

#if PLATFORM(EGL)
        EGLDisplayOpenVG::current()->sharedPlatformSurface()->makeCurrent();
#endif

        const IntSize vgMaxImageSize(vgGeti(VG_MAX_IMAGE_WIDTH), vgGeti(VG_MAX_IMAGE_HEIGHT));
        ASSERT_VG_NO_ERROR();

        PassRefPtr<TiledImageOpenVG> tiledImage = adoptRef(new TiledImageOpenVG(
            IntSize(width(), height()), vgMaxImageSize));

        const int numColumns = tiledImage->numColumns();
        const int numRows = tiledImage->numRows();

        for (int yIndex = 0; yIndex < numRows; ++yIndex) {
            for (int xIndex = 0; xIndex < numColumns; ++xIndex) {
                IntRect tileRect = tiledImage->tileRect(xIndex, yIndex);
                VGImage tile = vgCreateImage(imageFormat,
                    tileRect.width(), tileRect.height(), VG_IMAGE_QUALITY_FASTER);
                ASSERT_VG_NO_ERROR();

                PixelData* pixelData = m_bytes.data();
                pixelData += (tileRect.y() * width()) + tileRect.x();

                vgImageSubData(tile, reinterpret_cast<unsigned char*>(pixelData),
                    width() * sizeof(PixelData), bufferFormat,
                    0, 0, tileRect.width(), tileRect.height());
                ASSERT_VG_NO_ERROR();

                tiledImage->setTile(xIndex, yIndex, tile);
            }
        }

        // Incomplete images will be returned without storing them in m_image,
        // so the image will be regenerated once loading is complete.
        if (m_status != FrameComplete)
            return tiledImage;

        m_image = tiledImage;
        m_bytes.clear();
    }

    return m_image; // and increase refcount
}

bool RGBA32Buffer::hasAlpha() const
{
    return m_hasAlpha;
}

void RGBA32Buffer::setHasAlpha(bool alpha)
{
    m_hasAlpha = alpha;
}

void RGBA32Buffer::setStatus(FrameStatus status)
{
    m_status = status;
}

RGBA32Buffer& RGBA32Buffer::operator=(const RGBA32Buffer& other)
{
    if (this == &other)
        return *this;

    copyBitmapData(other);
    setRect(other.rect());
    setStatus(other.status());
    setDuration(other.duration());
    setDisposalMethod(other.disposalMethod());
    return *this;
}

int RGBA32Buffer::width() const
{
    return m_size.width();
}

int RGBA32Buffer::height() const
{
    return m_size.height();
}

}
