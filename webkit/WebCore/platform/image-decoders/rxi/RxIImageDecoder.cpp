/*
 * Copyright (C) 2006 Apple Computer, Inc.
 *
 * Copyright (C) 2001-2006 mozilla.org
 *
 * Other contributors:
 *   Stuart Parmenter <stuart@mozilla.com>
 *
 * Copyright (C) 2007-2009 Torch Mobile, Inc.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Alternatively, the contents of this file may be used under the terms
 * of either the Mozilla Public License Version 1.1, found at
 * http://www.mozilla.org/MPL/ (the "MPL") or the GNU General Public
 * License Version 2.0, found at http://www.fsf.org/copyleft/gpl.html
 * (the "GPL"), in which case the provisions of the MPL or the GPL are
 * applicable instead of those above.  If you wish to allow use of your
 * version of this file only under the terms of one of those two
 * licenses (the MPL or the GPL) and not to allow others to use your
 * version of this file under the LGPL, indicate your decision by
 * deletingthe provisions above and replace them with the notice and
 * other provisions required by the MPL or the GPL, as the case may be.
 * If you do not delete the provisions above, a recipient may use your
 * version of this file under any of the LGPL, the MPL or the GPL.
 *
 *
 */

#include "config.h"

#include "RxIImageDecoder.h"

#include "MemWriter.h"
#include "rimrxi_api.h"

namespace WebCore {


class RxIImageReader {

public:
    RxIImageReader(RxIImageDecoder* decoder)
        : m_decoder(decoder)
    {
        m_rxi = 0;
        m_memoryReader = 0;
        m_memoryManager = MemoryManager::S();
        m_w = 0;
        m_h = 0;
        m_sizeInitialized = false;
        m_format = RxIImageDecoder::FormatUnknown;
    }

    ~RxIImageReader()
    {
        if (m_rxi) {
            switch (m_format) {
            case RxIImageDecoder::FormatRdi:
                RdiDeath(m_rxi);
                break;
            case RxIImageDecoder::FormatRgi:
                RgiDeath(m_rxi);
                break;
            case RxIImageDecoder::FormatRpi:
                RpiDeath(m_rxi);
                break;
            case RxIImageDecoder::FormatRwi:
                RwiDeath(m_rxi);
                break;
            case RxIImageDecoder::FormatUnknown:
                break;
            }
        }
        m_rxi = 0;

        delete m_memoryReader;

        m_sizeInitialized = false;
        m_w = 0;
        m_h = 0;
        m_format = RxIImageDecoder::FormatUnknown;
    }

    bool decodeForSize(const Vector<char>& data, int fmt)
    {
        void* rxi = 0;
        bool result = false;
        MemReader memoryReader((unsigned char*)data.data(), data.size());

        if (m_sizeInitialized) {
            if (!m_decoder->setSize(m_w, m_h))
                return m_decoder->setFailed();
            return true;
        }

        if (!m_memoryManager)
            return false;

        // The rest of the code expects that the first 8 bytes has already been read.
        // The signature for these files is 8 bytes.
        memoryReader.SetOffset(8);


        switch (fmt) {
        case RxIImageDecoder::FormatRdi:
            rxi = RdiBirth(m_memoryManager, &memoryReader);
            break;
        case RxIImageDecoder::FormatRgi:
            rxi = RgiBirth(m_memoryManager, &memoryReader);
            break;
        case RxIImageDecoder::FormatRpi:
            rxi = RpiBirth(m_memoryManager, &memoryReader);
            break;
        case RxIImageDecoder::FormatRwi:
            rxi = RwiBirth(m_memoryManager, &memoryReader);
            break;
        case RxIImageDecoder::FormatUnknown:
            ASSERT_NOT_REACHED();
        }

        if (!rxi)
            return false;

        switch (fmt) {
        case RxIImageDecoder::FormatRdi:
            result = RdiGetWH(rxi, &m_w, &m_h);
            break;
        case RxIImageDecoder::FormatRgi:
            result = RgiGetWH(rxi, &m_w, &m_h);
            break;
        case RxIImageDecoder::FormatRpi:
            result = RpiGetWH(rxi, &m_w, &m_h);
            break;
        case RxIImageDecoder::FormatRwi:
            result = RwiGetWH(rxi, &m_w, &m_h);
            break;
        case RxIImageDecoder::FormatUnknown:
            ASSERT_NOT_REACHED();
            break;
        }

        if (!result) {
            delete m_memoryReader;
            m_memoryReader = 0;
            return false;
        }
        m_sizeInitialized = true;

        switch (fmt) {
        case RxIImageDecoder::FormatRdi:
            RdiDeath(rxi);
            break;
        case RxIImageDecoder::FormatRgi:
            RgiDeath(rxi);
            break;
        case RxIImageDecoder::FormatRpi:
            RpiDeath(rxi);
            break;
        case RxIImageDecoder::FormatRwi:
            RwiDeath(rxi);
            break;
        case RxIImageDecoder::FormatUnknown:
            ASSERT_NOT_REACHED();
            break;
        }

        // We can fill in the size now that the header is available.
        if (!m_decoder->setSize(m_w, m_h))
            return m_decoder->setFailed();

        return true;
    }

    bool decode(const Vector<char>& data, RxIImageDecoder::RxIFormat fmt)
    {
        int i = 0;
        int j = 0;
        bool result = false;
        RGBA32Buffer* frameBuffer = 0;
        unsigned char* rgbData = 0;

        if (!m_memoryManager)
            return false;

        m_memoryReader = new MemReader((unsigned char*)data.data(), data.size());

        // The rest of the code expects that the first 8 bytes has already been read.
        // The signature for these files is 8 bytes.

        m_memoryReader->SetOffset(8);


        switch (fmt) {
        case RxIImageDecoder::FormatRdi:
            m_rxi = RdiBirth(m_memoryManager, m_memoryReader);
            break;
        case RxIImageDecoder::FormatRgi:
            m_rxi = RgiBirth(m_memoryManager, m_memoryReader);
            break;
        case RxIImageDecoder::FormatRpi:
            m_rxi = RpiBirth(m_memoryManager, m_memoryReader);
            break;
        case RxIImageDecoder::FormatRwi:
            m_rxi = RwiBirth(m_memoryManager, m_memoryReader);
            break;
        case RxIImageDecoder::FormatUnknown:
            ASSERT_NOT_REACHED();
            break;
        }

        if (!m_rxi) {
            delete m_memoryReader;
            m_memoryReader = 0;
            return false;
        }
        m_format = fmt;

        switch (fmt) {
        case RxIImageDecoder::FormatRdi:
            result = RdiGetWH(m_rxi, &m_w, &m_h);
            break;
        case RxIImageDecoder::FormatRgi:
            result = RgiGetWH(m_rxi, &m_w, &m_h);
            break;
        case RxIImageDecoder::FormatRpi:
            result = RpiGetWH(m_rxi, &m_w, &m_h);
            break;
        case RxIImageDecoder::FormatRwi:
            result = RwiGetWH(m_rxi, &m_w, &m_h);
            break;
        case RxIImageDecoder::FormatUnknown:
            ASSERT_NOT_REACHED();
            break;
        }

        if (!result) {
            delete m_memoryReader;
            m_memoryReader = 0;
            return false;
        }

        // We can fill in the size now that the header is available.
        if (!m_decoder->setSize(m_w, m_h))
            return m_decoder->setFailed();

        frameBuffer = m_decoder->getUntouchedFrameBufferAtIndex(0);

        if (!frameBuffer) {
            delete m_memoryReader;
            m_memoryReader = 0;
            m_sizeInitialized = false;
            return false;
        }

        if (frameBuffer->status() == RGBA32Buffer::FrameEmpty) {
            if (!frameBuffer->setSize(m_w, m_h))
                return m_decoder->setFailed();
            frameBuffer->setStatus(RGBA32Buffer::FramePartial);

            if ( fmt == RxIImageDecoder::FormatRpi ) {
                // RPI format has alpha which we must support.
                frameBuffer->setHasAlpha(true);
            } else {
                frameBuffer->setHasAlpha(false);
            }
            // the frame always fills the entire image.
            frameBuffer->setRect(IntRect(IntPoint(0, 0), m_decoder->size()));
        }

        // note that we don't cleanup frameBuffer at all since we didn't allocate it.
        if ( fmt == RxIImageDecoder::FormatRgi ) {
            rgbData = (unsigned char*) malloc(m_w * m_h * 3);
        } else if ( fmt == RxIImageDecoder::FormatRpi ) {
            // extra channel is for alpha
            rgbData = (unsigned char*) malloc(m_w * m_h * 4);
        }

        MemWriter mw(m_w * m_h * 3);
        switch (fmt) {
        case RxIImageDecoder::FormatRdi:
            result = RdiGetRGB(m_rxi, &mw);
            break;
        case RxIImageDecoder::FormatRgi:
            result = RgiGetRGB(m_rxi, rgbData);
            break;
        case RxIImageDecoder::FormatRpi:
            result = RpiGetRGB(m_rxi, rgbData);
            break;
        case RxIImageDecoder::FormatRwi:
            result = RwiGetRGB(m_rxi, &mw);
            break;
        case RxIImageDecoder::FormatUnknown:
            ASSERT_NOT_REACHED();
            break;
        }

        if (result) {
            unsigned char* ptr = 0;
            if ((fmt == RxIImageDecoder::FormatRdi) || (fmt == RxIImageDecoder::FormatRwi))
                ptr = (unsigned char*)mw.GetBuffer();
            else
                ptr = rgbData;

            for (i = 0; i < m_h; ++i) {
                for (j = 0; j < m_w; ++j) {
                    unsigned char r, g, b;
                    unsigned char a = 0xFF;
                    r = *ptr;
                    ptr++;
                    g = *ptr;
                    ptr++;
                    b = *ptr;
                    ptr++;
                    if ( fmt == RxIImageDecoder::FormatRpi ) {
                        a = *ptr;
                        ptr++;
                    }
                    frameBuffer->setRGBA(j, i, r, g, b, a);
                }
            }
        }
        m_decoder->rxiComplete();

        // cleaning up
        free(rgbData);
        rgbData = 0;
        delete m_memoryReader;
        m_memoryReader = 0;
        m_sizeInitialized = false;

        switch (fmt) {
        case RxIImageDecoder::FormatRdi:
            RdiDeath(m_rxi);
            break;
        case RxIImageDecoder::FormatRgi:
            RgiDeath(m_rxi);
            break;
        case RxIImageDecoder::FormatRpi:
            RpiDeath(m_rxi);
            break;
        case RxIImageDecoder::FormatRwi:
            RwiDeath(m_rxi);
            break;
        case RxIImageDecoder::FormatUnknown:
            ASSERT_NOT_REACHED();
            break;
        }
        m_rxi = 0;
        return true;
    }

private:
    RxIImageDecoder* m_decoder;
    void* m_rxi;
    MemReader* m_memoryReader;
    MemoryManager* m_memoryManager;
    bool m_sizeRetrieved;
    unsigned int m_w;
    unsigned int m_h;
    bool m_sizeInitialized;
    int m_format;
};

RxIImageDecoder::RxIImageDecoder(RxIFormat fmt)
    : ImageDecoder(false /*premultiplyAlpha*/)
{
    m_format = fmt;
}

RxIImageDecoder::~RxIImageDecoder()
{
}

RGBA32Buffer* RxIImageDecoder::getUntouchedFrameBufferAtIndex(size_t index)
{
    if (index)
        return 0;

    if (m_frameBufferCache.isEmpty())
        m_frameBufferCache.resize(1);

    return &(m_frameBufferCache[index]);
}

String RxIImageDecoder::filenameExtension() const
{
    switch (m_format) {
    case RxIImageDecoder::FormatRdi:
        return "rdi";
    case RxIImageDecoder::FormatRgi:
        return "rgi";
    case RxIImageDecoder::FormatRpi:
        return "rpi";
    case RxIImageDecoder::FormatRwi:
        return "rwi";
    case RxIImageDecoder::FormatUnknown:
        // want to fall through; but this prevents a warning
        return "";
    }
    return "";
}

bool RxIImageDecoder::isSizeAvailable()
{
    if (!ImageDecoder::isSizeAvailable())
        decode(true);

    return ImageDecoder::isSizeAvailable();
}

bool RxIImageDecoder::setSize(unsigned width, unsigned height)
{
    if (!ImageDecoder::setSize(width, height))
        return false;

    prepareScaleDataIfNecessary();
    return true;
}

RGBA32Buffer* RxIImageDecoder::frameBufferAtIndex(size_t index)
{
    if (index)
        return 0;

    RGBA32Buffer* frame = getUntouchedFrameBufferAtIndex(index);

    if (!frame)
        return 0;

    if (frame->status() != RGBA32Buffer::FrameComplete)
        decode(false);
    return frame;
}

void RxIImageDecoder::rxiComplete()
{
    if (m_frameBufferCache.isEmpty())
        return;

    // Hand back an appropriately sized buffer, even if the image ended up being
    // empty.
    m_frameBufferCache[0].setStatus(RGBA32Buffer::FrameComplete);
}

void RxIImageDecoder::decode(bool onlySize)
{
    if (failed())
        return;

    if (!m_reader)
        m_reader.set(new RxIImageReader(this));

    if (onlySize) {
        if (!m_reader->decodeForSize(m_data->buffer(), m_format))
            setFailed();
        return;
    }

    if (!isAllDataReceived())
        return;

    // If we couldn't decode the image but we've received all the data, decoding
    // has failed.
    if (!m_reader->decode(m_data->buffer(), m_format))
        setFailed();

    if (failed() || (!m_frameBufferCache.isEmpty() && m_frameBufferCache[0].status() == RGBA32Buffer::FrameComplete))
        m_reader.clear();
}

}
