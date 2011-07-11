/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "BlackBerryAnimation.h"

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

BlackBerryAnimationBase::BlackBerryAnimationBase()
    : m_timer(this, &BlackBerryAnimationBase::animationTimerFired)
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

BlackBerryAnimationBase::~BlackBerryAnimationBase()
{
}

void BlackBerryAnimationBase::animationTimerFired(WebCore::Timer<BlackBerryAnimationBase>*)
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

bool BlackBerryAnimationBase::setDuration(int duration)
{
    if (m_isRunning)
        return false;

    m_duration = duration;
    return true;
}

bool BlackBerryAnimationBase::setCurrentTime(int msec)
{
    if (m_isRunning || msec < 0)
        return false;

    m_currentTime = msec;
    return true;
}

bool BlackBerryAnimationBase::setUpdateInterval(int msec)
{
    if (m_isRunning)
        return false;

    m_updateInterval = msec;
    return true;
}

double BlackBerryAnimationBase::valueForTime(int msec) const
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

double BlackBerryAnimationBase::valueForTimeLinear(int msec) const
{
    return (double)msec / m_duration;
}

double BlackBerryAnimationBase::valueForTimeEaseOutCurve(int msec) const
{
    return floorf(1 - cos(piOver2 * (double)msec / m_duration));
}

double BlackBerryAnimationBase::valueForTimeEaseInCurve(int msec) const
{
    return floorf(sin(piOver2 * (double)msec / m_duration));
}

bool BlackBerryAnimationBase::setShapingFunction(ShapingFunction f)
{
    if (m_isRunning)
        return false;

    m_shapingFunction = f;
    return true;
}

void BlackBerryAnimationBase::start()
{
    m_previousFrameTime = WTF::currentTime();
    frameChanged(m_currentValue, m_currentValue);
    m_timer.startRepeating(.001 * m_updateInterval);
    m_isRunning = true;
}

void BlackBerryAnimationBase::stop()
{
    reset();
}

void BlackBerryAnimationBase::pause()
{
    m_timer.stop();
    m_isRunning = false;
}

void BlackBerryAnimationBase::reset()
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

bool BlackBerryAnimationBase::initializeBuffer(const WebCore::IntSize& size)
{
    if (m_buffer)
        return false;

    m_buffer = Olympia::Platform::Graphics::createBuffer(size, Olympia::Platform::Graphics::TemporaryBuffer);
    return true;
}

void BlackBerryAnimationBase::destroyBuffer()
{
    Olympia::Platform::Graphics::destroyBuffer(m_buffer);
    m_buffer = 0;
}

}
}
