/*
 * Copyright (C) 2006 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2008-2009 Torch Mobile, Inc.
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RxIImageDecoder_h
#define RxIImageDecoder_h


#include "ImageDecoder.h"
#include <wtf/OwnPtr.h>

namespace WebCore {

class RxIImageReader;

// This class decodes the RxI image format.
class RxIImageDecoder : public ImageDecoder {
public:
    enum RxIFormat {
        FormatUnknown = 0,
        FormatRdi     = 1,
        FormatRgi     = 2,
        FormatRpi     = 3,
        FormatRwi     = 4
    };

    RxIImageDecoder(RxIFormat fmt);
    virtual ~RxIImageDecoder();

    // ImageDecoder
    virtual String filenameExtension() const;
    virtual bool isSizeAvailable();
    virtual bool setSize(unsigned width, unsigned height);
    virtual RGBA32Buffer* frameBufferAtIndex(size_t index);
    virtual bool supportsAlpha() const { return false; }

    bool outputScanlines();
    void rxiComplete();
    RGBA32Buffer *getUntouchedFrameBufferAtIndex(size_t index);


private:
    // Decodes the image.  If |onlySize| is true, stops decoding after
    // calculating the image size.  If decoding fails but there is no more
    // data coming, sets the "decode failure" flag.
    void decode(bool onlySize);
    RxIFormat m_format;
    OwnPtr<RxIImageReader> m_reader;
};

} // namespace WebCore
#endif
