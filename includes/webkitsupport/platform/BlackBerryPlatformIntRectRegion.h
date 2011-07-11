/*
 * Copyright (C) Research In Motion Limited 2010-2011. All rights reserved.
 * Copyright 1987, 1988, 1998 The Open Group
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of The Open Group shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from The Open Group.
 *
 *
 * Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts.
 *
 *                            All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of Digital not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
 * ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
 * DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
 * ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#ifndef BlackBerryPlatformIntRectRegion_h
#define BlackBerryPlatformIntRectRegion_h

#include "BlackBerryPlatformPrimitives.h"
#include <vector>

namespace BlackBerry {
namespace Platform {

// The functions in this file implement the Region abstraction used extensively
// throughout the X11 sample server. A Region is simply a set of disjoint
// (non-overlapping) rectangles, plus an "extent" rectangle which is the
// smallest single rectangle that contains all the non-overlapping rectangles.
//
// A Region is implemented as a "y-x-banded" array of rectangles.  This array
// imposes two degrees of order.  First, all rectangles are sorted by top side
// y coordinate first (y1), and then by left side x coordinate (x1).
//
// Furthermore, the rectangles are grouped into "bands".  Each rectangle in a
// band has the same top y coordinate (y1), and each has the same bottom y
// coordinate (y2).  Thus all rectangles in a band differ only in their left
// and right side (x1 and x2).  Bands are implicit in the array of rectangles:
// there is no separate list of band start pointers.
//
// The y-x band representation does not minimize rectangles.  In particular,
// if a rectangle vertically crosses a band (the rectangle has scan-lines in
// the y1 to y2 area spanned by the band), then the rectangle may be broken
// down into two or more smaller rectangles stacked one atop the other.
//
//  -----------                             -----------
//  |         |                             |         |             band 0
//  |         |  --------                   -----------  --------
//  |         |  |      |  in y-x banded    |         |  |      |   band 1
//  |         |  |      |  form is          |         |  |      |
//  -----------  |      |                   -----------  --------
//               |      |                                |      |   band 2
//               --------                                --------
//
// An added constraint on the rectangles is that they must cover as much
// horizontal area as possible: no two rectangles within a band are allowed
// to touch.
//
// Whenever possible, bands will be merged together to cover a greater vertical
// distance (and thus reduce the number of rectangles). Two bands can be merged
// only if the bottom of one touches the top of the other and they have
// rectangles in the same places (of the same width, of course).
class IntRectRegion {
public:
    enum IntersectionState { NotInRegion, ContainedInRegion, PartiallyContainedInRegion };

    IntRectRegion();
    IntRectRegion(const IntRect&);
    IntRectRegion(const IntRectRegion&);
    IntRectRegion(const IntRect& extents, const unsigned numRects, const std::vector<IntRect> rects);

    ~IntRectRegion();

    bool isEmpty() const { return !m_numRects; }

    unsigned numRects() const { return m_numRects; }

    std::vector<IntRect> rects() const;

    IntRect extents() const { return m_extents; }

    bool isEqual(const IntRectRegion&) const;

    IntersectionState isRectInRegion(const IntRect&) const;

    static IntRectRegion subtractRegions(const IntRectRegion& regM, const IntRectRegion& regS);

    static IntRectRegion intersectRegions(const IntRectRegion& regM, const IntRectRegion& regS);

    static IntRectRegion unionRegions(const IntRectRegion& regM, const IntRectRegion& regS);

private:

    typedef void (*OverlappingBandsFunctionPtr)(IntRectRegion*, const IntRect* startOfFirstBand, const IntRect* endOfFirstBand,
                                                const IntRect* startOfSecondBand, const IntRect* endOfSecondBand,
                                                int yTop, int yBottom);

    typedef void (*NonOverlappingBandsFunctionPtr)(IntRectRegion*, const IntRect* startOfBand, const IntRect* endOfBand, int yTop, int yBottom);

    static IntersectionState rectInRegion(const IntRectRegion* region, int rx, int ry, int rwidth, int rheight);

    static void miSetExtents(IntRectRegion*);

    static void miRegionOp(IntRectRegion*, const IntRectRegion* reg1, const IntRectRegion* reg2,
                    OverlappingBandsFunctionPtr, NonOverlappingBandsFunctionPtr = 0, NonOverlappingBandsFunctionPtr = 0);

    static int miCoalesce(IntRectRegion*, int prevStart, int curStart);

    static void miSubtractO(IntRectRegion*, const IntRect* r1, const IntRect* r1End, const IntRect* r2, const IntRect* r2End, int y1, int y2);

    static void miSubtractNonO1(IntRectRegion*, const IntRect* r, const IntRect* rEnd, int y1, int y2);

    static void miIntersectO(IntRectRegion*, const IntRect* r1, const IntRect* r1End, const IntRect* r2, const IntRect* r2End, int y1, int y2);

    static void miUnionNonO(IntRectRegion*, const IntRect* r, const IntRect* rEnd, int y1, int y2);

    static void miUnionO(IntRectRegion*, const IntRect* r1, const IntRect* r1End, const IntRect* r2, const IntRect* r2End, int y1, int y2);

    static void mergeRect(IntRectRegion*, IntRect*& pNextRect, const IntRect*& r, int y1, int y2);

    static void checkIfNearCapacityAndDoubleCapacity(IntRectRegion*, IntRect*& newEnd);

    friend IntRectRegion subtractRegions(const IntRectRegion& firstRegion, const IntRectRegion& secondRegion);
    friend IntRectRegion operator - (const IntRectRegion& firstRegion, const IntRectRegion& secondRegion)
    {
        return subtractRegions(firstRegion, secondRegion);
    }

    friend IntRectRegion intersectRegions(const IntRectRegion& firstRegion, const IntRectRegion& secondRegion);
    friend IntRectRegion unionRegions(const IntRectRegion& firstRegion, const IntRectRegion& secondRegion);

    unsigned m_numRects;
    std::vector<IntRect> m_rects;
    IntRect m_extents;

};

#ifndef NDEBUG
    void testIntRectRegion();
#endif


} // Platform
} // BlackBerry

#endif // BlackBerryPlatformIntRectRegion_h
