/*  
 *  Copyright (C) 2007-2009 Torch Mobile, Inc. http://www.torchmobile.com/
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

#ifndef icu_h
#define icu_h

#include "UnicodeBlackBerry.h"

bool icuIsLower(UChar32 c);
bool icuIsUpper(UChar32 c);
bool icuIsPrintableChar(UChar32 c);
bool icuIsSpace(UChar32 c);
bool icuIsLetter(UChar32 c);
bool icuIsDigit(UChar32 c);
bool icuIsPunct(UChar32 c);
WTF::Unicode::Direction icuDirection(UChar32 c);
WTF::Unicode::CharCategory icuCategory(unsigned int c);
WTF::Unicode::DecompositionType icuDecompositionType(UChar32 c);
unsigned char icuCombiningClass(UChar32 c);
UChar32 icuMirroredChar(UChar32 c);
int icuDigitValue(UChar32 c);
UChar32 icuToLower(UChar32 c);
UChar32 icuToUpper(UChar32 c);
UChar32 icuFoldCase(UChar32 c);
UChar32 icuToTitleCase(UChar32 c);

#endif // icu_h
