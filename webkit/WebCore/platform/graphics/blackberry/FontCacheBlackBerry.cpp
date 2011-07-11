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
#include "FontCache.h"

#include "FontDescription.h"
#include "FontPlatformData.h"
#include "NotImplemented.h"
#include "SimpleFontData.h"

namespace WebCore {

const SimpleFontData* FontCache::getFontDataForCharacters(const Font&, const UChar* characters, int length)
{
    if (!length && characters) {
        // HACK, see floatWidthForComplexText() in FontBlackBerry.cpp
        const FontPlatformData* fontPlatformData = reinterpret_cast<const FontPlatformData*>(characters);
        return getCachedFontData(fontPlatformData);
    }
    return 0;
}

void FontCache::platformInit()
{
}

void FontCache::getTraitsInFamily(const AtomicString& familyName, Vector<unsigned>& traitsMasks)
{
    notImplemented();
}

SimpleFontData* FontCache::getLastResortFallbackFont(const FontDescription& fontDescription)
{
    static AtomicString defaultFamily("-webkit-last-resort");
    return getCachedFontData(fontDescription, defaultFamily);
}

SimpleFontData* FontCache::getSimilarFontPlatformData(const Font& font)
{
    return 0;
}

FontPlatformData* FontCache::createFontPlatformData(const FontDescription& fontDescription, const AtomicString& family)
{
    FontPlatformData* fpd = new FontPlatformData(fontDescription, family.string());

    if (fpd->isValid())
        return fpd;

    delete fpd;
    return 0;
}

}
