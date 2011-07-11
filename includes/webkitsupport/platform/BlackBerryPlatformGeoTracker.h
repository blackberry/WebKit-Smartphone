/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef BlackBerryPlatformGeoTracker_h
#define BlackBerryPlatformGeoTracker_h

#include "BlackBerryPlatformGeoTrackerListener.h"

namespace BlackBerry {
namespace Platform {

    class GeoTracker {
    public:
        static GeoTracker* create(GeoTrackerListener* listener, int playerId, bool highAccuracy, int timeout, int maxAge);

        virtual void destroy() = 0;
        virtual void suspend() = 0;
        virtual void resume() = 0;

    protected:
        virtual ~GeoTracker();
    };

} // namespace Olympia
} // namespace Platform

#endif // BlackBerryPlatformGeoTracker_h
