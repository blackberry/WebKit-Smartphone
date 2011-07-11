
/********************************************************************************
*   Copyright (C) 1996-2007, International Business Machines Corporation and others.
********************************************************************************/

/*  This library is free software; you can redistribute it and/or
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

#include "config.h"
#include "icu.h"

#if !USE(OLYMPIA_NATIVE_UNICODE)

typedef bool UBool;

#define TRUE    (true)
#define FALSE   (false)

#define U_MASK(x) ((uint32_t)1<<(x))
#define U_GC_CN_MASK    U_MASK(U_GENERAL_OTHER_TYPES)
#define U_GC_CC_MASK    U_MASK(U_CONTROL_CHAR)
#define U_GC_CF_MASK    U_MASK(U_FORMAT_CHAR)
#define U_GC_CO_MASK    U_MASK(U_PRIVATE_USE_CHAR)
#define U_GC_CS_MASK    U_MASK(U_SURROGATE)
#define U_GC_ZS_MASK    U_MASK(U_SPACE_SEPARATOR)
#define U_GC_ZL_MASK    U_MASK(U_LINE_SEPARATOR)
#define U_GC_ZP_MASK    U_MASK(U_PARAGRAPH_SEPARATOR)
#define U_GC_LU_MASK    U_MASK(U_UPPERCASE_LETTER)
#define U_GC_LL_MASK    U_MASK(U_LOWERCASE_LETTER)
#define U_GC_LT_MASK    U_MASK(U_TITLECASE_LETTER)
#define U_GC_LM_MASK    U_MASK(U_MODIFIER_LETTER)
#define U_GC_LO_MASK    U_MASK(U_OTHER_LETTER)
#define U_GC_PD_MASK    U_MASK(U_DASH_PUNCTUATION)
#define U_GC_PS_MASK    U_MASK(U_START_PUNCTUATION)
#define U_GC_PE_MASK    U_MASK(U_END_PUNCTUATION)
#define U_GC_PC_MASK    U_MASK(U_CONNECTOR_PUNCTUATION)
#define U_GC_PO_MASK    U_MASK(U_OTHER_PUNCTUATION)
#define U_GC_PI_MASK    U_MASK(U_INITIAL_PUNCTUATION)
#define U_GC_PF_MASK    U_MASK(U_FINAL_PUNCTUATION)
#define U_GC_C_MASK \
            (U_GC_CN_MASK|U_GC_CC_MASK|U_GC_CF_MASK|U_GC_CO_MASK|U_GC_CS_MASK)
#define U_GC_Z_MASK (U_GC_ZS_MASK|U_GC_ZL_MASK|U_GC_ZP_MASK)
#define U_GC_L_MASK \
            (U_GC_LU_MASK|U_GC_LL_MASK|U_GC_LT_MASK|U_GC_LM_MASK|U_GC_LO_MASK)
#define U_GC_P_MASK \
            (U_GC_PD_MASK|U_GC_PS_MASK|U_GC_PE_MASK|U_GC_PC_MASK|U_GC_PO_MASK| \
             U_GC_PI_MASK|U_GC_PF_MASK)

#define CAT_MASK(props) U_MASK((props & 0x1f))
#define IS_THAT_CONTROL_SPACE(c) \
    (c<=0x9f && ((c>=TAB && c<=CR) || (c>=0x1c && c <=0x1f) || c==NL))

#define _UTRIE_GET_RAW(trie, data, offset, c16) \
    (trie)->data[ \
        ((int32_t)((trie)->index[(offset)+((c16)>>UTRIE_SHIFT)])<<UTRIE_INDEX_SHIFT)+ \
        ((c16)&UTRIE_MASK) \
    ]

#define _UTRIE_GET_FROM_PAIR(trie, data, c, c2, result, resultType) { \
    int32_t __offset; \
\
    /* get data for lead surrogate */ \
    (result)=_UTRIE_GET_RAW((trie), data, 0, (c)); \
    __offset=(trie)->getFoldingOffset(result); \
\
    /* get the real data from the folded lead/trail units */ \
    if(__offset>0) { \
        (result)=_UTRIE_GET_RAW((trie), data, __offset, (c2)&0x3ff); \
    } else { \
        (result)=(resultType)((trie)->initialValue); \
    } \
}

#define _UTRIE_GET_FROM_BMP(trie, data, c16) \
    _UTRIE_GET_RAW(trie, data, 0xd800<=(c16) && (c16)<=0xdbff ? UTRIE_LEAD_INDEX_DISP : 0, c16);

#define _UTRIE_GET(trie, data, c32, result, resultType) \
    if((uint32_t)(c32)<=0xffff) { \
        /* BMP code points */ \
        (result)=_UTRIE_GET_FROM_BMP(trie, data, c32); \
    } else if((uint32_t)(c32)<=0x10ffff) { \
        /* supplementary code point */ \
        uint16_t __lead16=(uint16_t)(((c32)>>10)+0xd7c0); \
        _UTRIE_GET_FROM_PAIR(trie, data, __lead16, c32, result, resultType); \
    } else { \
        /* out of range */ \
        (result)=(resultType)((trie)->initialValue); \
    }

#define UTRIE_GET16(trie, c16, result) _UTRIE_GET(trie, index, c16, result, uint16_t)
#define UTRIE_GET32(trie, c32, result) _UTRIE_GET(trie, data32, c32, result, uint32_t)

#define GET_PROPS(c, result) UTRIE_GET16(&charPropsTrie, c, result);
#define GET_BIDI_PROPS(bdp, c, result) \
    UTRIE_GET16(&(bdp)->trie, c, result);
#define UBIDI_GET_MIRROR_CODE_POINT(m) (UChar32)((m)&0x1fffff)
#define UBIDI_GET_MIRROR_INDEX(m) ((m)>>UBIDI_MIRROR_INDEX_SHIFT)
#define UBIDI_CLASS_MASK        0x0000001f
#define UBIDI_JT_MASK           0x000000e0
#define UBIDI_MAX_JG_MASK       0x00ff0000
#define UBIDI_GET_CLASS(props) ((props)&UBIDI_CLASS_MASK)
#define UBIDI_GET_FLAG(props, shift) (((props)>>(shift))&1)
#define UPROPS_DT_MASK          0x0000001f
#define UNEWTRIE2_MAX_DATA_LENGTH (0x110000+0x40+0x40+0x400)
#define _NORM_MIN_SPECIAL       0xfc000000
#define _NORM_SURROGATES_TOP    0xfff00000
#define _NORM_AUX_FNC_MASK          (uint32_t)(_NORM_AUX_MAX_FNC-1)
#define _NORM_AUX_MAX_FNC           ((int32_t)1<<_NORM_AUX_COMP_EX_SHIFT)

#define HAS_SLOT(flags, idx) ((flags)&(1<<(idx)))
#define SLOT_OFFSET(flags, idx) flagsOffset[(flags)&((1<<(idx))-1)]
#define GET_SLOT_VALUE(excWord, idx, pExc16, value) \
    if(((excWord)&UCASE_EXC_DOUBLE_SLOTS)==0) { \
        (pExc16)+=SLOT_OFFSET(excWord, idx); \
        (value)=*pExc16; \
    } else { \
        (pExc16)+=2*SLOT_OFFSET(excWord, idx); \
        (value)=*pExc16++; \
        (value)=((value)<<16)|*pExc16; \
    }
#define UCASE_EXC_DOUBLE_SLOTS      0x100
#define UCASE_EXC_CONDITIONAL_FOLD      0x8000

#define _UTRIE2_INDEX_FROM_CP(trie, asciiOffset, c) \
    ((uint32_t)(c)<0xd800 ? \
        _UTRIE2_INDEX_RAW(0, (trie)->index, c) : \
        (uint32_t)(c)<=0xffff ? \
            _UTRIE2_INDEX_RAW( \
                (uint32_t)(c)<=0xdbff ? UTRIE2_LSCP_INDEX_2_OFFSET-(0xd800>>UTRIE2_SHIFT_2) : 0, \
                (trie)->index, c) : \
            (uint32_t)(c)>0x10ffff ? \
                (asciiOffset)+UTRIE2_BAD_UTF8_DATA_OFFSET : \
                (uint32_t)(c)>=(trie)->highStart ? \
                    (trie)->highValueIndex : \
                    _UTRIE2_INDEX_FROM_SUPP((trie)->index, c))
#define _UTRIE2_GET(trie, data, asciiOffset, c) \
    (trie)->data[_UTRIE2_INDEX_FROM_CP(trie, asciiOffset, c)]
#define UTRIE2_GET16(trie, c) _UTRIE2_GET((trie), index, (trie)->indexLength, (c))

#define _UTRIE2_INDEX_RAW(offset, trieIndex, c) \
    (((int32_t)((trieIndex)[(offset)+((c)>>UTRIE2_SHIFT_2)]) \
    <<UTRIE2_INDEX_SHIFT)+ \
    ((c)&UTRIE2_DATA_MASK))
#define _UTRIE2_INDEX_FROM_SUPP(trieIndex, c) \
    (((int32_t)((trieIndex)[ \
        (trieIndex)[(UTRIE2_INDEX_1_OFFSET-UTRIE2_OMITTED_BMP_INDEX_1_LENGTH)+ \
                      ((c)>>UTRIE2_SHIFT_1)]+ \
        (((c)>>UTRIE2_SHIFT_2)&UTRIE2_INDEX_2_MASK)]) \
    <<UTRIE2_INDEX_SHIFT)+ \
    ((c)&UTRIE2_DATA_MASK))

#define PROPS_HAS_EXCEPTION(props) ((props)&UCASE_EXCEPTION)
#define UCASE_EXCEPTION     8
#define UCASE_TYPE_MASK     3
#define UCASE_GET_TYPE(props) ((props)&UCASE_TYPE_MASK)
#define UCASE_DELTA_SHIFT   6
#define UCASE_GET_DELTA(props) ((int16_t)(props)>>UCASE_DELTA_SHIFT)
#define GET_EXCEPTIONS(csp, props) ((csp)->exceptions+((props)>>UCASE_EXC_SHIFT))
#define UCASE_EXC_SHIFT     4
#define UCASE_EXC_MASK      0xfff0
#define UCASE_MAX_EXCEPTIONS 0x1000

enum {
    UTRIE_SHIFT=5,
    UTRIE_DATA_BLOCK_LENGTH=1<<UTRIE_SHIFT,
    UTRIE_MASK=UTRIE_DATA_BLOCK_LENGTH-1,
    UTRIE_LEAD_INDEX_DISP=0x2800>>UTRIE_SHIFT,
    UTRIE_INDEX_SHIFT=2,
    UTRIE_DATA_GRANULARITY=1<<UTRIE_INDEX_SHIFT,
    UTRIE_SURROGATE_BLOCK_BITS=10-UTRIE_SHIFT,
    UTRIE_SURROGATE_BLOCK_COUNT=(1<<UTRIE_SURROGATE_BLOCK_BITS),
    UTRIE_BMP_INDEX_LENGTH=0x10000>>UTRIE_SHIFT
};

enum {
    UPROPS_INDEX_COUNT=16,

    /* general category shift==0                                0 (5 bits) */
    UPROPS_NUMERIC_TYPE_SHIFT=5,                            /*  5 (3 bits) */
    UPROPS_NUMERIC_VALUE_SHIFT=8                            /*  8 (8 bits) */
};

enum {
    UBIDI_IX_MIRROR_LENGTH = 3,

    UBIDI_MAX_VALUES_INDEX=15,
    UBIDI_IX_TOP=16,

    /* UBIDI_CLASS_SHIFT=0, */     /* bidi class: 5 bits (4..0) */
    UBIDI_JT_SHIFT=5,           /* joining type: 3 bits (7..5) */

    /* UBIDI__SHIFT=8, reserved: 2 bits (9..8) */

    UBIDI_JOIN_CONTROL_SHIFT=10,
    UBIDI_BIDI_CONTROL_SHIFT=11,

    UBIDI_IS_MIRRORED_SHIFT=12,         /* 'is mirrored' */
    UBIDI_MIRROR_DELTA_SHIFT=13,        /* bidi mirroring delta: 3 bits (15..13) */

    UBIDI_MAX_JG_SHIFT=16,              /* max JG value in indexes[UBIDI_MAX_VALUES_INDEX] bits 23..16 */

    UBIDI_ESC_MIRROR_DELTA=-4,
    UBIDI_MIN_MIRROR_DELTA=-3,
    UBIDI_MAX_MIRROR_DELTA=3,

    /* the source Unicode code point takes 21 bits (20..0) */
    UBIDI_MIRROR_INDEX_SHIFT=21,
    UBIDI_MAX_MIRROR_INDEX=0x7ff
};

enum {
    /** Shift size for getting the index-1 table offset. */
    UTRIE2_SHIFT_1=6+5,

    /** Shift size for getting the index-2 table offset. */
    UTRIE2_SHIFT_2=5,

    /**
     * Difference between the two shift sizes,
     * for getting an index-1 offset from an index-2 offset. 6=11-5
     */
    UTRIE2_SHIFT_1_2=UTRIE2_SHIFT_1-UTRIE2_SHIFT_2,

    /**
     * Number of index-1 entries for the BMP. 32=0x20
     * This part of the index-1 table is omitted from the serialized form.
     */
    UTRIE2_OMITTED_BMP_INDEX_1_LENGTH=0x10000>>UTRIE2_SHIFT_1,

    /** Number of code points per index-1 table entry. 2048=0x800 */
    UTRIE2_CP_PER_INDEX_1_ENTRY=1<<UTRIE2_SHIFT_1,

    /** Number of entries in an index-2 block. 64=0x40 */
    UTRIE2_INDEX_2_BLOCK_LENGTH=1<<UTRIE2_SHIFT_1_2,

    /** Mask for getting the lower bits for the in-index-2-block offset. */
    UTRIE2_INDEX_2_MASK=UTRIE2_INDEX_2_BLOCK_LENGTH-1,

    /** Number of entries in a data block. 32=0x20 */
    UTRIE2_DATA_BLOCK_LENGTH=1<<UTRIE2_SHIFT_2,

    /** Mask for getting the lower bits for the in-data-block offset. */
    UTRIE2_DATA_MASK=UTRIE2_DATA_BLOCK_LENGTH-1,

    /**
     * Shift size for shifting left the index array values.
     * Increases possible data size with 16-bit index values at the cost
     * of compactability.
     * This requires data blocks to be aligned by UTRIE2_DATA_GRANULARITY.
     */
    UTRIE2_INDEX_SHIFT=2,

    /** The alignment size of a data block. Also the granularity for compaction. */
    UTRIE2_DATA_GRANULARITY=1<<UTRIE2_INDEX_SHIFT,

    /* Fixed layout of the first part of the index array. ------------------- */

    /**
     * The BMP part of the index-2 table is fixed and linear and starts at offset 0.
     * Length=2048=0x800=0x10000>>UTRIE2_SHIFT_2.
     */
    UTRIE2_INDEX_2_OFFSET=0,

    /**
     * The part of the index-2 table for U+D800..U+DBFF stores values for
     * lead surrogate code _units_ not code _points_.
     * Values for lead surrogate code _points_ are indexed with this portion of the table.
     * Length=32=0x20=0x400>>UTRIE2_SHIFT_2. (There are 1024=0x400 lead surrogates.)
     */
    UTRIE2_LSCP_INDEX_2_OFFSET=0x10000>>UTRIE2_SHIFT_2,
    UTRIE2_LSCP_INDEX_2_LENGTH=0x400>>UTRIE2_SHIFT_2,

    /** Count the lengths of both BMP pieces. 2080=0x820 */
    UTRIE2_INDEX_2_BMP_LENGTH=UTRIE2_LSCP_INDEX_2_OFFSET+UTRIE2_LSCP_INDEX_2_LENGTH,

    /**
     * The 2-byte UTF-8 version of the index-2 table follows at offset 2080=0x820.
     * Length 32=0x20 for lead bytes C0..DF, regardless of UTRIE2_SHIFT_2.
     */
    UTRIE2_UTF8_2B_INDEX_2_OFFSET=UTRIE2_INDEX_2_BMP_LENGTH,
    UTRIE2_UTF8_2B_INDEX_2_LENGTH=0x800>>6,  /* U+0800 is the first code point after 2-byte UTF-8 */

    /**
     * The index-1 table, only used for supplementary code points, at offset 2112=0x840.
     * Variable length, for code points up to highStart, where the last single-value range starts.
     * Maximum length 512=0x200=0x100000>>UTRIE2_SHIFT_1.
     * (For 0x100000 supplementary code points U+10000..U+10ffff.)
     *
     * The part of the index-2 table for supplementary code points starts
     * after this index-1 table.
     *
     * Both the index-1 table and the following part of the index-2 table
     * are omitted completely if there is only BMP data.
     */
    UTRIE2_INDEX_1_OFFSET=UTRIE2_UTF8_2B_INDEX_2_OFFSET+UTRIE2_UTF8_2B_INDEX_2_LENGTH,
    UTRIE2_MAX_INDEX_1_LENGTH=0x100000>>UTRIE2_SHIFT_1,

    /*
     * Fixed layout of the first part of the data array. -----------------------
     * Starts with 4 blocks (128=0x80 entries) for ASCII.
     */

    /**
     * The illegal-UTF-8 data block follows the ASCII block, at offset 128=0x80.
     * Used with linear access for single bytes 0..0xbf for simple error handling.
     * Length 64=0x40, not UTRIE2_DATA_BLOCK_LENGTH.
     */
    UTRIE2_BAD_UTF8_DATA_OFFSET=0x80,

    /** The start of non-linear-ASCII data blocks, at offset 192=0xc0. */
    UTRIE2_DATA_START_OFFSET=0xc0
};

enum {
    /**
     * At build time, leave a gap in the index-2 table,
     * at least as long as the maximum lengths of the 2-byte UTF-8 index-2 table
     * and the supplementary index-1 table.
     * Round up to UTRIE2_INDEX_2_BLOCK_LENGTH for proper compacting.
     */
    UNEWTRIE2_INDEX_GAP_OFFSET=UTRIE2_INDEX_2_BMP_LENGTH,
    UNEWTRIE2_INDEX_GAP_LENGTH=
        ((UTRIE2_UTF8_2B_INDEX_2_LENGTH+UTRIE2_MAX_INDEX_1_LENGTH)+UTRIE2_INDEX_2_MASK)&
        ~UTRIE2_INDEX_2_MASK,

    /**
     * Maximum length of the build-time index-2 array.
     * Maximum number of Unicode code points (0x110000) shifted right by UTRIE2_SHIFT_2,
     * plus the part of the index-2 table for lead surrogate code points,
     * plus the build-time index gap,
     * plus the null index-2 block.
     */
    UNEWTRIE2_MAX_INDEX_2_LENGTH=
        (0x110000>>UTRIE2_SHIFT_2)+
        UTRIE2_LSCP_INDEX_2_LENGTH+
        UNEWTRIE2_INDEX_GAP_LENGTH+
        UTRIE2_INDEX_2_BLOCK_LENGTH,

    UNEWTRIE2_INDEX_1_LENGTH=0x110000>>UTRIE2_SHIFT_1
};

enum {
    _NORM_AUX_COMP_EX_SHIFT=10,
    _NORM_AUX_UNSAFE_SHIFT=11,
    _NORM_AUX_NFC_SKIPPABLE_F_SHIFT=12,

    _NORM_INDEX_TOP=32,                  /* changing this requires a new formatVersion */

    _NORM_CC_SHIFT=8,           /* UnicodeData.txt combining class in bits 15..8 */

    _NORM_EXTRA_SHIFT=16,               /* 16 bits for the index to UChars and other extra data */
};

enum {
    TAB     =0x0009,
    CR      =0x000d,
    NL      =0x0085,
};

enum {
    UCASE_NONE,
    UCASE_LOWER,
    UCASE_UPPER,
    UCASE_TITLE
};

enum {
    UCASE_EXC_LOWER,
    UCASE_EXC_FOLD,
    UCASE_EXC_UPPER,
    UCASE_EXC_TITLE,
    UCASE_EXC_4,            /* reserved */
    UCASE_EXC_5,            /* reserved */
    UCASE_EXC_CLOSURE,
    UCASE_EXC_FULL_MAPPINGS,
    UCASE_EXC_ALL_SLOTS     /* one past the last slot */
};


typedef int32_t UTrieGetFoldingOffset(uint32_t data);

typedef struct {
    const uint16_t *index;
    const uint32_t *data32; /* NULL if 16b data is used via index */

    /**
     * This function is not used in _FROM_LEAD, _FROM_BMP, and _FROM_OFFSET_TRAIL macros.
     * If convenience macros like _GET16 or _NEXT32 are used, this function must be set.
     *
     * utrie_unserialize() sets a default function which simply returns
     * the lead surrogate's value itself - which is the inverse of the default
     * folding function used by utrie_serialize().
     *
     * @see UTrieGetFoldingOffset
     */
    UTrieGetFoldingOffset *getFoldingOffset;

    int32_t indexLength, dataLength;
    uint32_t initialValue;
    UBool isLatin1Linear;
} UTrie;

typedef struct {
    void *mem;
    const int32_t *indexes;
    const uint32_t *mirrors;
    const uint8_t *jgArray;

    UTrie trie;
    uint8_t formatVersion[4];
} UBiDiProps;

typedef struct {
    int32_t index1[UNEWTRIE2_INDEX_1_LENGTH];
    int32_t index2[UNEWTRIE2_MAX_INDEX_2_LENGTH];
    uint32_t *data;

    uint32_t initialValue, errorValue;
    int32_t index2Length, dataCapacity, dataLength;
    int32_t firstFreeBlock;
    int32_t index2NullOffset, dataNullOffset;
    uint32_t highStart;
    UBool isCompacted;

    /**
     * Multi-purpose per-data-block table.
     *
     * Before compacting:
     *
     * Per-data-block reference counters/free-block list.
     *  0: unused
     * >0: reference counter (number of index-2 entries pointing here)
     * <0: next free data block in free-block list
     *
     * While compacting:
     *
     * Map of adjusted indexes, used in compactData() and compactIndex2().
     * Maps from original indexes to new ones.
     */
    int32_t map[UNEWTRIE2_MAX_DATA_LENGTH>>UTRIE2_SHIFT_2];
} UNewTrie2;

typedef struct {
    /* protected: used by macros and functions for reading values */
    const uint16_t *index;
    const uint16_t *data16;     /* for fast UTF-8 ASCII access, if 16b data */
    const uint32_t *data32;     /* NULL if 16b data is used via index */

    int32_t indexLength, dataLength;
    uint16_t index2NullOffset;  /* 0xffff if there is no dedicated index-2 null block */
    uint16_t dataNullOffset;
    uint32_t initialValue;
    /** Value returned for out-of-range code points and illegal UTF-8. */
    uint32_t errorValue;

    /* Start of the last range which ends at U+10ffff, and its value. */
    uint32_t highStart;
    int32_t highValueIndex;

    /* private: used by builder and unserialization functions */
    void *memory;           /* serialized bytes; NULL if not frozen yet */
    int32_t length;         /* number of serialized bytes at memory; 0 if not frozen yet */
    UBool isMemoryOwned;    /* TRUE if the trie owns the memory */
    UBool padding1;
    int16_t padding2;
    UNewTrie2 *newTrie;     /* builder object; NULL when frozen */
} UTrie2;

typedef struct {
    void *mem;
    const int32_t *indexes;
    const uint16_t *exceptions;
    const uint16_t *unfold;

    UTrie2 trie;
    uint8_t formatVersion[4];
} UCaseProps;

static int32_t utrie_defaultGetFoldingOffset(uint32_t data) {
    return (int32_t)data;
}

static int32_t getFoldingNormOffset(uint32_t norm32) {
    if (_NORM_MIN_SPECIAL<=norm32 && norm32<_NORM_SURROGATES_TOP) {
        return
            UTRIE_BMP_INDEX_LENGTH+
                (((int32_t)norm32>>(_NORM_EXTRA_SHIFT-UTRIE_SURROGATE_BLOCK_BITS))&
                 (0x3ff<<UTRIE_SURROGATE_BLOCK_BITS));
    } else {
        return 0;
    }
}

static int32_t getFoldingAuxOffset(uint32_t data) {
    return (int32_t)(data&_NORM_AUX_FNC_MASK)<<UTRIE_SURROGATE_BLOCK_BITS;
}

#include "uchar_props_data.c"
#include "ubidi_props_data.c"
#include "unorm_props_data.c"
#include "ucase_props_data.c"

static uint32_t u_getUnicodeProperties(UChar32 c, int32_t column) {
    uint16_t vecIndex;

    if (column == -1) {
        uint32_t props;
        GET_PROPS(c, props);
        return props;
    } else if (column < 0 || column >= propsVectorsColumns) {
        return 0;
    } else {
        UTRIE_GET16(&charPropsVectorsTrie, c, vecIndex);
        return charPropsVectors[vecIndex+column];
    }
}

static const uint8_t flagsOffset[256]={
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

// Functions

using namespace WTF::Unicode;

bool icuIsLower(UChar32 c)
{
    uint32_t props;
    GET_PROPS(c, props);
    return (props & 0x1f) == U_LOWERCASE_LETTER;
}

bool icuIsUpper(UChar32 c)
{
    uint32_t props;
    GET_PROPS(c, props);
    return (props & 0x1f) == U_UPPERCASE_LETTER;
}

bool icuIsPrintableChar(UChar32 c)
{
    uint32_t props;
    GET_PROPS(c, props);
    return (CAT_MASK(props) & U_GC_C_MASK) == 0;
}

bool icuIsSpace(UChar32 c)
{
    uint32_t props;
    GET_PROPS(c, props);
    return (CAT_MASK(props) & U_GC_Z_MASK) != 0 || IS_THAT_CONTROL_SPACE(c);
}

bool icuIsLetter(UChar32 c)
{
    uint32_t props;
    GET_PROPS(c, props);
    return (CAT_MASK(props) & U_GC_L_MASK) != 0;
}

bool icuIsDigit(UChar32 c)
{
    uint32_t props;
    GET_PROPS(c, props);
    return (props & 0x1f) == U_DECIMAL_DIGIT_NUMBER;
}

bool icuIsPunct(UChar32 c)
{
    uint32_t props;
    GET_PROPS(c, props);
    return (CAT_MASK(props) & U_GC_P_MASK) != 0;
}

Direction icuDirection(UChar32 c)
{
    uint32_t props;
    GET_BIDI_PROPS(&ubidi_props_singleton, c, props);
    return static_cast<Direction>(UBIDI_GET_CLASS(props));
}

CharCategory icuCategory(unsigned int c)
{
    uint32_t props;
    GET_PROPS(c, props);
    return static_cast<CharCategory>(TO_MASK((int8_t)(props & 0x1f)));
}

DecompositionType icuDecompositionType(UChar32 c)
{
    return static_cast<DecompositionType>(u_getUnicodeProperties(c, 2)&UPROPS_DT_MASK);
}

unsigned char icuCombiningClass(UChar32 c)
{
    uint32_t norm32;

    UTRIE_GET32(&normTrie, c, norm32);
    return (uint8_t)(norm32 >> _NORM_CC_SHIFT);
}

UChar32 icuMirroredChar(UChar32 c)
{
    const UBiDiProps *bdp = &ubidi_props_singleton;

    uint32_t props;
    int32_t delta;

    GET_BIDI_PROPS(bdp, c, props);
    delta = ((int16_t)props) >> UBIDI_MIRROR_DELTA_SHIFT;
    if (delta != UBIDI_ESC_MIRROR_DELTA) {
        return c+delta;
    } else {
        /* look for mirror code point in the mirrors[] table */
        const uint32_t *mirrors = bdp->mirrors;
        int32_t length = bdp->indexes[UBIDI_IX_MIRROR_LENGTH];

        /* linear search */
        for (int32_t i = 0; i < length; ++i) {
            uint32_t m = mirrors[i];
            UChar32 c2 = UBIDI_GET_MIRROR_CODE_POINT(m);
            if (c == c2) {
                /* found c, return its mirror code point using the index in m */
                return UBIDI_GET_MIRROR_CODE_POINT(mirrors[UBIDI_GET_MIRROR_INDEX(m)]);
            } else if (c < c2) {
                break;
            }
        }

        /* c not found, return it itself */
        return c;
    }
}

int icuDigitValue(UChar32 c)
{
    uint32_t props;
    GET_PROPS(c, props);

    if (((props >> UPROPS_NUMERIC_TYPE_SHIFT) & 7) == 1) {
        return (props >> UPROPS_NUMERIC_VALUE_SHIFT) & 0xff;
    } else {
        return -1;
    }
}


UChar32 icuToLower(UChar32 c)
{
    uint16_t props = UTRIE2_GET16(&ucase_props_singleton.trie, c);
    if (!PROPS_HAS_EXCEPTION(props)) {
        if(UCASE_GET_TYPE(props)>=UCASE_UPPER) {
            c += UCASE_GET_DELTA(props);
        }
    } else {
        const uint16_t* pe = GET_EXCEPTIONS(&ucase_props_singleton, props);
        uint16_t excWord = *pe++;
        if (HAS_SLOT(excWord, UCASE_EXC_LOWER)) {
            GET_SLOT_VALUE(excWord, UCASE_EXC_LOWER, pe, c);
        }
    }
    return c;
}

UChar32 icuToUpper(UChar32 c)
{
    uint16_t props = UTRIE2_GET16(&ucase_props_singleton.trie, c);
    if (!PROPS_HAS_EXCEPTION(props)) {
        if (UCASE_GET_TYPE(props) == UCASE_LOWER) {
            c += UCASE_GET_DELTA(props);
        }
    } else {
        const uint16_t* pe = GET_EXCEPTIONS(&ucase_props_singleton, props);
        uint16_t excWord = *pe++;
        if (HAS_SLOT(excWord, UCASE_EXC_UPPER))
            GET_SLOT_VALUE(excWord, UCASE_EXC_UPPER, pe, c);
    }
    return c;
}

UChar32 icuFoldCase(UChar32 c)
{
    uint16_t props = UTRIE2_GET16(&ucase_props_singleton.trie, c);
    if (!PROPS_HAS_EXCEPTION(props)) {
        if (UCASE_GET_TYPE(props) >= UCASE_UPPER)
            c += UCASE_GET_DELTA(props);
    } else {
        const uint16_t* pe = GET_EXCEPTIONS(&ucase_props_singleton, props);
        uint16_t excWord = *pe++;
        int32_t idx;
        if (excWord & UCASE_EXC_CONDITIONAL_FOLD) {
            /* special case folding mappings, hardcoded */
            // FIXME: to check if turkic mapping is required.
            if (true) {
                /* default mappings */
                if (c == 0x49) {
                    /* 0049; C; 0069; # LATIN CAPITAL LETTER I */
                    return 0x69;
                } else if (c == 0x130) {
                    /* no simple case folding for U+0130 */
                    return c;
                }
            } else {
                /* Turkic mappings */
                if (c == 0x49) {
                    /* 0049; T; 0131; # LATIN CAPITAL LETTER I */
                    return 0x131;
                } else if (c == 0x130) {
                    /* 0130; T; 0069; # LATIN CAPITAL LETTER I WITH DOT ABOVE */
                    return 0x69;
                }
            }
        }
        if (HAS_SLOT(excWord, UCASE_EXC_FOLD)) {
            idx = UCASE_EXC_FOLD;
        } else if (HAS_SLOT(excWord, UCASE_EXC_LOWER)) {
            idx = UCASE_EXC_LOWER;
        } else {
            return c;
        }
        GET_SLOT_VALUE(excWord, idx, pe, c);
    }
    return c;
}

UChar32 icuToTitleCase(UChar32 c)
{
    uint16_t props = UTRIE2_GET16(&ucase_props_singleton.trie, c);
    if (!PROPS_HAS_EXCEPTION(props)) {
        if (UCASE_GET_TYPE(props) == UCASE_LOWER)
            c += UCASE_GET_DELTA(props);
    } else {
        const uint16_t *pe = GET_EXCEPTIONS(&ucase_props_singleton, props);
        uint16_t excWord = *pe++;
        int32_t idx;
        if (HAS_SLOT(excWord, UCASE_EXC_TITLE)) {
            idx = UCASE_EXC_TITLE;
        } else if(HAS_SLOT(excWord, UCASE_EXC_UPPER)) {
            idx = UCASE_EXC_UPPER;
        } else {
            return c;
        }
        GET_SLOT_VALUE(excWord, idx, pe, c);
    }
    return c;
}

#endif // !USE(OLYMPIA_NATIVE_UNICODE)
