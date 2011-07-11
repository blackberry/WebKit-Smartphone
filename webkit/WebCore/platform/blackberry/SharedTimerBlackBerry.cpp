/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#include "config.h"
#include "SharedTimer.h"

#include "CurrentTime.h"
#include "OlympiaPlatformTimer.h"

namespace WebCore {

class SharedTimerBlackBerry {
    friend void setSharedTimerFiredFunction(void (*f)());

public:
    static SharedTimerBlackBerry* inst();

    void start(double);
    void stop();

    bool m_started;
    void (*m_timerFunction)();
private:
    SharedTimerBlackBerry();
    ~SharedTimerBlackBerry();
};

static SharedTimerBlackBerry* s_timer = 0;

SharedTimerBlackBerry::SharedTimerBlackBerry()
    : m_started(false)
    , m_timerFunction(0)
{
}

SharedTimerBlackBerry::~SharedTimerBlackBerry()
{
    if (m_started)
        stop();
}

SharedTimerBlackBerry* SharedTimerBlackBerry::inst()
{
    if (!s_timer)
        s_timer = new SharedTimerBlackBerry();
    return s_timer;
}

void SharedTimerBlackBerry::start(double fireTime)
{
    Olympia::Platform::timerStart(fireTime - currentTime(), m_timerFunction);
    m_started = true;
}

void SharedTimerBlackBerry::stop()
{
    if (m_started) {
        Olympia::Platform::timerStop();
        m_started = false;
    }
}

void setSharedTimerFiredFunction(void (*f)())
{
    SharedTimerBlackBerry::inst()->m_timerFunction = f;
}

void setSharedTimerFireTime(double fireTime)
{
    SharedTimerBlackBerry::inst()->start(fireTime);
}

void stopSharedTimer()
{
    SharedTimerBlackBerry::inst()->stop();
}

} // namespace WebCore

