/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef BlackBerryPlatformGeoTrackerImpl_h
#define BlackBerryPlatformGeoTrackerImpl_h

#include "BlackBerryPlatformGeoTracker.h"

namespace BlackBerry {
namespace Platform {

    class GeoTrackerImpl: public GeoTracker {
    public:
        GeoTrackerImpl(GeoTrackerListener* listener, int m_playerId);
        bool init(bool highAccuracy, int timeout, int maxAge);

        virtual void destroy();
        virtual void suspend();
        virtual void resume();

        int handle() const { return m_localHandle; }
        
        void onRemoteHandleCreated(int remoteHandle);
        void onRemoteHandleDestroyed();
        void onLocationUpdate(double timestamp, double latitude, double longitude, double altitude, double accuracy, double altitudeAccuracy);
        void onLocationError(const char* error);
        void onPermission(bool isAllowed);
        void setIsRequestingPermission(bool);
    private:
        ~GeoTrackerImpl();
        GeoTrackerListener* m_listener;
        int m_playerId;
        int m_localHandle;
        int m_remoteHandle;
        int m_locationUpdates;
        bool m_isActive;
        bool m_isRequestingPermission;           
    };

} // namespace Olympia
} // namespace Platform

#endif // BlackBerryPlatformGeoTrackerImpl_h
