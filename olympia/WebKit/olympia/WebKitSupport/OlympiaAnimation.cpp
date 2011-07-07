/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "OlympiaAnimation.h"

#include "IntSize.h"
#include "OlympiaPlatformMisc.h"
#include "SurfaceOpenVG.h"
#include "EGLDisplayOpenVG.h"

#include <wtf/CurrentTime.h>
#include <wtf/MathExtras.h>

using namespace WebCore;

namespace Olympia {
namespace WebKit {

static const double piOver2 = piDouble / 2;

OlympiaAnimationBase::OlympiaAnimationBase()
    : m_timer(this, &OlympiaAnimationBase::animationTimerFired)
    , m_duration(0)
    , m_currentTime(0)
    , m_updateInterval(33)
    , m_currentValue(0.0)
    , m_previousFrameTime(0.0)
    , m_isRunning(false)
    , m_canRenderNextFrame(true)
#if ENABLE_FRAME_COUNTER_DEBUG
    , m_frameCount(0)
#endif
    , m_shapingFunction(Linear)
    , m_buffer(0)
{
}

OlympiaAnimationBase::~OlympiaAnimationBase()
{
}

void OlympiaAnimationBase::animationTimerFired(WebCore::Timer<OlympiaAnimationBase>*)
{
    double time = WTF::currentTime();
    double previousValue = m_currentValue;
    m_currentTime += (time - m_previousFrameTime) * 1000;
    m_previousFrameTime = time;

    if (m_currentTime > m_duration) {
#if ENABLE_FRAME_COUNTER_DEBUG
        Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "Animation finished. Frames rendered = %d Frames/Sec = %f\n", m_frameCount + 1, (double)(m_frameCount + 1) / m_currentTime * 1000);
#endif
        stop();
        frameChanged(-1.0, previousValue);
        return;
    }

    if (!m_canRenderNextFrame)
        return;

#if ENABLE_FRAME_COUNTER_DEBUG
        m_frameCount++;
#endif
        m_canRenderNextFrame = false;
        m_currentValue = valueForTimeLinear(m_currentTime);
        frameChanged(m_currentValue, previousValue);
}

bool OlympiaAnimationBase::setDuration(int duration)
{
    if (m_isRunning)
        return false;

    m_duration = duration;
    return true;
}

bool OlympiaAnimationBase::setCurrentTime(int msec)
{
    if (m_isRunning || msec < 0)
        return false;

    m_currentTime = msec;
    return true;
}

bool OlympiaAnimationBase::setUpdateInterval(int msec)
{
    if (m_isRunning)
        return false;

    m_updateInterval = msec;
    return true;
}

double OlympiaAnimationBase::valueForTime(int msec) const
{
    if (msec <= 0)
        return -1.0;

    if (!m_duration)
        return 0;

    if (msec > m_duration)
        return -1.0;

    switch (m_shapingFunction) {
    case Linear:
        return valueForTimeLinear(msec);
    case EaseInCurve:
        return valueForTimeEaseInCurve(msec);
    case EaseOutCurve:
        return valueForTimeEaseOutCurve(msec);
    default:
        return -2.0;
    }
}

double OlympiaAnimationBase::valueForTimeLinear(int msec) const
{
    return (double)msec / m_duration;
}

double OlympiaAnimationBase::valueForTimeEaseOutCurve(int msec) const
{
    return floorf(1 - cos(piOver2 * (double)msec / m_duration));
}

double OlympiaAnimationBase::valueForTimeEaseInCurve(int msec) const
{
    return floorf(sin(piOver2 * (double)msec / m_duration));
}

bool OlympiaAnimationBase::setShapingFunction(ShapingFunction f)
{
    if (m_isRunning)
        return false;

    m_shapingFunction = f;
    return true;
}

void OlympiaAnimationBase::start()
{
    m_previousFrameTime = WTF::currentTime();
    frameChanged(m_currentValue, m_currentValue);
    m_timer.startRepeating(.001 * m_updateInterval);
    m_isRunning = true;
}

void OlympiaAnimationBase::stop()
{
    reset();
}

void OlympiaAnimationBase::pause()
{
    m_timer.stop();
    m_isRunning = false;
}

void OlympiaAnimationBase::reset()
{
    m_timer.stop();
    m_currentTime = 0;
    m_currentValue = 0.0;
    m_previousFrameTime = 0.0;
    m_isRunning = false;
    m_canRenderNextFrame = true;

#if ENABLE_FRAME_COUNTER_DEBUG
    m_frameCount = 0;
#endif
}

bool OlympiaAnimationBase::initializeBuffer(const WebCore::IntSize& size)
{
    if (m_buffer)
        return false;

    m_buffer = new SurfaceOpenVG(size, EGLDisplayOpenVG::current()->display());

    return true;
}

void OlympiaAnimationBase::destroyBuffer()
{
    delete m_buffer;
    m_buffer = 0;
}

}
}
