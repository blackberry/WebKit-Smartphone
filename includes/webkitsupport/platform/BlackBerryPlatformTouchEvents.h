#ifndef OLYMPIAPLATFORMTOUCHEVENTS_H
#define OLYMPIAPLATFORMTOUCHEVENTS_H

#include "BlackBerryPlatformPrimitives.h"

#include <vector>

namespace BlackBerry {
namespace WebKit {
    // NOTE: these enums must match OlympiaConstants.java
    enum TouchEventType { TouchEventPressed, TouchEventReleased, TouchEventMoved, TouchEventNone, TouchEventCanceled, TouchEventApplyReleased, TouchEventStationary };
}
}

namespace BlackBerry {
namespace Platform {

    class TouchPoint {
    public:
        int m_id;
        enum State { TouchReleased, TouchMoved, TouchPressed, TouchStationary };
        State m_state;
        IntPoint m_screenPos;
        IntPoint m_pos;
    };

    class GestureData {
    public:
        virtual GestureData* copy() = 0;
    };

    class PinchGestureData : public GestureData {
    public:
        enum PinchGestureState { PinchStart, PinchMove, PinchEnd };

        PinchGestureData(PinchGestureState state, IntPoint& focalPoint, IntPoint& scrollPosition, double scale)
            : m_state(state)
            , m_focalPoint(focalPoint)
            , m_scrollPosition(scrollPosition)
            , m_scale(scale)
        {
        };

        virtual GestureData* copy() { return new PinchGestureData(m_state, m_focalPoint, m_scrollPosition, m_scale); }

        PinchGestureState m_state;
        IntPoint m_focalPoint;
        IntPoint m_scrollPosition;
        double m_scale;
    };

    class Gesture {
    public:
        enum GestureType { Tap, DoubleTap, DoubleTapTimerFired, TouchHold, Pinch, TwoFingerScroll };

        Gesture(GestureType type, GestureData* data = 0)
            : m_type(type)
            , m_data(data)
        {
        }

        Gesture(const Gesture& gesture)
            : m_type(gesture.m_type)
            , m_data(gesture.m_data ? gesture.m_data->copy() : 0)
        {
        }

        ~Gesture() { delete m_data; }

        Gesture& operator=(const Gesture& rhs)
        {
            if (this == &rhs)
                return *this;

            m_type = rhs.m_type;
            delete m_data;
            m_data = rhs.m_data ? rhs.m_data->copy() : 0;
            return *this;
        }

        GestureType m_type;
        GestureData* m_data;
    };

    class TouchEvent {
    public:
        enum Type { TouchStart, TouchMove, TouchEnd, TouchCancel, TouchCancelAndClearFocusedNode };
        Type m_type;
        bool m_altKey;
        bool m_shiftKey;
        bool m_singleTouch;
        std::vector<TouchPoint> m_points;
        std::vector<Gesture> m_gestures;
    };

    class SingleTouchEvent {
    public:
        enum SingleType { SinglePressed, SingleReleased, SingleMoved };
        SingleType m_singleType;
        IntPoint m_screenPos;
        IntPoint m_pos;
    };


} // namespace Platform
} // namespace BlackBerry

#endif // OLYMPIAPLATFORMTOUCHEVENTS_H
