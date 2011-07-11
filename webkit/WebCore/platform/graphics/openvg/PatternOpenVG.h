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

#ifndef PatternOpenVG_h
#define PatternOpenVG_h

#include "IntPoint.h"
#include "TiledImageOpenVG.h"
#include "AffineTransform.h"

#include <openvg.h>
#include <wtf/RefPtr.h>
#include <wtf/PassRefPtr.h>

namespace WebCore {

class FloatRect;

class PatternOpenVG {
public:
    PatternOpenVG();
    PatternOpenVG(const PatternOpenVG&);
    PatternOpenVG(PassRefPtr<TiledImageOpenVG>);
    PatternOpenVG(PassRefPtr<TiledImageOpenVG>, const FloatRect& srcRect);

    bool isValid() const;

    void setImage(PassRefPtr<TiledImageOpenVG>, const FloatRect& srcRect);
    VGImage vgImage() const;

    void setTransformation(const AffineTransform& transformation) { m_transformation = transformation; }
    const AffineTransform& originalTransformation() const { return m_transformation; }

    /** PatternOpenVG might scale down the original tiled image so the
     * original pattern transformation might need to be scaled up to make up
     * for that. This method returns the original transformation adapted by
     * that scale factor. */
    AffineTransform effectiveTransformation() const;

private:
    RefPtr<TiledImageOpenVG> m_image;
    IntPoint m_imageTile;
    AffineTransform m_transformation;
    float m_scaleX;
    float m_scaleY;
};

}

#endif
