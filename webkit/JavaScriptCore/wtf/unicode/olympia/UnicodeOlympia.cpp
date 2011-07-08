/*
 *  Copyright (C) 2006 George Staikos <staikos@kde.org>
 *  Copyright (C) 2006 Alexey Proskuryakov <ap@nypop.com>
 *  Copyright (C) 2007-2009 Torch Mobile, Inc. http://www.torchmobile.com/
 *  Copyright (C) Research In Motion, Limited 2009. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

/********************************************************************************
*   Copyright (C) 1996-2007, International Business Machines Corporation and others.
********************************************************************************/

#include "config.h"
#include "UnicodeOlympia.h"

#if !USE(OLYMPIA_NATIVE_UNICODE)
#include "icu.h"
#endif

namespace WTF {
namespace Unicode {

#if !USE(OLYMPIA_NATIVE_UNICODE)

// Implemented with ICU code
bool isLower(UChar32 c)
{
    return icuIsLower(c);
}

bool isUpper(UChar32 c)
{
    return icuIsUpper(c);
}

bool isPrintableChar(UChar32 c)
{
    return icuIsPrintableChar(c);
}

bool isSpace(UChar32 c)
{
    return icuIsSpace(c);
}

bool isLetter(UChar32 c)
{
    return icuIsLetter(c);
}

bool isDigit(UChar32 c)
{
    return icuIsDigit(c);
}

bool isPunct(UChar32 c)
{
    return icuIsPunct(c);
}

Direction direction(UChar32 c)
{
    return icuDirection(c);
}

CharCategory category(unsigned int c)
{
    return icuCategory(c);
}

DecompositionType decompositionType(UChar32 c)
{
    return icuDecompositionType(c);
}

unsigned char combiningClass(UChar32 c)
{
    return icuCombiningClass(c);
}

UChar32 mirroredChar(UChar32 c)
{
    return icuMirroredChar(c);
}

int digitValue(UChar32 c)
{
    return icuDigitValue(c);
}

UChar32 toLower(UChar32 c)
{
    return icuToLower(c);
}

UChar32 toUpper(UChar32 c)
{
    return icuToUpper(c);
}

UChar32 foldCase(UChar32 c)
{
    return icuFoldCase(c);
}

UChar32 toTitleCase(UChar32 c)
{
    return icuToTitleCase(c);
}

#endif // USE(OLYMPIA_NATIVE_UNICODE)

enum ConvertType {
    ConvertLower,
    ConvertUpper
};

static inline UChar32 convertToLowerOrUpper(UChar32 c, ConvertType type)
{
    if (type == ConvertLower)
        return toLower(c);
    else
        return toUpper(c);
}

template <ConvertType convertType> static int toLowerOrUpper(UChar* result, int resultLength, const UChar* src, int srcLength,  bool* error)
{
    const UChar *e = src + srcLength;
    const UChar *s = src;
    UChar *r = result;
    UChar *re = result + resultLength;

    int needed = 0;
    if (srcLength <= resultLength)
        while (s < e)
            *r++ = convertToLowerOrUpper(*s++, convertType);
    else
        while (r < re)
            *r++ = convertToLowerOrUpper(*s++, convertType);

    if (s < e)
        needed += e - s;
    *error = (needed != 0);
    if (r < re)
        *r = 0;
    return (r - result) + needed;
}

int toLower(UChar* result, int resultLength, const UChar* src, int srcLength,  bool* error)
{
    return toLowerOrUpper<ConvertLower>(result, resultLength, src, srcLength, error);
}

int toUpper(UChar* result, int resultLength, const UChar* src, int srcLength,  bool* error)
{
    return toLowerOrUpper<ConvertUpper>(result, resultLength, src, srcLength, error);
}

int foldCase(UChar* result, int resultLength, const UChar* src, int srcLength,  bool* error)
{
    *error = false;
    if (resultLength < srcLength) {
        *error = true;
        return srcLength;
    }
    for (int i = 0; i < srcLength; ++i)
        result[i] = foldCase(src[i]);
    return srcLength;
}

} // namespace Unicode
} // namespace WTF

