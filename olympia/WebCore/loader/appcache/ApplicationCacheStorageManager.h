/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef ApplicationCacheStorageManager_h
#define ApplicationCacheStorageManager_h

#if ENABLE(OFFLINE_WEB_APPLICATIONS)

#include "PlatformString.h"
#include "StringHash.h"
#include <wtf/HashTraits.h>
#include <wtf/HashMap.h>

namespace WebCore {

class ApplicationCacheStorage;

class ApplicationCacheStorageManager : public Noncopyable {
public:
    ApplicationCacheStorage& cacheStorage(const String& groupName);

private:
    ApplicationCacheStorageManager();
    ~ApplicationCacheStorageManager();

    typedef HashMap<String, ApplicationCacheStorage*> CacheStorageMap;
    CacheStorageMap m_cacheStorages;

    ApplicationCacheStorage* m_defaultStorage;

    friend ApplicationCacheStorageManager& cacheStorageManager();
};

ApplicationCacheStorageManager& cacheStorageManager();

} // namespace WebCore

#endif // ENABLE(OFFLINE_WEB_APPLICATIONS)

#endif // ApplicationCacheStorageManager_h
