/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"

#include "MainThread.h"
#include "OlympiaPlatformMisc.h"

namespace WTF {

void initializeMainThreadPlatform()
{
}

void scheduleDispatchFunctionsOnMainThread()
{
#if ENABLE(SINGLE_THREADED)
    dispatchFunctionsFromMainThread();
#else
    Olympia::Platform::scheduleCallOnMainThread(dispatchFunctionsFromMainThread);
#endif
}

} // namespace WTF
