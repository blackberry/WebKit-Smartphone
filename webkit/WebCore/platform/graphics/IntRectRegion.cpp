/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#include "config.h"
#include "IntRectRegion.h"
#include <stdio.h>

namespace WebCore {

IntRectRegion subtractRegions(const IntRectRegion& firstRegion, const IntRectRegion& secondRegion)
{
    return IntRectRegion::subtractRegions(firstRegion, secondRegion);
}

IntRectRegion intersectRegions(const IntRectRegion& firstRegion, const IntRectRegion& secondRegion)
{
    return IntRectRegion::intersectRegions(firstRegion, secondRegion);
}

IntRectRegion unionRegions(const IntRectRegion& firstRegion, const IntRectRegion& secondRegion)
{
    return IntRectRegion::unionRegions(firstRegion, secondRegion);
}

IntRectRegion::IntRectRegion()
    : m_numRects(0)
{
}

IntRectRegion::IntRectRegion(const IntRect& rect)
{
    if (rect.isEmpty())
        m_numRects = 0;
    else {
        m_numRects = 1;
        m_extents = rect;
        m_rects.append(m_extents);
    }
}

IntRectRegion::~IntRectRegion()
{
}

void IntRectRegion::checkIfNearCapacityAndDoubleCapacity(IntRectRegion* region, IntRect*& newEnd)
{
    if (region->m_numRects < region->m_rects.size() - 1)
        return;

    region->m_rects.resize(2 * region->m_rects.size());
    newEnd = &region->m_rects[region->m_numRects];
}

IntRectRegion::IntRectRegion(const IntRectRegion& region)
{
    m_extents = IntRect(region.m_extents);
    m_rects = region.m_rects;
    m_numRects = region.m_numRects;
}

Vector<IntRect> IntRectRegion::rects() const
{
    Vector<IntRect> result;
    for (unsigned i = 0; i < m_numRects; ++i)
        result.append(m_rects[i]);
    return result;
}

IntRectRegion IntRectRegion::subtractRegions(const IntRectRegion& regM, const IntRectRegion& regS)
{
    IntRectRegion result;

    if (regM.isEmpty() || regS.isEmpty())
        return regM;
    if (!regM.m_extents.intersects(regS.m_extents))
        return regM;

    miRegionOp(&result, &regM, &regS, &miSubtractO, &miSubtractNonO1);
    miSetExtents(&result);
    return result;
}

IntRectRegion IntRectRegion::intersectRegions(const IntRectRegion& regM, const IntRectRegion& regS)
{
    IntRectRegion result;
    if (regM.isEmpty() || regS.isEmpty()) {
        result.m_numRects = 0;
        return result;
    }

    if (!regM.m_extents.intersects(regS.m_extents)) {
        result.m_numRects = 0;
        return result;
    }

    miRegionOp(&result, &regM, &regS, &miIntersectO);
    miSetExtents(&result);
    return result;
}

IntRectRegion IntRectRegion::unionRegions(const IntRectRegion& regM, const IntRectRegion& regS)
{
    // Region 1 and 2 are the same or region 1 is empty
    if (regM.isEmpty())
        return regS;

    // if nothing to union (region 2 empty)
    if (regS.isEmpty())
        return regM;

    // Region 1 completely subsumes region 2
    if (regM.m_numRects == 1 && regM.m_extents.contains(regS.m_extents))
        return regM;

    // Region 2 completely subsumes region 1
    if (regS.m_numRects == 1 && regS.m_extents.contains(regM.m_extents))
        return regS;

    IntRectRegion result;
    miRegionOp(&result, &regM, &regS, &miUnionO, &miUnionNonO, &miUnionNonO);

    result.m_extents.setX(std::min(regM.m_extents.x(), regS.m_extents.x()));
    result.m_extents.setY(std::min(regM.m_extents.y(), regS.m_extents.y()));
    result.m_extents.setRight(std::max(regM.m_extents.right(), regS.m_extents.right()));
    result.m_extents.setBottom(std::max(regM.m_extents.bottom(), regS.m_extents.bottom()));
    return result;
}

void IntRectRegion::miSetExtents(IntRectRegion* pReg)
{
    IntRect* pBox;
    IntRect* pBoxEnd;
    IntRect* pExtents;

    if (pReg->isEmpty()) {
        pReg->m_extents = IntRect();
        return;
    }

    pExtents = &pReg->m_extents;
    pBox = pReg->m_rects.data();
    pBoxEnd = &pBox[pReg->m_numRects - 1];

    // Since pBox is the first rectangle in the region, it must have the
    // smallest y1 and since pBoxEnd is the last rectangle in the region,
    // it must have the largest y2, because of banding. Initialize x1 and
    // x2 from  pBox and pBoxEnd, resp., as good things to initialize them
    // to...
    pExtents->setLocation(IntPoint(pBox->x(), pBox->y()));
    pExtents->setRight(pBoxEnd->right());
    pExtents->setBottom(pBoxEnd->bottom());

    ASSERT(pExtents->y() < pExtents->bottom());
    while (pBox <= pBoxEnd) {
        if (pBox->x() < pExtents->x())
            pExtents->setX(pBox->x());
        if (pBox->right() > pExtents->right())
            pExtents->setRight(pBox->right());
        ++pBox;
    }
    ASSERT(pExtents->x() < pExtents->right());
}

bool IntRectRegion::isEqual(const IntRectRegion& region) const
{
    if (m_numRects != region.m_numRects)
        return false;

    if (!m_numRects)
        return true;

    if (m_extents != region.m_extents)
        return false;

    if (m_numRects == 1 && region.m_numRects == 1)
        return true;

    for (size_t i = 0; i < m_numRects; ++i) {
        if (m_rects[i] != region.m_rects[i])
            return false;
    }
    return true;
}

IntRectRegion::IntersectionState IntRectRegion::isRectInRegion(const IntRect& rect) const
{
    return rectInRegion(this, rect.x(), rect.y(), rect.width(), rect.height());
}

IntRectRegion::IntersectionState IntRectRegion::rectInRegion(const IntRectRegion* region, int rx, int ry, int rwidth, int rheight)
{
    IntRect rect(rx, ry, rwidth, rheight);

    // This is (just) a useful optimization
    if (!region || region->isEmpty() || !region->m_extents.intersects(rect))
        return NotInRegion;

    bool partOut = false;
    bool partIn = false;

    // Can stop when both partOut and partIn are true, or we reach rect.bottom()
    const IntRect* pbox = region->m_numRects == 1 ? &region->m_extents : region->m_rects.data();
    const IntRect* pboxEnd = pbox + region->m_numRects;
    for (; pbox < pboxEnd; ++pbox) {
        if (pbox->bottom() <= ry)
            continue; // Getting up to speed or skipping remainder of band

        if (pbox->y() > ry) {
            partOut = true; // Missed part of rectangle above
            if (partIn || (pbox->y() >= rect.bottom()))
                break;
            ry = pbox->y(); // X guaranteed to be == rect.x()
        }

        if (pbox->right() <= rx)
            continue; // Not far enough over yet

        if (pbox->x() > rx) {
            partOut = true; // Missed part of rectangle to left
            if (partIn)
                break;
        }

        if (pbox->x() < rect.right()) {
            partIn = true; // Definitely overlap
            if (partOut)
                break;
        }

        if (pbox->right() >= rect.right()) {
            ry = pbox->bottom(); // Finished with this band
            if (ry >= rect.bottom())
                break;
            rx = rect.x(); // Reset x out to left again
        } else {
            // Because boxes in a band are maximal width, if the first box
            // to overlap the rectangle doesn't completely cover it in that
            // band, the rectangle must be partially out, since some of it
            // will be uncovered in that band. partIn will have been set true
            // by now...
            break;
        }
    }

    return partIn ? ((ry < rect.bottom()) ? PartiallyContainedInRegion : ContainedInRegion) : NotInRegion;
}

int IntRectRegion::miCoalesce(IntRectRegion* pReg /* Region to coalesce */, int prevStart /* Index of start of previous band */, int curStart /* Index of start of current band */)
{
    IntRect* pPrevBox; // Current box in previous band
    IntRect* pCurBox; // Current box in current band
    IntRect* pRegEnd; // End of region
    int curNumRects; // Number of rectangles in current band
    int prevNumRects; // Number of rectangles in previous band
    int bandY1; // Y1 coordinate for current band

    pRegEnd = &pReg->m_rects[pReg->m_numRects];

    pPrevBox = &pReg->m_rects[prevStart];
    prevNumRects = curStart - prevStart;

    // Figure out how many rectangles are in the current band. Have to do
    // this because multiple bands could have been added in miRegionOp
    // at the end when one region has been exhausted.
    pCurBox = &pReg->m_rects[curStart];
    bandY1 = pCurBox->y();
    for (curNumRects = 0; pCurBox != pRegEnd && pCurBox->y() == bandY1; ++curNumRects)
        ++pCurBox;

    if (pCurBox != pRegEnd) {
        // If more than one band was added, we have to find the start
        // of the last band added so the next coalescing job can start
        // at the right place... (given when multiple bands are added,
        // this may be pointless -- see above).
        --pRegEnd;
        while (pRegEnd[-1].y() == pRegEnd->y())
            --pRegEnd;
        curStart = pRegEnd - pReg->m_rects.data();
        pRegEnd = pReg->m_rects.data() + pReg->m_numRects;
    }

    if (curNumRects == prevNumRects && curNumRects) {
        pCurBox -= curNumRects;
        // The bands may only be coalesced if the bottom of the previous
        // matches the y() scanline of the current.
        if (pPrevBox->bottom() == pCurBox->y()) {
            // Make sure the bands have boxes in the same places. This
            // assumes that boxes have been added in such a way that they
            // cover the most area possible. I.e. two boxes in a band must
            // have some horizontal space between them.
            do {
                if (pPrevBox->x() != pCurBox->x() || pPrevBox->right() != pCurBox->right())
                    return curStart; // The bands don't line up so they can't be coalesced.
                ++pPrevBox;
                ++pCurBox;
                --prevNumRects;
            } while (prevNumRects);

            pReg->m_numRects -= curNumRects;
            pCurBox -= curNumRects;
            pPrevBox -= curNumRects;

            // The bands may be merged, so set the bottom y of each box
            // in the previous band to that of the corresponding box in
            // the current band.
            do {
                pPrevBox->setBottom(pCurBox->bottom());
                ++pPrevBox;
                ++pCurBox;
                --curNumRects;
            } while (curNumRects);

            // If only one band was added to the region, we have to backup
            // curStart to the start of the previous band.
            // If more than one band was added to the region, copy the
            // other bands down. The assumption here is that the other bands
            // came from the same region as the current one and no further
            // coalescing can be done on them since it's all been done
            // already... curStart is already in the right place.
            if (pCurBox == pRegEnd)
                curStart = prevStart;
            else
                std::memcpy(pPrevBox, pCurBox, (pRegEnd - pCurBox) * sizeof(IntRect));
        }
    }
    return curStart;
}

void IntRectRegion::miRegionOp(IntRectRegion* newReg, const IntRectRegion* reg1, const IntRectRegion* reg2,
                       OverlappingBandsFunctionPtr overlapFunc,
                       NonOverlappingBandsFunctionPtr nonOverlap1Func,
                       NonOverlappingBandsFunctionPtr nonOverlap2Func)
{
    const IntRect* r1; // Pointer into first region
    const IntRect* r2; // Pointer into 2d region
    const IntRect* r1End; // End of 1st region
    const IntRect* r2End; // End of 2d region
    int ybot; // Bottom of intersection
    int ytop; // Top of intersection
    int prevBand; // Index of start of previous band in newReg
    unsigned curBand; // Index of start of current band in newReg
    const IntRect* r1BandEnd; // End of current band in r1
    const IntRect* r2BandEnd; // End of current band in r2
    int top; // Top of non-overlapping band
    int bot; // Bottom of non-overlapping band

    // Initialization:
    // set r1, r2, r1End and r2End appropriately, preserve the important
    // parts of the destination region until the end in case it's one of
    // the two source regions, then mark the "new" region empty, allocating
    // another array of rectangles for it to use.
    r1 = reg1->m_rects.data();
    r2 = reg2->m_rects.data();
    r1End = r1 + reg1->m_numRects;
    r2End = r2 + reg2->m_numRects;

    newReg->m_numRects = 0; // Clear the region.

    // Allocate a reasonable number of rectangles for the new region. The idea
    // is to allocate enough so the individual functions don't need to
    // reallocate and copy the array, which is time consuming, yet we don't
    // have to worry about using too much memory. I hope to be able to
    // nuke the Xrealloc() at the end of this function eventually.
    newReg->m_rects.resize(std::max(reg1->m_numRects, reg2->m_numRects) * 2);

    // Initialize ybot and ytop.
    // In the upcoming loop, ybot and ytop serve different functions depending
    // on whether the band being handled is an overlapping or non-overlapping
    // band.
    //     In the case of a non-overlapping band (only one of the regions
    // has points in the band), ybot is the bottom of the most recent
    // intersection and thus clips the top of the rectangles in that band.
    // ytop is the top of the next intersection between the two regions and
    // serves to clip the bottom of the rectangles in the current band.
    // For an overlapping band (where the two regions intersect), ytop clips
    // the top of the rectangles of both regions and ybot clips the bottoms.
    if (reg1->m_extents.y() < reg2->m_extents.y())
        ybot = reg1->m_extents.y();
    else
        ybot = reg2->m_extents.y();

    // prevBand serves to mark the start of the previous band so rectangles
    // can be coalesced into larger rectangles. qv. miCoalesce, above.
    // In the beginning, there is no previous band, so prevBand == curBand
    // (curBand is set later on, of course, but the first band will always
    // start at index 0). prevBand and curBand must be indices because of
    // the possible expansion, and resultant moving, of the new region's
    // array of rectangles.
    prevBand = 0;

    do {
        curBand = newReg->m_numRects;

        // This algorithm proceeds one source-band (as opposed to a
        // destination band, which is determined by where the two regions
        // intersect) at a time. r1BandEnd and r2BandEnd serve to mark the
        // rectangle after the last one in the current band for their
        // respective regions.
        r1BandEnd = r1;
        while (r1BandEnd != r1End && r1BandEnd->y() == r1->y())
            ++r1BandEnd;

        r2BandEnd = r2;
        while (r2BandEnd != r2End && r2BandEnd->y() == r2->y())
            ++r2BandEnd;

        // First handle the band that doesn't intersect, if any.
        //
        // Note that attention is restricted to one band in the
        // non-intersecting region at once, so if a region has n
        // bands between the current position and the next place it overlaps
        // the other, this entire loop will be passed through n times.
        if (r1->y() < r2->y()) {
            top = std::max(r1->y(), ybot);
            bot = std::min(r1->bottom(), r2->y());

            if (top != bot && nonOverlap1Func)
                (*nonOverlap1Func)(newReg, r1, r1BandEnd, top, bot);

            ytop = r2->y();
        } else if (r2->y() < r1->y()) {
            top = std::max(r2->y(), ybot);
            bot = std::min(r2->bottom(), r1->y());

            if (top != bot && nonOverlap2Func)
                (*nonOverlap2Func)(newReg, r2, r2BandEnd, top, bot);

            ytop = r1->y();
        } else
            ytop = r1->y();

        // If any rectangles got added to the region, try and coalesce them
        // with rectangles from the previous band. Note we could just do
        // this test in miCoalesce, but some machines incur a not
        // inconsiderable cost for function calls, so...
        if (newReg->m_numRects != curBand)
            prevBand = miCoalesce(newReg, prevBand, curBand);

        // Now see if we've hit an intersecting band. The two bands only
        // intersect if ybot > ytop
        ybot = std::min(r1->bottom(), r2->bottom());
        curBand = newReg->m_numRects;
        if (ybot > ytop)
            (*overlapFunc)(newReg, r1, r1BandEnd, r2, r2BandEnd, ytop, ybot);

        if (newReg->m_numRects != curBand)
            prevBand = miCoalesce(newReg, prevBand, curBand);

        // If we've finished with a band (y2 == ybot) we skip forward
        // in the region to the next band.
        if (r1->bottom() == ybot)
            r1 = r1BandEnd;
        if (r2->bottom() == ybot)
            r2 = r2BandEnd;
    } while (r1 != r1End && r2 != r2End);

    // Deal with whichever region still has rectangles left.
    curBand = newReg->m_numRects;
    if (r1 != r1End && nonOverlap1Func) {
        do {
            r1BandEnd = r1;
            while (r1BandEnd < r1End && r1BandEnd->y() == r1->y())
                ++r1BandEnd;
            (*nonOverlap1Func)(newReg, r1, r1BandEnd, std::max(r1->y(), ybot), r1->bottom());
            r1 = r1BandEnd;
        } while (r1 != r1End);
    } else if (r2 != r2End && nonOverlap2Func) {
        do {
            r2BandEnd = r2;
            while (r2BandEnd < r2End && r2BandEnd->y() == r2->y())
                ++r2BandEnd;
            (*nonOverlap2Func)(newReg, r2, r2BandEnd, std::max(r2->y(), ybot), r2->bottom());
            r2 = r2BandEnd;
        } while (r2 != r2End);
    }

    if (newReg->m_numRects != curBand)
        miCoalesce(newReg, prevBand, curBand);

    // A bit of cleanup. To keep regions from growing without bound,
    // we shrink the array of rectangles to match the new number of
    // rectangles in the region. This never goes to 0, however...
    //
    // Only do this stuff if the number of rectangles allocated is more than
    // twice the number of rectangles in the region (a simple optimization...).
    if (newReg->m_numRects < newReg->m_rects.size() / 2) {
        if (!newReg->isEmpty()) {
            // The region is not empty.
            newReg->m_rects.resize(newReg->m_numRects);
        } else {
            // No point in doing the extra work involved in an Xrealloc if
            // the region is empty
            newReg->m_rects.resize(1);
        }
    }
}

void IntRectRegion::miSubtractO(IntRectRegion* pReg, const IntRect* r1, const IntRect* r1End, const IntRect* r2, const IntRect* r2End, int y1, int y2)
{
    IntRect* pNextRect;
    int x1;

    x1 = r1->x();

    ASSERT(y1 < y2);
    pNextRect = &pReg->m_rects[pReg->m_numRects];

    while (r1 != r1End && r2 != r2End) {
        if (r2->right() <= x1) {
            // Subtrahend missed the boat: go to next subtrahend.
            ++r2;
        } else if (r2->x() <= x1) {
            // Subtrahend preceeds minuend: nuke left edge of minuend.
            x1 = r2->right();
            if (x1 >= r1->right()) {
                // Minuend completely covered: advance to next minuend and
                // reset left fence to edge of new minuend.
                ++r1;
                if (r1 != r1End)
                    x1 = r1->x();
            } else {
                // Subtrahend now used up since it doesn't extend beyond
                // minuend
                ++r2;
            }
        } else if (r2->x() < r1->right()) {
            // Left part of subtrahend covers part of minuend: add uncovered
            // part of minuend to region and skip to next subtrahend.
            ASSERT(x1 < r2->x());
            checkIfNearCapacityAndDoubleCapacity(pReg, pNextRect);
            pNextRect->setLocation(IntPoint(x1, y1));
            pNextRect->setRight(r2->x());
            pNextRect->setBottom(y2);
            ++pReg->m_numRects;
            ++pNextRect;

            ASSERT(pReg->m_numRects <= pReg->m_rects.size());

            x1 = r2->right();
            if (x1 >= r1->right()) {
                // Minuend used up: advance to new...
                ++r1;
                if (r1 != r1End)
                    x1 = r1->x();
            } else {
                // Subtrahend used up
                ++r2;
            }
        } else {
            // Minuend used up: add any remaining piece before advancing.
            if (r1->right() > x1) {
                checkIfNearCapacityAndDoubleCapacity(pReg, pNextRect);
                pNextRect->setLocation(IntPoint(x1, y1));
                pNextRect->setRight(r1->right());
                pNextRect->setBottom(y2);
                ++pReg->m_numRects;
                ++pNextRect;
                ASSERT(pReg->m_numRects <= pReg->m_rects.size());
            }
            ++r1;
            if (r1 != r1End)
                x1 = r1->x();
        }
    }

    // Add remaining minuend rectangles to region.
    while (r1 != r1End) {
        ASSERT(x1 < r1->right());
        checkIfNearCapacityAndDoubleCapacity(pReg, pNextRect);
        pNextRect->setLocation(IntPoint(x1, y1));
        pNextRect->setRight(r1->right());
        pNextRect->setBottom(y2);
        ++pReg->m_numRects;
        ++pNextRect;

        ASSERT(pReg->m_numRects <= pReg->m_rects.size());

        ++r1;
        if (r1 != r1End)
            x1 = r1->x();
    }
}

void IntRectRegion::miSubtractNonO1(IntRectRegion* pReg, const IntRect* r, const IntRect* rEnd, int y1, int y2)
{
    IntRect* pNextRect;

    pNextRect = &pReg->m_rects[pReg->m_numRects];

    ASSERT(y1 < y2);

    while (r != rEnd) {
        ASSERT(r->x() < r->right());
        checkIfNearCapacityAndDoubleCapacity(pReg, pNextRect);
        pNextRect->setLocation(IntPoint(r->x(), y1));
        pNextRect->setRight(r->right());
        pNextRect->setBottom(y2);
        ++pReg->m_numRects;
        ++pNextRect;

        ASSERT(pReg->m_numRects <= pReg->m_rects.size());

        ++r;
    }
}

void IntRectRegion::miIntersectO(IntRectRegion* pReg, const IntRect* r1, const IntRect* r1End, const IntRect* r2, const IntRect* r2End, int y1, int y2)
{
    int x1;
    int x2;
    IntRect* pNextRect;

    pNextRect = &pReg->m_rects[pReg->m_numRects];

    while (r1 != r1End && r2 != r2End) {
        x1 = std::max(r1->x(), r2->x());
        x2 = std::min(r1->right(), r2->right());

        // If there's any overlap between the two rectangles, add that
        // overlap to the new region.
        // There's no need to check for subsumption because the only way
        // such a need could arise is if some region has two rectangles
        // right next to each other. Since that should never happen...
        if (x1 < x2) {
            ASSERT(y1 < y2);

            checkIfNearCapacityAndDoubleCapacity(pReg, pNextRect);
            pNextRect->setLocation(IntPoint(x1, y1));
            pNextRect->setRight(x2);
            pNextRect->setBottom(y2);
            ++pReg->m_numRects;
            ++pNextRect;
            ASSERT(pReg->m_numRects <= pReg->m_rects.size());
        }

        // Need to advance the pointers. Shift the one that extends
        // to the right the least, since the other still has a chance to
        // overlap with that region's next rectangle, if you see what I mean.
        if (r1->right() < r2->right())
            ++r1;
        else if (r2->right() < r1->right())
            ++r2;
        else {
            ++r1;
            ++r2;
        }
    }
}

void IntRectRegion::miUnionNonO(IntRectRegion* pReg, const IntRect* r, const IntRect* rEnd, int y1, int y2)
{
    IntRect* pNextRect;

    pNextRect = &pReg->m_rects[pReg->m_numRects];

    ASSERT(y1 < y2);

    while (r != rEnd) {
        ASSERT(r->x() < r->right());
        checkIfNearCapacityAndDoubleCapacity(pReg, pNextRect);
        pNextRect->setLocation(IntPoint(r->x(), y1));
        pNextRect->setRight(r->right());
        pNextRect->setBottom(y2);
        ++pReg->m_numRects;
        ++pNextRect;

        ASSERT(pReg->m_numRects <= pReg->m_rects.size());
        ++r;
    }
}

void IntRectRegion::mergeRect(IntRectRegion* pReg, IntRect*& pNextRect, const IntRect*& r, int y1, int y2)
{
    if (!pReg->isEmpty() && pNextRect[-1].y() == y1 && pNextRect[-1].bottom() == y2 && pNextRect[-1].right() >= r->x()) {
        if (pNextRect[-1].right() < r->right()) {
            pNextRect[-1].setRight(r->right());
            ASSERT(pNextRect[-1].x() < pNextRect[-1].right());
        }
    } else {
        checkIfNearCapacityAndDoubleCapacity(pReg, pNextRect);
        pNextRect->setLocation(IntPoint(r->x(), y1));
        pNextRect->setRight(r->right());
        pNextRect->setBottom(y2);
        ++pReg->m_numRects;
        ++pNextRect;
    }
    ASSERT(pReg->m_numRects <= pReg->m_rects.size());
    ++r;
}

void IntRectRegion::miUnionO(IntRectRegion* pReg, const IntRect* r1, const IntRect* r1End, const IntRect* r2, const IntRect* r2End, int y1, int y2)
{
    IntRect* pNextRect;

    pNextRect = &pReg->m_rects[pReg->m_numRects];

    ASSERT(y1 < y2);
    while (r1 != r1End && r2 != r2End) {
        if (r1->x() < r2->x())
            mergeRect(pReg, pNextRect, r1, y1, y2);
        else
            mergeRect(pReg, pNextRect, r2, y1, y2);
    }

    if (r1 != r1End) {
        do {
            mergeRect(pReg, pNextRect, r1, y1, y2);
        } while (r1 != r1End);
    } else {
        while (r2 != r2End)
            mergeRect(pReg, pNextRect, r2, y1, y2);
    }
}

#if PLATFORM(OLYMPIA)
String IntRectRegion::toString() const
{
    String result = String::format("[IntRectRegion extents: %s, rects (%i): {", m_extents.toString().utf8().data(), m_numRects);
    if (m_numRects) {
        result += m_rects[0].toString();
        for (unsigned i = 1; i < m_numRects; ++i)
            result += String::format(", %s", m_rects[i].toString().utf8().data());
    }
    result += "}]";
    return result;
}
#endif

#ifndef NDEBUG
static void shouldBeEqualRects(const IntRect& actual, const IntRect& expected)
{
    if (actual == expected)
        printf("## PASSED. [x: %i, y: %i, width: %i, height: %i] is [x: %i, y: %i, width: %i, height: %i].\n",
               actual.x(), actual.y(), actual.width(), actual.height(), expected.x(), expected.y(), expected.width(), expected.height());
    else
        printf("## FAILED. Should be [x: %i, y: %i, width: %i, height: %i]. Was [x: %i, y: %i, width: %i, height: %i].\n",
               expected.x(), expected.y(), expected.width(), expected.height(), actual.x(), actual.y(), actual.width(), actual.height());
}

static void testSubtractEmptyRegionFromNonEmptyRegion()
{
    IntRectRegion emptyRegion;
    IntRectRegion a(IntRect(200, 100, 100, 100));
    IntRectRegion result = subtractRegions(a, emptyRegion);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 1;

    printf("## When subtracting an empty region from a non-empty region:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    shouldBeEqualRects(rects[0], IntRect(200, 100, 100, 100));
}

static void testSubtractNonEmptyRegionFromEmptyRegion()
{
    IntRectRegion emptyRegion;
    IntRectRegion a(IntRect(200, 100, 100, 100));
    IntRectRegion result = subtractRegions(emptyRegion, a);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 0;

    printf("## When subtracting a non-empty region from an empty region:\n");
    if (rects.size() != expectedNumberOfRects)
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
    else
        printf("## PASSED. Is empty.\n");
}

static void testSubtractOfOverlappingRegions()
{
    IntRectRegion a(IntRect(0, 0, 500, 500));
    IntRectRegion b(IntRect(200, 100, 100, 100));
    IntRectRegion result = subtractRegions(a, b);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 4;

    printf("## When subtracting two regions that overlap:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    shouldBeEqualRects(rects[0], IntRect(0, 0, 500, 100));
    shouldBeEqualRects(rects[1], IntRect(0, 100, 200, 100));
    shouldBeEqualRects(rects[2], IntRect(300, 100, 200, 100));
    shouldBeEqualRects(rects[3], IntRect(0, 200, 500, 300));
}

static void testSubtractOfRegionsWithSameLocationAndSize()
{
    IntRectRegion a(IntRect(0, 0, 500, 500));
    IntRectRegion b(IntRect(0, 0, 500, 500));
    IntRectRegion result = subtractRegions(a, b);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 0;

    printf("## When subtracting two regions that have the same location and size:\n");
    if (rects.size() != 0)
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
    else
        printf("## PASSED. Is empty.\n");
}

static void testSubtractOfRegionsWithNoOverlap()
{
    IntRectRegion a(IntRect(0, 0, 500, 500));
    IntRectRegion b(IntRect(500, 0, 500, 500));
    IntRectRegion result = subtractRegions(a, b);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 1;

    printf("## When subtracting two regions that do not overlap:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangle. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    shouldBeEqualRects(rects[0], IntRect(0, 0, 500, 500));
}

static void testTwiceSubtractOfRegions()
{
    IntRectRegion a(IntRect(0, 0, 500, 500));
    IntRectRegion b(IntRect(200, 100, 100, 100));
    IntRectRegion c(IntRect(0, 400, 300, 100));
    IntRectRegion result = subtractRegions(subtractRegions(a, b), c);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 5;

    printf("## When subtracting a regions C from the region that is the subtraction of two regions A, B:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    shouldBeEqualRects(rects[0], IntRect(0, 0, 500, 100));
    shouldBeEqualRects(rects[1], IntRect(0, 100, 200, 100));
    shouldBeEqualRects(rects[2], IntRect(300, 100, 200, 100));
    shouldBeEqualRects(rects[3], IntRect(0, 200, 500, 200));
    shouldBeEqualRects(rects[4], IntRect(300, 400, 200, 100));
}

static void testSubtractRegions()
{
    testSubtractOfOverlappingRegions();
    testSubtractOfRegionsWithSameLocationAndSize();
    testSubtractOfRegionsWithNoOverlap();
    testTwiceSubtractOfRegions();
    testSubtractEmptyRegionFromNonEmptyRegion();
    testSubtractNonEmptyRegionFromEmptyRegion();
}

static void testIntersectOfOverlappingRegions()
{
    IntRectRegion a(IntRect(0, 0, 500, 500));
    IntRectRegion b(IntRect(200, 100, 100, 100));
    IntRectRegion result = intersectRegions(a, b);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 1;

    printf("## When intersecting two regions that overlap:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangle. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    shouldBeEqualRects(rects[0], IntRect(200, 100, 100, 100));
}

static void testIntersectOfRegionsWithSameLocationAndSize()
{
    IntRectRegion a(IntRect(0, 0, 500, 500));
    IntRectRegion b(IntRect(0, 0, 500, 500));
    IntRectRegion result = intersectRegions(a, b);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 1;

    printf("## When intersecting two regions that have the same location and size:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangle. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    shouldBeEqualRects(rects[0], IntRect(0, 0, 500, 500));
}

static void testTwiceIntersectOfRegionsWithOneRegionThatDoesNotOverlap()
{
    IntRectRegion a(IntRect(0, 0, 500, 500));
    IntRectRegion b(IntRect(200, 100, 100, 100));
    IntRectRegion c(IntRect(0, 400, 300, 100));
    IntRectRegion result = intersectRegions(intersectRegions(a, b), c);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 0;

    printf("## When intersecting a regions C that does not overlap with the region that is the intersection of two regions A, B:\n");
    if (rects.size() != expectedNumberOfRects)
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
    else
        printf("## PASSED. Is empty.\n");
}

static void testTwiceIntersectOfOverlappingRegions()
{
    IntRectRegion a(IntRect(0, 0, 500, 500));
    IntRectRegion b(IntRect(200, 100, 100, 100));
    IntRectRegion c(IntRect(290, 100, 20, 20));
    IntRectRegion result = intersectRegions(intersectRegions(a, b), c);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 1;

    printf("## When intersecting a regions C that overlaps with the region that is the intersection of two regions A, B:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    shouldBeEqualRects(rects[0], IntRect(290, 100, 10, 20));
}

static void testIntersectEmptyRegion()
{
    IntRectRegion emptyRegion;
    IntRectRegion a(IntRect(200, 100, 100, 100));
    IntRectRegion result = intersectRegions(a, emptyRegion);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 0;

    printf("## When intersecting an empty region with a non-empty region:\n");
    if (rects.size() != expectedNumberOfRects)
        printf("## FAILED. Should be %u rectangle. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
    else
        printf("## PASSED. Is empty.\n");
}

static void testIntersectRegions()
{
    testIntersectOfOverlappingRegions();
    testIntersectOfRegionsWithSameLocationAndSize();
    testTwiceIntersectOfRegionsWithOneRegionThatDoesNotOverlap();
    testTwiceIntersectOfOverlappingRegions();
    testIntersectEmptyRegion();
}

static void testUnionOfRegions()
{
    IntRectRegion a(IntRect(0, 0, 500, 500));
    IntRectRegion b(IntRect(600, 100, 100, 100));
    IntRectRegion result = unionRegions(a, b);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 4;

    printf("## When unioning two regions:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    shouldBeEqualRects(rects[0], IntRect(0, 0, 500, 100));
    shouldBeEqualRects(rects[1], IntRect(0, 100, 500, 100));
    shouldBeEqualRects(rects[2], IntRect(600, 100, 100, 100));
    shouldBeEqualRects(rects[3], IntRect(0, 200, 500, 300));
}

static void testUnionOfRegionsWithSameLocationAndSize()
{
    IntRectRegion a(IntRect(0, 0, 500, 500));
    IntRectRegion b(IntRect(0, 0, 500, 500));
    IntRectRegion result = unionRegions(a, b);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 1;

    printf("## When unioning two regions that have the same location and size:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    shouldBeEqualRects(rects[0], IntRect(0, 0, 500, 500));
}

static void testUnionPartiallyOverlappingRegions()
{
    IntRectRegion a(IntRect(0, 0, 120, 480));
    IntRectRegion b(IntRect(0, 0, 180, 240));
    IntRectRegion result = unionRegions(a, b);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 2;

    printf("## When unioning regions that partially overlap:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    shouldBeEqualRects(rects[0], IntRect(0, 0, 180, 240));
    shouldBeEqualRects(rects[1], IntRect(0, 240, 120, 240));
}

static void testUnionCommutativity()
{
    IntRectRegion a(IntRect(0, 0, 120, 480));
    IntRectRegion b(IntRect(0, 0, 180, 240));
    IntRectRegion result = unionRegions(a, b);
    Vector<IntRect> resultRects = result.rects();

    printf("## Test union commutativity:\n");    
    IntRectRegion result2 = unionRegions(b, a);
    Vector<IntRect> result2Rects = result2.rects();
    if (resultRects.size() != result2Rects.size()) {
        printf("## FAILED. |unionRegions(a, b)| != |unionRegions(b, a)|. |unionRegions(a, b)| = %u, but |unionRegions(b, a)| = %u.\n", (unsigned)resultRects.size(), (unsigned)result2Rects.size());
        return;
    }
    for (unsigned i = 0; i < resultRects.size(); ++i)
        shouldBeEqualRects(resultRects[i], result2Rects[i]);
}

static void testUnionOfEmptyRegion()
{
    IntRectRegion emptyRegion;
    IntRectRegion a(IntRect(0, 0, 180, 240));
    IntRectRegion result = unionRegions(a, emptyRegion);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 1;

    printf("## When unioning an empty region with a non-empty region:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    shouldBeEqualRects(rects[0], IntRect(0, 0, 180, 240));
}

static void testUnionOfEmptyRegion2()
{
    IntRectRegion emptyRegion;
    IntRectRegion a(IntRect(0, 218, 360, 261));
    IntRectRegion b(IntRect(0, 0, 180, 240));
    IntRectRegion c(IntRect(180, 0 , 180, 240));
    IntRectRegion d(IntRect(0, 240, 180, 240));
    IntRectRegion e(IntRect(180, 240, 180, 240));
    IntRectRegion result = unionRegions(a, emptyRegion);
    result = unionRegions(b, result);
    result = unionRegions(c, result);
    result = unionRegions(d, result);
    result = unionRegions(e, result);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 1;

    printf("## When unioning an empty region with a non-empty region (Test 2):\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    shouldBeEqualRects(rects[0], IntRect(0, 0, 360, 480));
}

static void testUnionRegions()
{
    testUnionOfRegions();
    testUnionOfRegionsWithSameLocationAndSize();
    testUnionPartiallyOverlappingRegions();
    testUnionCommutativity();
    testUnionOfEmptyRegion();
    testUnionOfEmptyRegion2();
}

static void testPartiallyContainsRectInRegionWithTwoRectsThatAreSideBySide()
{
    IntRectRegion a(IntRect(0, 0, 500, 500));
    IntRectRegion b(IntRect(500, 100, 100, 100));
    IntRectRegion result = unionRegions(a, b);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 3;

    printf("## Tests that region partially contains a rectangle:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    if (result.isRectInRegion(IntRect(400, 0, 200, 100)) == IntRectRegion::PartiallyContainedInRegion)
        printf("## PASSED.\n");
    else
        printf("## FAILED.\n");
}

static void testPartiallyContainsRectInRegionWithTwoRectsThatAreSideBySide2()
{
    IntRectRegion a(IntRect(0, 0, 180, 240));
    IntRectRegion b(IntRect(180, 0, 180, 240));
    IntRectRegion c(IntRect(0, 240, 180, 240));
    IntRectRegion result = unionRegions(a, b);
    result = unionRegions(result, c);

    printf("## Tests that region partially contains a rectangle (Test 2):\n");
    if (result.isRectInRegion(IntRect(0, 0, 360, 480)) == IntRectRegion::PartiallyContainedInRegion)
        printf("## PASSED.\n");
    else
        printf("## FAILED.\n");
}

static void testDoesNotContainRectInRegionWithTwoRectsThatAreSideBySide()
{
    IntRectRegion a(IntRect(0, 0, 500, 500));
    IntRectRegion b(IntRect(500, 100, 100, 100));
    IntRectRegion result = unionRegions(a, b);
    Vector<IntRect> rects = result.rects();
    const unsigned expectedNumberOfRects = 3;

    printf("## Tests that region does not contain rectangle:\n");
    if (rects.size() != expectedNumberOfRects) {
        printf("## FAILED. Should be %u rectangles. Was %u.\n", expectedNumberOfRects, (unsigned)rects.size());
        return;
    }
    if (result.isRectInRegion(IntRect(700, 0, 200, 100)) == IntRectRegion::NotInRegion)
        printf("## PASSED.\n");
    else
        printf("## FAILED.\n");
}

static void testContainsRectInRegionWithTwoRectsThatAreSideBySide()
{
    IntRectRegion a(IntRect(0, 0, 180, 240));
    IntRectRegion b(IntRect(180, 0, 180, 240));
    IntRectRegion c(IntRect(0, 240, 180, 240));
    IntRectRegion d(IntRect(180, 240, 180, 240));
    IntRectRegion result = unionRegions(a, b);
    result = unionRegions(result, c);
    result = unionRegions(result, d);

    printf("## Tests that region contains a rectangle:\n");
    if (result.isRectInRegion(IntRect(0, 0, 360, 480)) == IntRectRegion::ContainedInRegion)
        printf("## PASSED.\n");
    else
        printf("## FAILED.\n");
}

static void testContainsRectInRegionWithTwoRectsThatAreSideBySide2()
{
    IntRectRegion a(IntRect(0, 0, 120, 480));
    IntRectRegion b(IntRect(0, 0, 180, 240));
    IntRectRegion c(IntRect(180, 0, 180, 240));
    IntRectRegion d(IntRect(0, 240, 180, 240));
    IntRectRegion e(IntRect(180, 240, 180, 240));

    IntRectRegion result = unionRegions(b, a);
    result = unionRegions(c, result);
    result = unionRegions(d, result);
    result = unionRegions(e, result);

    printf("## Tests that region contains a rectangle (Test 2):\n");
    if (result.isRectInRegion(IntRect(0, 0, 360, 480)) == IntRectRegion::ContainedInRegion)
        printf("## PASSED.\n");
    else
        printf("## FAILED.\n");
}

static void testContains()
{
    testPartiallyContainsRectInRegionWithTwoRectsThatAreSideBySide();
    testPartiallyContainsRectInRegionWithTwoRectsThatAreSideBySide2();
    testDoesNotContainRectInRegionWithTwoRectsThatAreSideBySide();
    testContainsRectInRegionWithTwoRectsThatAreSideBySide();
    testContainsRectInRegionWithTwoRectsThatAreSideBySide2();
}

static void testEqualRegions()
{
    IntRectRegion a(IntRect(0, 0, 180, 240));
    IntRectRegion b(IntRect(0, 0, 180, 240));

    printf("## Tests that IntRectRegion(IntRect(0, 0, 180, 240)) == IntRectRegion(IntRect(0, 0, 180, 240)):\n");
    if (a.isEqual(b))
        printf("## PASSED.\n");
    else
        printf("## FAILED.\n");
}

static void testEqualRegionsForEmptyRegions()
{
    IntRectRegion a;
    IntRectRegion b(IntRect(0, 0, 0, 0));

    printf("## Tests that IntRectRegion() == IntRectRegion(IntRect(0, 0, 0, 0)):\n");
    if (a.isEqual(b))
        printf("## PASSED.\n");
    else
        printf("## FAILED.\n");
}

static void testNotEqualRegions()
{
    IntRectRegion a(IntRect(0, 0, 180, 240));
    IntRectRegion b(IntRect(0, 0, 170, 240));

    printf("## Tests that IntRectRegion(IntRect(0, 0, 180, 240)) != IntRectRegion(IntRect(0, 0, 170, 240)):\n");
    if (!a.isEqual(b))
        printf("## PASSED.\n");
    else
        printf("## FAILED.\n");
}

static void testNotEqualRegions2()
{
    IntRectRegion a;
    IntRectRegion b(IntRect(0, 0, 170, 240));

    printf("## Tests that IntRectRegion() != IntRectRegion(IntRect(0, 0, 170, 240)):\n");
    if (!a.isEqual(b))
        printf("## PASSED.\n");
    else
        printf("## FAILED.\n");
}

static void testEqualsCommutativity()
{
    IntRectRegion a(IntRect(0, 0, 180, 240));
    IntRectRegion b(IntRect(0, 0, 180, 240));

    printf("## Test equals commutativity:\n");
    if (a.isEqual(b) && b.isEqual(a))
        printf("## PASSED.\n");
    else
        printf("## FAILED.\n");
}

static void testEqualsCommutativity2()
{
    IntRectRegion a(IntRect(0, 0, 180, 240));
    IntRectRegion b(IntRect(0, 0, 170, 240));

    printf("## Test equals commutativity (Test 2):\n");
    if (!a.isEqual(b) && !b.isEqual(a))
        printf("## PASSED.\n");
    else
        printf("## FAILED.\n");
}

static void testEqualsCommutativity3()
{
    IntRectRegion a(IntRect(0, 0, 100, 100));
    IntRectRegion b(IntRect(100, 0, 100, 100));
    IntRectRegion result = unionRegions(a, b);

    IntRectRegion c(IntRect(0, 0, 500, 500));
    IntRectRegion d(IntRect(0, 0, 200, 100));
    IntRectRegion result2 = intersectRegions(c, d);

    printf("## Test equals commutativity (Test 3):\n");
    if (result.isEqual(result2) && result2.isEqual(result))
        printf("## PASSED.\n");
    else
        printf("## FAILED.\n");
}

static void testEquals()
{
    testEqualRegions();
    testEqualRegionsForEmptyRegions();
    testNotEqualRegions();
    testNotEqualRegions2();
    testEqualsCommutativity();
    testEqualsCommutativity2();
    testEqualsCommutativity3();
}

void testIntRectRegion()
{
    testSubtractRegions();
    testIntersectRegions();
    testUnionRegions();
    testContains();
    testEquals();
}
#endif

} // namespace WebCore
