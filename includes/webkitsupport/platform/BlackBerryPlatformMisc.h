/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef BlackBerryPlatformMisc_h
#define BlackBerryPlatformMisc_h

#include "BlackBerryPlatformPrimitives.h"
#include "memorymanagement/VirtualMemoryManager.h"

// FIXME these should be removed, here for backwards compatibility
// with BlackBerryPlatformMisc.h before refactoring
#include "BlackBerryPlatformCursor.h"
#include "BlackBerryPlatformInputEvents.h"
#include "BlackBerryPlatformLog.h"
#include "BlackBerryPlatformWorker.h"

#include <stdarg.h>
#include <string>

#ifndef BRIDGE_IMPORT
#ifdef _MSC_VER
#define BRIDGE_IMPORT __declspec(dllimport)
#else
#define BRIDGE_IMPORT
#endif
#endif

namespace BlackBerry {
    namespace Platform {
        // Return current UTC time in seconds
        double currentUTCTimeMS();

        double tickCount();

        void* stackBase();
        void addStackBase(void* base);
        void removeStackBase();

        void willRunEventLoop();
        void processNextEvent();

        void scheduleLazyInitialization();

        void scheduleCallOnMainThread(void(*callback)(void));

        const char* environment(const char* key);

#ifdef _MSC_VER
        // FIXME: on device, environment is just a wrapper around getenv.  On
        // simulator, we need to add strings to the environment by hand.
        // see http://bugzilla-torch.rim.net/show_bug.cgi?id=707; remove this function when it is fixed
        void addToEnvironment(const char* key, const char* value);
#endif

        struct MemoryStatus {
            unsigned baseAddress;
            unsigned committedPhysicalMemory;
            unsigned availableVirtualMemory;
            unsigned availablePhysicalMemory;
            unsigned lowPhysicalMemoryThreshold;

            unsigned systemAllocatedMemory;
            unsigned usedMemory;
            unsigned freeMemory;
        };

        void getMemoryStatus(MemoryStatus& status);

    } // Platform

} // Olympia

#endif // BlackBerryPlatformMisc_h
