/*
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */

#ifndef CacheClientOlympia_h
#define CacheClientOlympia_h

namespace WebCore {

class CacheClientBlackBerry {
public:
    static CacheClientBlackBerry* get();

    void initialize();
    void updateCacheCapacity();

private:
    CacheClientBlackBerry();
    unsigned m_lastCapacity;
};


} // WebCore

#endif // CacheClientOlympia_h
