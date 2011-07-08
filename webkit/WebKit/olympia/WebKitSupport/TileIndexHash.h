/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef TileIndexHash_h
#define TileIndexHash_h

#include "TileIndex.h"
#include <limits>
#include <wtf/HashMap.h>
#include <wtf/HashSet.h>

using Olympia::WebKit::TileIndex;

namespace WTF {

    template<> struct IntHash<TileIndex> {
        static unsigned hash(const TileIndex& key) { return intHash((static_cast<uint64_t>(key.i()) << 32 | key.j())); }
        static bool equal(const TileIndex& a, const TileIndex& b) { return a == b; }
        static const bool safeToCompareToEmptyOrDeleted = true;
    };
    template<> struct DefaultHash<TileIndex> { typedef IntHash<TileIndex> Hash; };

    template<> struct HashTraits<TileIndex> : GenericHashTraits<TileIndex> {
        static const bool emptyValueIsZero = false;
        static const bool needsDestruction = false;
        static TileIndex emptyValue() { return TileIndex(); }
        static void constructDeletedValue(TileIndex& slot)
        {
            new (&slot) TileIndex(std::numeric_limits<unsigned int>::max() - 1,
                                  std::numeric_limits<unsigned int>::max() - 1);
        }
        static bool isDeletedValue(const TileIndex& value)
        {
            return value.i() == (std::numeric_limits<unsigned int>::max() - 1)
                   && value.j() == (std::numeric_limits<unsigned int>::max() - 1);
        }
    };
} // namespace WTF

#endif // TileIndexHash_h
