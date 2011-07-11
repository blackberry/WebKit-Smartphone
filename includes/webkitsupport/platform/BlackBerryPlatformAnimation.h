/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef BlackBerryPlatformAnimation_h
#define BlackBerryPlatformAnimation_h

#include "BlackBerryPlatformPrimitives.h"
#include "BlackBerryPlatformTimer.h"

#define ENABLE_FRAME_COUNTER_DEBUG 0

// Class Description:
//
// This class can be used to generate an animation timeline for generic animations.
//
// Notes:
//
// - setDuration(), setCurrentTime(), setUpdateInterval(), setShapingFunction() require
//   the that animation is not running in order to succeed. These functions
//   return true if they succeed and false otherwise.
//
// - The renderer is responsible for calling setCanRenderNextFrame(true) before
//   the timeline will call the frameChanged() callback again.
//
//  - All Times are in seconds.
//

namespace BlackBerry {
namespace Platform {

class AnimationBase {
    public:
        enum ShapingFunction { Linear, EaseInCurve, EaseOutCurve };

        AnimationBase(TimerClient*);
        ~AnimationBase();

        // set the maximum duration of the animation in seconds
        bool setDuration(double duration);

        // returns the maximum duration of the animation in seconds
        double duration() const { return m_duration; }

        // set the current time in the timeline, between 0 and duration
        bool setCurrentTime(double time);

        // returns the current time in the timeline in seconds
        double currentTime() const { return m_currentTime; }

        // returns the position value in the timeline at time msec from 0.0 to 1.0
        double valueForTime(double time) const;

        // returns the current position value in the timeline from 0.0 to 1.0
        double currentValue() const { return m_currentValue; }

        // returns true is the animation is running; otherwise returns false
        bool isRunning() const { return m_isRunning; }

        // sets the rate at which the animation timer fires in seconds
        bool setUpdateInterval(double interval);

        // returns the rate at which the animation timer fires
        int updateInterval() const { return m_updateInterval; }

        bool setShapingFunction(ShapingFunction f);
        ShapingFunction shapingFunction() const { return m_shapingFunction; }

        void setCanRenderNextFrame(bool canRenderNextFrame) { m_canRenderNextFrame = canRenderNextFrame; }

        void start();
        void stop();
        void pause();

        // Callback function to be re-implemented by the renderer. This function is called every time the
        // renderer should render a new frame in the animation. CurrentValue and previousValue represent
        // relative positions in the animation timeline from 0.0 to 1.0.
        virtual void frameChanged(double currentValue, double previousValue) = 0;

        // called when the animation timer fires
        void animationTimerFired();

    private:
        void reset();
        double valueForTimeLinear(double time) const;
        double valueForTimeEaseInCurve(double time) const;
        double valueForTimeEaseOutCurve(double time) const;
        double getMonotonicTime();

        BlackBerry::Platform::Timer<AnimationBase> m_timer;
        double m_duration;
        double m_currentTime;
        double m_updateInterval;

        double m_currentValue;
        double m_previousFrameTime;

        bool m_isRunning;
        bool m_isPaused;
        bool m_canRenderNextFrame;

#if ENABLE_FRAME_COUNTER_DEBUG
        int m_frameCount;
#endif
        ShapingFunction m_shapingFunction;
};

template <typename AnimationControlClass> class Animation : public AnimationBase {
    public:
        typedef void (AnimationControlClass::*AnimationTimeoutFunction)(Animation*, double currentValue, double previousValue);

        Animation(AnimationControlClass* o, AnimationTimeoutFunction f, TimerClient* c = 0)
            : AnimationBase(c)
            , m_object(o)
            , m_function(f)
            { }

    private:
        virtual void frameChanged(double currentValue, double previousValue) { (m_object->*m_function)(this, currentValue, previousValue); }

        AnimationControlClass* m_object;
        AnimationTimeoutFunction m_function;
};

} // namespace Platform
} // namespace BlackBerry

#endif // BlackBerryPlatformAnimation_h
