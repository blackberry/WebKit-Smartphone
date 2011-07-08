/*
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */

#include "config.h"
#include "CacheClientOlympia.h"

#include "OlympiaPlatformMisc.h"
#include "OlympiaPlatformSettings.h"

namespace WebCore {

CacheClientOlympia* CacheClientOlympia::get()
{
    static CacheClientOlympia s_cacheClient;
    return &s_cacheClient;
}

CacheClientOlympia::CacheClientOlympia()
    : m_lastCapacity(0)
{
}

void CacheClientOlympia::initialize()
{
#if ENABLE(OLYMPIA_DEBUG_MEMORY)
    bool isDisabled = true;
#else
    bool isDisabled = false;
#endif
    cache()->setDisabled(isDisabled);
    if (!isDisabled) {
        cache()->setClient(this);
        // We have to set a non-zero interval to schedule cache pruning after a CachedImage becoming dead
        cache()->setDeadDecodedDataDeletionInterval(0.01);
        updateCacheCapacity();
    }
}

void CacheClientOlympia::updateCacheCapacity()
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

void CacheClientOlympia::didAddToLiveResourcesSize(CachedResource*)
{
    // Check suggested cache capacity at this point
    updateCacheCapacity();
}

void CacheClientOlympia::didRemoveFromLiveResourcesSize(CachedResource*)
{
    // Check suggested cache capacity at this point
    updateCacheCapacity();
}

} // WebCore
