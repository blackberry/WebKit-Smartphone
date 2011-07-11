/*
 * Copyright (C) Research In Motion Limited 2009-2011. All rights reserved.
 */

#ifndef BlackBerryPlatformTimer_h
#define BlackBerryPlatformTimer_h

namespace blackberry {
namespace bridge {
    class Timer;
}
}

namespace BlackBerry {
namespace Platform {

    class TimerClient {
    public:
        virtual bool willFireTimer() = 0;
    };

    typedef void (*TimerFunction)();
    typedef void (*ContextTimerFunction)(void*);

    void timerStart(double interval, TimerFunction function);
    void timerStop();
    void setTimerClient(TimerClient*);

    class TimerBase
    {
    public:
        TimerBase();
        virtual ~TimerBase();

        void start(double interval, ContextTimerFunction function, void* context);
        void start(double interval, TimerFunction function);
        void stop();
        void setPeriodic(bool periodic);

        void setClient(TimerClient* client);
        TimerClient* client() const;

        virtual void fired() = 0;

        bool started() const;

    protected:
        static void timerFired(blackberry::bridge::Timer*, void* context);

        TimerClient* m_timerClient;
        TimerFunction m_timerFunction;
        ContextTimerFunction m_contextTimerFunction;

        void* m_context;
        bool m_timerStarted;
        bool m_periodic;
        double m_interval;
        blackberry::bridge::Timer* m_timer;
    };

    // A templated timer class that will invoke a method when the timer fires
    template<typename T> class Timer : public TimerBase {
    public:
        Timer(T* object, void(T::*method)()) : m_object(object), m_method(method) { }
        void start(double interval);
        virtual void fired();
    private:
        T* m_object;
        void(T::*m_method)();
    };

    template<typename T>
    void Timer<T>::start(double interval)
    {
        TimerBase::start(interval, 0);
    }

    template<typename T>
    void Timer<T>::fired()
    {
        if (!started())
            return;

        if (m_periodic)
            start(m_interval);
        else
            m_timerStarted = false;

        if (m_timerClient && !m_timerClient->willFireTimer())
            TimerBase::start(0.0, m_timerFunction);
        else
            (m_object->*m_method)();
    }

} // namespace Platform
} // namespace BlackBerry

#endif // BlackBerryPlatformTimer_h
