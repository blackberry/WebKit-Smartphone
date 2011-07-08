/*
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */

#ifndef CacheClientOlympia_h
#define CacheClientOlympia_h

#include "Cache.h"

namespace WebCore {

class CacheClientOlympia: public CacheClient {
public:
    static CacheClientOlympia* get();

    void initialize();
    void updateCacheCapacity();

    // Derived from CacheClient
    virtual void didAddToLiveResourcesSize(CachedResource* resource);
    virtual void didRemoveFromLiveResourcesSize(CachedResource* resource);
private:
    CacheClientOlympia();
    unsigned m_lastCapacity;
};


} // WebCore

#endif // CacheClientOlympia_h
