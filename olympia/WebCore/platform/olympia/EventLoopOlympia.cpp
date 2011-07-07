/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "EventLoop.h"

#include "OlympiaPlatformMisc.h"

namespace WebCore {

EventLoop::EventLoop()
    : m_ended(false)
{
    Olympia::Platform::willRunEventLoop();
}

void EventLoop::cycle()
{
    Olympia::Platform::processNextEvent();
}

} // namespace WebCore
