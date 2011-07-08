/*
 *  Copyright (C) 2006 George Staikos <staikos@kde.org>
 *  Copyright (C) 2006 Alexey Proskuryakov <ap@nypop.com>
 *  Copyright (C) 2007 Apple Computer, Inc. All rights reserved.
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

#ifndef KJS_UNICODE_OLYMPIA_H
#define KJS_UNICODE_OLYMPIA_H

#include <stdint.h>
#include <wchar.h>

#if USE(OLYMPIA_NATIVE_UNICODE)
#include <OlympiaPlatformUnicode.h>
#else
#define TO_MASK(x) (1 << (x))
#endif

typedef uint16_t UChar;
typedef uint32_t UChar32;

// some defines from ICU needed one or two places

#define U16_IS_LEAD(c) (((c)&0xfffffc00)==0xd800)
#define U16_IS_TRAIL(c) (((c)&0xfffffc00)==0xdc00)
#define U16_SURROGATE_OFFSET ((0xd800<<10UL)+0xdc00-0x10000)
#define U16_GET_SUPPLEMENTARY(lead, trail) \
    (((UChar32)(lead)<<10UL)+(UChar32)(trail)-U16_SURROGATE_OFFSET)

#define U16_LEAD(supplementary) (UChar)(((supplementary)>>10)+0xd7c0)
#define U16_TRAIL(supplementary) (UChar)(((supplementary)&0x3ff)|0xdc00)

#define U_IS_SURROGATE(c) (((c)&0xfffff800)==0xd800)
#define U16_IS_SURROGATE(c) U_IS_SURROGATE(c)
#define U16_IS_SURROGATE_LEAD(c) (((c)&0x400)==0)

#define U16_NEXT(s, i, length, c) { \
    (c)=(s)[(i)++]; \
    if(U16_IS_LEAD(c)) { \
        uint16_t __c2; \
        if((i)<(length) && U16_IS_TRAIL(__c2=(s)[(i)])) { \
            ++(i); \
            (c)=U16_GET_SUPPLEMENTARY((c), __c2); \
        } \
    } \
}

#define U16_IS_SINGLE(c) !U_IS_SURROGATE(c)

#define U16_PREV(s, start, i, c) { \
    (c)=(s)[--(i)]; \
    if(U16_IS_TRAIL(c)) { \
        uint16_t __c2; \
        if((i)>(start) && U16_IS_LEAD(__c2=(s)[(i)-1])) { \
            --(i); \
            (c)=U16_GET_SUPPLEMENTARY(__c2, (c)); \
        } \
    } \
}

namespace WTF {
    namespace Unicode {

#if USE(OLYMPIA_NATIVE_UNICODE)

        enum Direction {
            LeftToRight = Olympia::Platform::Unicode::DirectionL,
            RightToLeft = Olympia::Platform::Unicode::DirectionR,
            EuropeanNumber = Olympia::Platform::Unicode::DirectionEN,
            EuropeanNumberSeparator = Olympia::Platform::Unicode::DirectionES,
            EuropeanNumberTerminator = Olympia::Platform::Unicode::DirectionET,
            ArabicNumber = Olympia::Platform::Unicode::DirectionAN,
            CommonNumberSeparator = Olympia::Platform::Unicode::DirectionCS,
            BlockSeparator = Olympia::Platform::Unicode::DirectionB,
            SegmentSeparator = Olympia::Platform::Unicode::DirectionS,
            WhiteSpaceNeutral = Olympia::Platform::Unicode::DirectionWS,
            OtherNeutral = Olympia::Platform::Unicode::DirectionON,
            LeftToRightEmbedding = Olympia::Platform::Unicode::DirectionLRE,
            LeftToRightOverride = Olympia::Platform::Unicode::DirectionLRO,
            RightToLeftArabic = Olympia::Platform::Unicode::DirectionAL,
            RightToLeftEmbedding = Olympia::Platform::Unicode::DirectionRLE,
            RightToLeftOverride = Olympia::Platform::Unicode::DirectionRLO,
            PopDirectionalFormat = Olympia::Platform::Unicode::DirectionPDF,
            NonSpacingMark = Olympia::Platform::Unicode::DirectionNSM,
            BoundaryNeutral = Olympia::Platform::Unicode::DirectionBN
        };

        enum DecompositionType {
            DecompositionNone = Olympia::Platform::Unicode::DecompositionNoDecomp,
            DecompositionCanonical = Olympia::Platform::Unicode::DecompositionCanonicalDecomp,
            DecompositionCompat = Olympia::Platform::Unicode::DecompositionCompatibilityDecomp,
            // FIXME: We only support the above basic decomposition types
            DecompositionFont,
        };

        enum CharCategory {
            NoCategory =  0,
            Letter_Uppercase = Olympia::Platform::Unicode::CategoryLuCat,
            Letter_Lowercase = Olympia::Platform::Unicode::CategoryLlCat,
            Letter_Titlecase = Olympia::Platform::Unicode::CategoryLtCat,
            Letter_Modifier = Olympia::Platform::Unicode::CategoryLmCat,
            Letter_Other = Olympia::Platform::Unicode::CategoryLoCat,

            Mark_NonSpacing = Olympia::Platform::Unicode::CategoryMnCat,
            Mark_Enclosing = Olympia::Platform::Unicode::CategoryMeCat,
            Mark_SpacingCombining = Olympia::Platform::Unicode::CategoryMcCat,

            Number_DecimalDigit = Olympia::Platform::Unicode::CategoryNdCat,
            Number_Letter = Olympia::Platform::Unicode::CategoryNlCat,
            Number_Other = Olympia::Platform::Unicode::CategoryNoCat,

            Separator_Space = Olympia::Platform::Unicode::CategoryZsCat,
            Separator_Line = Olympia::Platform::Unicode::CategoryZlCat,
            Separator_Paragraph = Olympia::Platform::Unicode::CategoryZpCat,

            Other_Control = Olympia::Platform::Unicode::CategoryCcCat,
            Other_Format = Olympia::Platform::Unicode::CategoryCfCat,
            Other_PrivateUse = Olympia::Platform::Unicode::CategoryCoCat,
            Other_Surrogate = Olympia::Platform::Unicode::CategoryCsCat,
            Other_NotAssigned = Olympia::Platform::Unicode::CategoryCnCat,

            Punctuation_Dash = Olympia::Platform::Unicode::CategoryPdCat,
            Punctuation_Open = Olympia::Platform::Unicode::CategoryPsCat,
            Punctuation_Close = Olympia::Platform::Unicode::CategoryPeCat,
            Punctuation_Connector = Olympia::Platform::Unicode::CategoryPcCat,
            Punctuation_InitialQuote = Olympia::Platform::Unicode::CategoryPiCat,
            Punctuation_FinalQuote = Olympia::Platform::Unicode::CategoryPfCat,
            Punctuation_Other = Olympia::Platform::Unicode::CategoryPoCat,

            Symbol_Math = Olympia::Platform::Unicode::CategorySmCat,
            Symbol_Currency = Olympia::Platform::Unicode::CategoryScCat,
            Symbol_Modifier = Olympia::Platform::Unicode::CategorySkCat,
            Symbol_Other = Olympia::Platform::Unicode::CategorySoCat,
        };

        static inline CharCategory category(UChar32 c) { return static_cast<CharCategory>(Olympia::Platform::Unicode::category(c)); }
        static inline DecompositionType decompositionType(UChar32 c) { return static_cast<DecompositionType>(Olympia::Platform::Unicode::decompositionType(c)); }
        static inline Direction direction(UChar32 c) { return static_cast<Direction>(Olympia::Platform::Unicode::direction(c)); }
        static inline bool isSpace(UChar32 c) { return Olympia::Platform::Unicode::isSpace(c); }
        static inline bool isLetter(UChar32 c) { return Olympia::Platform::Unicode::isLetter(c); }
        static inline bool isPrintableChar(UChar32 c) { return Olympia::Platform::Unicode::isPrintableChar(c); }
        static inline bool isUpper(UChar32 c) { return Olympia::Platform::Unicode::isUpper(c); }
        static inline bool isLower(UChar32 c) { return Olympia::Platform::Unicode::isLower(c); }
        static inline bool isPunct(UChar32 c) { return Olympia::Platform::Unicode::isPunct(c); }
        static inline bool isDigit(UChar32 c) { return Olympia::Platform::Unicode::isDigit(c); }
        static inline UChar32 toLower(UChar32 c) { return Olympia::Platform::Unicode::toLower(c); }
        static inline UChar32 toUpper(UChar32 c) { return Olympia::Platform::Unicode::toUpper(c); }
        static inline UChar32 foldCase(UChar32 c) { return Olympia::Platform::Unicode::foldCase(c); }
        static inline UChar32 toTitleCase(UChar32 c) { return Olympia::Platform::Unicode::toTitleCase(c); }
        static inline UChar32 mirroredChar(UChar32 c) { return Olympia::Platform::Unicode::mirroredChar(c); }
        static inline unsigned char combiningClass(UChar32 c) { return Olympia::Platform::Unicode::combiningClass(c); }
#else

        enum IcuCharDirection {
            U_LEFT_TO_RIGHT               = 0,
            U_RIGHT_TO_LEFT               = 1,
            U_EUROPEAN_NUMBER             = 2,
            U_EUROPEAN_NUMBER_SEPARATOR   = 3,
            U_EUROPEAN_NUMBER_TERMINATOR  = 4,
            U_ARABIC_NUMBER               = 5,
            U_COMMON_NUMBER_SEPARATOR     = 6,
            U_BLOCK_SEPARATOR             = 7,
            U_SEGMENT_SEPARATOR           = 8,
            U_WHITE_SPACE_NEUTRAL         = 9,
            U_OTHER_NEUTRAL               = 10,
            U_LEFT_TO_RIGHT_EMBEDDING     = 11,
            U_LEFT_TO_RIGHT_OVERRIDE      = 12,
            U_RIGHT_TO_LEFT_ARABIC        = 13,
            U_RIGHT_TO_LEFT_EMBEDDING     = 14,
            U_RIGHT_TO_LEFT_OVERRIDE      = 15,
            U_POP_DIRECTIONAL_FORMAT      = 16,
            U_DIR_NON_SPACING_MARK        = 17,
            U_BOUNDARY_NEUTRAL            = 18,
            U_CHAR_DIRECTION_COUNT
        };
        enum IcuDecompositionType {
            U_DT_NONE,              /*[none]*/ /*See note !!*/
            U_DT_CANONICAL,         /*[can]*/
            U_DT_COMPAT,            /*[com]*/
            U_DT_CIRCLE,            /*[enc]*/
            U_DT_FINAL,             /*[fin]*/
            U_DT_FONT,              /*[font]*/
            U_DT_FRACTION,          /*[fra]*/
            U_DT_INITIAL,           /*[init]*/
            U_DT_ISOLATED,          /*[iso]*/
            U_DT_MEDIAL,            /*[med]*/
            U_DT_NARROW,            /*[nar]*/
            U_DT_NOBREAK,           /*[nb]*/
            U_DT_SMALL,             /*[sml]*/
            U_DT_SQUARE,            /*[sqr]*/
            U_DT_SUB,               /*[sub]*/
            U_DT_SUPER,             /*[sup]*/
            U_DT_VERTICAL,          /*[vert]*/
            U_DT_WIDE,              /*[wide]*/
            U_DT_COUNT /* 18 */
        };
        enum IcuCharCategory
        {
            U_UNASSIGNED              = 0,
            U_GENERAL_OTHER_TYPES     = 0,
            U_UPPERCASE_LETTER        = 1,
            U_LOWERCASE_LETTER        = 2,
            U_TITLECASE_LETTER        = 3,
            U_MODIFIER_LETTER         = 4,
            U_OTHER_LETTER            = 5,
            U_NON_SPACING_MARK        = 6,
            U_ENCLOSING_MARK          = 7,
            U_COMBINING_SPACING_MARK  = 8,
            U_DECIMAL_DIGIT_NUMBER    = 9,
            U_LETTER_NUMBER           = 10,
            U_OTHER_NUMBER            = 11,
            U_SPACE_SEPARATOR         = 12,
            U_LINE_SEPARATOR          = 13,
            U_PARAGRAPH_SEPARATOR     = 14,
            U_CONTROL_CHAR            = 15,
            U_FORMAT_CHAR             = 16,
            U_PRIVATE_USE_CHAR        = 17,
            U_SURROGATE               = 18,
            U_DASH_PUNCTUATION        = 19,
            U_START_PUNCTUATION       = 20,
            U_END_PUNCTUATION         = 21,
            U_CONNECTOR_PUNCTUATION   = 22,
            U_OTHER_PUNCTUATION       = 23,
            U_MATH_SYMBOL             = 24,
            U_CURRENCY_SYMBOL         = 25,
            U_MODIFIER_SYMBOL         = 26,
            U_OTHER_SYMBOL            = 27,
            U_INITIAL_PUNCTUATION     = 28,
            U_FINAL_PUNCTUATION       = 29,
            U_CHAR_CATEGORY_COUNT
        };
        
        enum Direction {
          LeftToRight = U_LEFT_TO_RIGHT,
          RightToLeft = U_RIGHT_TO_LEFT,
          EuropeanNumber = U_EUROPEAN_NUMBER,
          EuropeanNumberSeparator = U_EUROPEAN_NUMBER_SEPARATOR,
          EuropeanNumberTerminator = U_EUROPEAN_NUMBER_TERMINATOR,
          ArabicNumber = U_ARABIC_NUMBER,
          CommonNumberSeparator = U_COMMON_NUMBER_SEPARATOR,
          BlockSeparator = U_BLOCK_SEPARATOR,
          SegmentSeparator = U_SEGMENT_SEPARATOR,
          WhiteSpaceNeutral = U_WHITE_SPACE_NEUTRAL,
          OtherNeutral = U_OTHER_NEUTRAL,
          LeftToRightEmbedding = U_LEFT_TO_RIGHT_EMBEDDING,
          LeftToRightOverride = U_LEFT_TO_RIGHT_OVERRIDE,
          RightToLeftArabic = U_RIGHT_TO_LEFT_ARABIC,
          RightToLeftEmbedding = U_RIGHT_TO_LEFT_EMBEDDING,
          RightToLeftOverride = U_RIGHT_TO_LEFT_OVERRIDE,
          PopDirectionalFormat = U_POP_DIRECTIONAL_FORMAT,
          NonSpacingMark = U_DIR_NON_SPACING_MARK,
          BoundaryNeutral = U_BOUNDARY_NEUTRAL
        };

        enum DecompositionType {
          DecompositionNone = U_DT_NONE,
          DecompositionCanonical = U_DT_CANONICAL,
          DecompositionCompat = U_DT_COMPAT,
          DecompositionCircle = U_DT_CIRCLE,
          DecompositionFinal = U_DT_FINAL,
          DecompositionFont = U_DT_FONT,
          DecompositionFraction = U_DT_FRACTION,
          DecompositionInitial = U_DT_INITIAL,
          DecompositionIsolated = U_DT_ISOLATED,
          DecompositionMedial = U_DT_MEDIAL,
          DecompositionNarrow = U_DT_NARROW,
          DecompositionNoBreak = U_DT_NOBREAK,
          DecompositionSmall = U_DT_SMALL,
          DecompositionSquare = U_DT_SQUARE,
          DecompositionSub = U_DT_SUB,
          DecompositionSuper = U_DT_SUPER,
          DecompositionVertical = U_DT_VERTICAL,
          DecompositionWide = U_DT_WIDE,
        };

        enum CharCategory {
          NoCategory =  0,
          Other_NotAssigned = TO_MASK(U_GENERAL_OTHER_TYPES),
          Letter_Uppercase = TO_MASK(U_UPPERCASE_LETTER),
          Letter_Lowercase = TO_MASK(U_LOWERCASE_LETTER),
          Letter_Titlecase = TO_MASK(U_TITLECASE_LETTER),
          Letter_Modifier = TO_MASK(U_MODIFIER_LETTER),
          Letter_Other = TO_MASK(U_OTHER_LETTER),

          Mark_NonSpacing = TO_MASK(U_NON_SPACING_MARK),
          Mark_Enclosing = TO_MASK(U_ENCLOSING_MARK),
          Mark_SpacingCombining = TO_MASK(U_COMBINING_SPACING_MARK),

          Number_DecimalDigit = TO_MASK(U_DECIMAL_DIGIT_NUMBER),
          Number_Letter = TO_MASK(U_LETTER_NUMBER),
          Number_Other = TO_MASK(U_OTHER_NUMBER),

          Separator_Space = TO_MASK(U_SPACE_SEPARATOR),
          Separator_Line = TO_MASK(U_LINE_SEPARATOR),
          Separator_Paragraph = TO_MASK(U_PARAGRAPH_SEPARATOR),

          Other_Control = TO_MASK(U_CONTROL_CHAR),
          Other_Format = TO_MASK(U_FORMAT_CHAR),
          Other_PrivateUse = TO_MASK(U_PRIVATE_USE_CHAR),
          Other_Surrogate = TO_MASK(U_SURROGATE),

          Punctuation_Dash = TO_MASK(U_DASH_PUNCTUATION),
          Punctuation_Open = TO_MASK(U_START_PUNCTUATION),
          Punctuation_Close = TO_MASK(U_END_PUNCTUATION),
          Punctuation_Connector = TO_MASK(U_CONNECTOR_PUNCTUATION),
          Punctuation_Other = TO_MASK(U_OTHER_PUNCTUATION),

          Symbol_Math = TO_MASK(U_MATH_SYMBOL),
          Symbol_Currency = TO_MASK(U_CURRENCY_SYMBOL),
          Symbol_Modifier = TO_MASK(U_MODIFIER_SYMBOL),
          Symbol_Other = TO_MASK(U_OTHER_SYMBOL),

          Punctuation_InitialQuote = TO_MASK(U_INITIAL_PUNCTUATION),
          Punctuation_FinalQuote = TO_MASK(U_FINAL_PUNCTUATION)
        };

        CharCategory category(UChar32 c);
        bool isSpace(UChar32 c);
        bool isLetter(UChar32 c);
        bool isPrintableChar(UChar32 c);
        bool isUpper(UChar32 c);
        bool isLower(UChar32 c);
        bool isPunct(UChar32 c);
        bool isDigit(UChar32 c);
        UChar32 toLower(UChar32 c);
        UChar32 toUpper(UChar32 c);
        UChar32 foldCase(UChar32 c);
        UChar32 toTitleCase(UChar32 c);
        int digitValue(UChar32 c);
        UChar32 mirroredChar(UChar32 c);
        unsigned char combiningClass(UChar32 c);
        DecompositionType decompositionType(UChar32 c);
        Direction direction(UChar32 c);
#endif

        inline bool isAlphanumeric(UChar32 c) { return isLetter(c) || isDigit(c); }
        inline bool isSeparatorSpace(UChar32 c) { return category(c) == Separator_Space; }
        inline bool isHighSurrogate(UChar32 c) { return (c & 0xfc00) == 0xd800; }
        inline bool isLowSurrogate(UChar32 c) { return (c & 0xfc00) == 0xdc00; }

        int toLower(UChar* result, int resultLength, const UChar* src, int srcLength,  bool* error);
        int toUpper(UChar* result, int resultLength, const UChar* src, int srcLength,  bool* error);
        int foldCase(UChar* result, int resultLength, const UChar* src, int srcLength,  bool* error);

        inline bool isArabicChar(UChar32 /*c*/)
        {
            return false; // FIXME: implement!
        }

        inline int umemcasecmp(const UChar* a, const UChar* b, int len)
        {
            for (int i = 0; i < len; ++i)
            {
                UChar c1 = foldCase(a[i]);
                UChar c2 = foldCase(b[i]);
                if (c1 != c2)
                    return c1 - c2;
            }
            return 0;
        }

        inline UChar32 surrogateToUcs4(UChar high, UChar low)
        {
            return (UChar32(high)<<10) + low - 0x35fdc00;
        }

        inline bool hasLineBreakingPropertyComplexContext(UChar32)
        {
            return false;
        }
    }
}

#endif
// vim: ts=2 sw=2 et
