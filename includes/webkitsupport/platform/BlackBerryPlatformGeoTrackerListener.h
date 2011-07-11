/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef BlackBerryPlatformGeoTrackerListener_h
#define BlackBerryPlatformGeoTrackerListener_h

namespace BlackBerry {
namespace Platform {
    class GeoTrackerListener {

    public:
        virtual void onLocationUpdate(double timestamp, double latitude, double longitude, double altitude, double accuracy, double altitudeAccuracy) = 0;
        virtual void onLocationError(const char* error) = 0;
        virtual void onPermission(bool isAllowed) = 0;
    };
} // namespace Olympia
} // namespace Platform

#endif // BlackBerryPlatformGeoTrackerListener_h
