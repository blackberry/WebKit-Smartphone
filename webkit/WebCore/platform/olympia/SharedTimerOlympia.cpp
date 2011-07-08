/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#include "config.h"
#include "SharedTimer.h"

#include "CurrentTime.h"
#include "OlympiaPlatformTimer.h"

namespace WebCore {

class SharedTimerOlympia {
    friend void setSharedTimerFiredFunction(void (*f)());

public:
    static SharedTimerOlympia* inst();

    void start(double);
    void stop();

    bool m_started;
    void (*m_timerFunction)();
private:
    SharedTimerOlympia();
    ~SharedTimerOlympia();
};

static SharedTimerOlympia* s_timer = 0;

SharedTimerOlympia::SharedTimerOlympia()
    : m_started(false)
    , m_timerFunction(0)
{
}

SharedTimerOlympia::~SharedTimerOlympia()
{
    if (m_started)
        stop();
}

SharedTimerOlympia* SharedTimerOlympia::inst()
{
    if (!s_timer)
        s_timer = new SharedTimerOlympia();
    return s_timer;
}

void SharedTimerOlympia::start(double fireTime)
{
    Olympia::Platform::timerStart(fireTime - currentTime(), m_timerFunction);
    m_started = true;
}

void SharedTimerOlympia::stop()
{
    if (m_started) {
        Olympia::Platform::timerStop();
        m_started = false;
    }
}

void setSharedTimerFiredFunction(void (*f)())
{
    SharedTimerOlympia::inst()->m_timerFunction = f;
}

void setSharedTimerFireTime(double fireTime)
{
    SharedTimerOlympia::inst()->start(fireTime);
}

void stopSharedTimer()
{
    SharedTimerOlympia::inst()->stop();
}

} // namespace WebCore

