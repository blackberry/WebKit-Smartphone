/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef OlympiaAnimation_h
#define OlympiaAnimation_h

#include "OlympiaGlobal.h"
#include "Timer.h"

#define ENABLE_FRAME_COUNTER_DEBUG 0

namespace WebCore {
    class SurfaceOpenVG;
    class IntSize;
}

namespace Olympia {
namespace WebKit {

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

class OlympiaAnimationBase {
    public:
        enum ShapingFunction { Linear, EaseInCurve, EaseOutCurve };

        OlympiaAnimationBase();
        ~OlympiaAnimationBase();

        // set the maximum duration of the animation in msec
        bool setDuration(int duration);

        // returns the maximum duration of the animation in msec
        int duration() const { return m_duration; }

        // set the current time in the timeline, between 0 and duration
        bool setCurrentTime(int msec);

        // returns the current time in the timeline
        int currentTime() const { return m_currentTime; }

        // returns the position value in the timeline at time msec from 0.0 to 1.0
        double valueForTime(int msec) const;

        // returns the current position value in the timeline from 0.0 to 1.0
        double currentValue() const { return m_currentValue; }

        // returns true is the animation is running; otherwise returns false
        bool isRunning() const { return m_isRunning; }

        // sets the rate at which the animation timer fires
        bool setUpdateInterval(int msec);

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
        void animationTimerFired(WebCore::Timer<OlympiaAnimationBase>*);

        bool initializeBuffer(const WebCore::IntSize& size);
        void destroyBuffer();
        unsigned short* buffer() { return m_buffer; }
        unsigned int bufferStride() const { return m_bufferStride; }

    private:
        WebCore::Timer<OlympiaAnimationBase> m_timer;

        void reset();
        double valueForTimeLinear(int msec) const;
        double valueForTimeEaseInCurve(int msec) const;
        double valueForTimeEaseOutCurve(int msec) const;

        int m_duration;
        int m_currentTime;
        int m_updateInterval;

        double m_currentValue;
        double m_previousFrameTime;

        bool m_isRunning;
        bool m_canRenderNextFrame;

#if ENABLE_FRAME_COUNTER_DEBUG
        int m_frameCount;
#endif

        ShapingFunction m_shapingFunction;

        unsigned short* m_buffer;
        unsigned int m_bufferStride;
};

template <typename OlympiaAnimationControlClass> class OlympiaAnimation : public OlympiaAnimationBase {
    public:
        typedef void (OlympiaAnimationControlClass::*OlympiaAnimationTimeoutFunction)(OlympiaAnimation*, double currentValue, double previousValue);

        OlympiaAnimation(OlympiaAnimationControlClass* o, OlympiaAnimationTimeoutFunction f)
            : m_object(o)
            , m_function(f)
            { }

    private:
        virtual void frameChanged(double currentValue, double previousValue) { (m_object->*m_function)(this, currentValue, previousValue); }

        OlympiaAnimationControlClass* m_object;
        OlympiaAnimationTimeoutFunction m_function;
};
}
}

#endif // OlympiaAnimation_h
