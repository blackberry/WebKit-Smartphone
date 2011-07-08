/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#include "config.h"
#include "GeolocationServiceOlympia.h"

#include "CString.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "FrameLoaderClient.h"
#include "FrameLoaderClientOlympia.h"
#include "Geolocation.h"
#include "NotImplemented.h"
#include "PositionOptions.h"

namespace WebCore {

GeolocationService* GeolocationServiceOlympia::create(GeolocationServiceClient* client)
{
    return new GeolocationServiceOlympia(client);
}

GeolocationService::FactoryFunction* GeolocationService::s_factoryFunction = &GeolocationServiceOlympia::create;

GeolocationServiceOlympia::GeolocationServiceOlympia(GeolocationServiceClient* client)
    : GeolocationService(client)
    , m_lastError(0)
    , m_tracker(0)
    , m_geolocation(static_cast<Geolocation*>(client))
    , m_updating(false)
    , m_defersNotifications(false)
    , m_hasDeferredPosition(false)
    , m_hasDeferredPermission(false)
    , m_deferredPermissionIsAllowed(false)
{
    m_geolocation->frame()->loader()->client()->didCreateGeolocation(m_geolocation);
}

GeolocationServiceOlympia::~GeolocationServiceOlympia()
{
    stopUpdating();
}

bool GeolocationServiceOlympia::startUpdating(PositionOptions* options)
{
    Frame *frame = m_geolocation->frame();
    if(!frame)
        return false;
    int playerId = static_cast<FrameLoaderClientOlympia*>(frame->loader()->client())->playerId();
    m_tracker = Olympia::Platform::GeoTracker::create(this, playerId, options->enableHighAccuracy(), options->hasTimeout() ? options->timeout() : -1, options->hasMaximumAge() ? options->maximumAge() : -1);
    m_updating = true;
    return m_tracker;
}

void GeolocationServiceOlympia::stopUpdating()
{
    if (m_tracker)
        m_tracker->destroy();
    m_tracker = 0;
    m_updating = false;
}

void GeolocationServiceOlympia::suspend()
{
    m_updating = false;
    if (m_tracker)
        m_tracker->suspend();
}

void GeolocationServiceOlympia::resume()
{
    m_updating = true;
    if (m_tracker)
        m_tracker->resume();
}

Geoposition* GeolocationServiceOlympia::lastPosition() const
{
    return m_lastPosition.get();
}

PositionError* GeolocationServiceOlympia::lastError() const
{
    return m_lastError.get();
}

void GeolocationServiceOlympia::onLocationUpdate(double timestamp, double latitude, double longitude, double altitude, double accuracy, double altitudeAccuracy)
{
    if (!m_updating)
        return;
    m_lastError = 0;
    RefPtr<Coordinates> coordinates = Coordinates::create(latitude,
                                                          longitude,
                                                          true, // providesAltititude
                                                          altitude,
                                                          accuracy,
                                                          true, // providesAltitudeAccuracy
                                                          altitudeAccuracy,
                                                          false, // providesHeading
                                                          0.0,   // heading
                                                          false, // providesSpeed
                                                          0.0);  // speed
    m_lastPosition = Geoposition::create(coordinates.release(), timestamp);

    if (m_defersNotifications) {
        m_hasDeferredPosition = true;
        return;
    }

    positionChanged();
}

void GeolocationServiceOlympia::onLocationError(const char* error)
{
    if (!m_updating)
        return;
    m_lastPosition = 0;
    m_lastError = PositionError::create(PositionError::POSITION_UNAVAILABLE, String::fromUTF8(error));
}

void GeolocationServiceOlympia::onPermission(bool isAllowed)
{
    if (m_defersNotifications) {
        m_hasDeferredPermission = true;
        m_deferredPermissionIsAllowed = isAllowed;
        return;
    }

    m_geolocation->setIsAllowed(isAllowed);
}

void GeolocationServiceOlympia::deferNotifications()
{
    m_defersNotifications = true;
}

void GeolocationServiceOlympia::resumeNotifications()
{
    m_defersNotifications = false;

    if (m_hasDeferredPermission) {
        m_hasDeferredPermission = false;
        m_geolocation->setIsAllowed(m_deferredPermissionIsAllowed);
    }
    if (m_hasDeferredPosition) {
        m_hasDeferredPosition = false;
        positionChanged();
    }
}

}
