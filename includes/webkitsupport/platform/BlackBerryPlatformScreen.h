/*
 * Copyright (C) Research In Motion, Limited 2011. All rights reserved.
 */

#ifndef BlackBerryPlatformScreen_h
#define BlackBerryPlatformScreen_h

#include "BlackBerryPlatformPrimitives.h"

namespace BlackBerry {
namespace Platform {
namespace Graphics {

class Screen
{
public:
    static int width();
    static int height();
    static IntSize size();
    static void setScreenSize(const IntSize&);

    static int landscapeWidth();
    static int landscapeHeight();
    static IntSize landscapeSize();

    // These return landscape or portrait depending on device
    static int defaultWidth();
    static int defaultHeight();
    static IntSize defaultSize();
};

} // namespace Graphics
} // namespace Platform
} // namespace BlackBerry

#endif /* BlackBerryPlatformScreen_h */
