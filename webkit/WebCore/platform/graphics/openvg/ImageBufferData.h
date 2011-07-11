/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
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

#ifndef ImageBufferData_h
#define ImageBufferData_h

#include "TiledImageOpenVG.h"

#include <openvg.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>

namespace WebCore {

class IntPoint;
class IntSize;
class IntRect;
class ImageData;
class SurfaceOpenVG;

class ImageBufferData {
public:
    ImageBufferData(const IntSize&, int colorSpace, bool& success);
    ~ImageBufferData();

    PassRefPtr<ImageData> getImageData(const IntRect& rect,
        const IntSize& size, VGImageFormat format
    ) const;
    void putImageData(ImageData*& source, const IntRect& sourceRect,
        const IntPoint& destPoint, const IntSize& size, VGImageFormat format
    );

    void transformColorSpace(const Vector<int>& lookUpTable);

    VGImageFormat m_format;
    SurfaceOpenVG* m_surface;
    RefPtr<TiledImageOpenVG> m_tiledImage;
};

} // namespace WebCore

#endif  // ImageBufferData_h
