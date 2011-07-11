/*
 * Copyright (C) Research In Motion, Limited 2009. All rights reserved.
 */

#ifndef BlackBerryPlatformUnicode_h
#define BlackBerryPlatformUnicode_h

namespace BlackBerry {
namespace Platform {
namespace Unicode {

    enum Direction {
        DirectionMinValue = 0,
        DirectionL = 0,
        DirectionLRE = 1,
        DirectionLRO = 2,
        DirectionR = 3,
        DirectionAL = 4,
        DirectionRLE = 5,
        DirectionRLO = 6,
        DirectionPDF = 7,
        DirectionEN = 8,
        DirectionES = 9,
        DirectionET = 10,
        DirectionAN = 11,
        DirectionCS = 12,
        DirectionNSM = 13,
        DirectionBN = 14,
        DirectionB = 15,
        DirectionS = 16,
        DirectionWS = 17,
        DirectionON = 18,
        DirectionMaxValue = 18,
    };

    enum DecompositionType {
        DecompositionNoDecomp,
        DecompositionCanonicalDecomp,
        DecompositionCompatibilityDecomp
    };

    enum CharCategory {
        CategoryLuCat = 1,
        CategoryLlCat = 2,
        CategoryLtCat = 4,
        CategoryLmCat = 8,
        CategoryLoCat = 0x10,

        CategoryLetterCatMask = CategoryLuCat | CategoryLlCat | CategoryLtCat | CategoryLmCat | CategoryLoCat, 

        CategoryMnCat = 0x20,
        CategoryMcCat = 0x40,
        CategoryMeCat = 0x80,

        CategoryMarkCatMask = CategoryMnCat | CategoryMcCat | CategoryMeCat,

        CategoryNdCat = 0x100,
        CategoryNlCat = 0x200,
        CategoryNoCat = 0x400,

        CategoryNumberCatMask = CategoryNdCat | CategoryNlCat | CategoryNoCat,

        CategoryZsCat = 0x800,
        CategoryZlCat = 0x1000,
        CategoryZpCat = 0x2000,

        CategorySeparatorCatMask = CategoryZsCat | CategoryZlCat | CategoryZpCat,

        CategoryCcCat = 0x4000,
        CategoryCfCat = 0x8000,
        CategoryCsCat = 0x10000,
        CategoryCoCat = 0x20000,
        CategoryCnCat = 0x40000,

        CategoryControlCatMask = CategoryCcCat | CategoryCfCat | CategoryCsCat | CategoryCoCat | CategoryCnCat,

        CategoryPcCat = 0x80000,
        CategoryPdCat = 0x100000,
        CategoryPsCat = 0x200000,
        CategoryPeCat = 0x400000,
        CategoryPiCat = 0x800000,
        CategoryPfCat = 0x1000000,
        CategoryPoCat = 0x2000000,

        CategoryPunctCatMask = CategoryPcCat | CategoryPdCat | CategoryPsCat | CategoryPeCat | CategoryPiCat | CategoryPfCat | CategoryPoCat,

        CategorySmCat = 0x4000000,
        CategoryScCat = 0x8000000,
        CategorySkCat = 0x10000000,
        CategorySoCat = 0x20000000,

        CategorySymbolCatMask = CategorySmCat | CategoryScCat | CategorySkCat | CategorySoCat
    };

    Direction direction(unsigned c);
    DecompositionType decompositionType(unsigned c);
    CharCategory category(unsigned c);
    bool isSpace(unsigned c);
    bool isLetter(unsigned c);
    bool isPrintableChar(unsigned c);
    bool isUpper(unsigned c);
    bool isLower(unsigned c);
    bool isPunct(unsigned c);
    bool isDigit(unsigned c);
    unsigned toLower(unsigned c);
    unsigned toUpper(unsigned c);
    unsigned foldCase(unsigned c);
    unsigned toTitleCase(unsigned c);
    unsigned mirroredChar(unsigned c);
    unsigned char combiningClass(unsigned c);

} // namespace Unicode
} // namespace Platform
} // namespace Olympia

#endif // BlackBerryPlatformUnicode_h
