/*
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */

#include "config.h"
#include "CacheClientBlackBerry.h"

#include "Cache.h"
#include "OlympiaPlatformMisc.h"
#include "OlympiaPlatformSettings.h"

namespace WebCore {

CacheClientBlackBerry* CacheClientBlackBerry::get()
{
    static CacheClientBlackBerry s_cacheClient;
    return &s_cacheClient;
}

CacheClientBlackBerry::CacheClientBlackBerry()
    : m_lastCapacity(0)
{
}

void CacheClientBlackBerry::initialize()
{
#if ENABLE(OLYMPIA_DEBUG_MEMORY)
    bool isDisabled = true;
#else
    bool isDisabled = false;
#endif
    cache()->setDisabled(isDisabled);
    if (!isDisabled) {
        // We have to set a non-zero interval to schedule cache pruning after a CachedImage becoming dead
        cache()->setDeadDecodedDataDeletionInterval(0.01);
        updateCacheCapacity();
    }
}

void CacheClientBlackBerry::updateCacheCapacity()
{
#if ENABLE(OLYMPIA_DEBUG_MEMORY)
    // We're debugging memory usage. So keep it disabled
#else
    unsigned cacheCapacity = Olympia::Platform::Settings::get()->getSuggestedCacheCapacity(cache()->totalSize());
    if (m_lastCapacity == cacheCapacity) {
        // Suggested capacity hasn't been changed
        return;
    }

    m_lastCapacity = cacheCapacity;
    cache()->setCapacities(0, m_lastCapacity, m_lastCapacity);
#endif
}

} // WebCore
