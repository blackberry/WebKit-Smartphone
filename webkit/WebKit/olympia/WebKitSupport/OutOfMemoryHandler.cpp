/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#include "config.h"
#include "OutOfMemoryHandler.h"

#include "Cache.h"
#include "CacheClientOlympia.h"
#include "GCController.h"
#include "ObjectAllocator.h"
#include "OlympiaCookieCache.h"
#include "OlympiaPlatformSettings.h"

namespace Olympia {
namespace WebKit {

unsigned OutOfMemoryHandler::handleOutOfMemory(unsigned)
{
    unsigned bytesFreed = Platform::ObjectAllocator::collectAll();

    // FIXME: Add more clean-up actions here

    return bytesFreed;
}

void OutOfMemoryHandler::shrinkMemoryUsage()
{
    WebCore::gcController().garbageCollectNow();

    // Clean cache after JS garbage collection because JS GC can
    // generate more dead resources.
    if (!WebCore::cache()->disabled()) {
        // setDisabled(true) will try to evict all cached resources
        WebCore::cache()->setDisabled(true);
        // FIXME: should we permanently turn it off?
        WebCore::cache()->setDisabled(false);

        // Probably we want to shrink cache capacity at this point
        WebCore::CacheClientOlympia::get()->updateCacheCapacity();
    }

    WebCore::OlympiaCookieCache::instance().clearAllCookies();

    // Do this at last because above actions may generate new garbages in ObjectAllocator.
    Platform::ObjectAllocator::collectAll();
}

void initializeOutOfMemoryHandler()
{
    static OutOfMemoryHandler* handler = new OutOfMemoryHandler;

    Olympia::Platform::setOOMHandler(handler);
}

}
}