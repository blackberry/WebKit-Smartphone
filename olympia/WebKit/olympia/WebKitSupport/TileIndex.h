/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef TileIndex_h
#define TileIndex_h

#include <limits>

namespace Olympia {
    namespace WebKit {

        class TileIndex {
        public:
            TileIndex()
                : m_i(std::numeric_limits<unsigned int>::max())
                , m_j(std::numeric_limits<unsigned int>::max()) {}
            TileIndex(unsigned int i, unsigned int j)
                : m_i(i)
                , m_j(j) {}
            ~TileIndex() {}

            unsigned int i() const { return m_i; }
            unsigned int j() const { return m_j; }
            void setIndex(unsigned int i, unsigned int j)
            {
                m_i = i;
                m_j = j;
            }

        private:
            bool m_isValid;
            unsigned int m_i;
            unsigned int m_j;
        };

        inline bool operator==(const Olympia::WebKit::TileIndex& a, const Olympia::WebKit::TileIndex& b)
        {
            return a.i() == b.i() && a.j() == b.j();
        }

        inline bool operator!=(const Olympia::WebKit::TileIndex& a, const Olympia::WebKit::TileIndex& b)
        {
            return a.i() != b.i() || a.j() != b.j();
        }
    }
}

#endif // TileIndex_h
