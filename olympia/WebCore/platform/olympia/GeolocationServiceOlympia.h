/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef GeolocationServiceOlympia_h
#define GeolocationServiceOlympia_h

#include "GeolocationService.h"
#include "Geoposition.h"
#include "PositionError.h"
#include <wtf/RefPtr.h>
#include "OlympiaPlatformGeoTracker.h"
#include "OlympiaPlatformGeoTrackerListener.h"

namespace WebCore {
    class Geolocation;

    class GeolocationServiceOlympia : public GeolocationService, Olympia::Platform::GeoTrackerListener {
    public:
        static GeolocationService* create(GeolocationServiceClient*);
        ~GeolocationServiceOlympia();

        virtual bool startUpdating(PositionOptions*);
        virtual void stopUpdating();

        virtual void suspend();
        virtual void resume();

        Geoposition* lastPosition() const;
        PositionError* lastError() const;
        Olympia::Platform::GeoTracker* tracker() const { return m_tracker; }

        void onLocationUpdate(double timestamp, double latitude, double longitude, double altitude, double accuracy, double altitudeAccuracy);
        void onLocationError(const char* error);
        void onPermission(bool isAllowed);

        bool hasDeferredNotifications() const { return m_hasDeferredPosition || m_hasDeferredPermission; }
        void deferNotifications();
        void resumeNotifications();

    private:
        GeolocationServiceOlympia(GeolocationServiceClient*);
        void setError(PositionError::ErrorCode, const char* message);

    private:
        RefPtr<Geoposition> m_lastPosition;
        RefPtr<PositionError> m_lastError;
        Olympia::Platform::GeoTracker* m_tracker;
        Geolocation* m_geolocation;
        bool m_updating;
        bool m_defersNotifications;
        bool m_hasDeferredPosition;
        bool m_hasDeferredPermission;
        bool m_deferredPermissionIsAllowed;
    };
}

#endif
