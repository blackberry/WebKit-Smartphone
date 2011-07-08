/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "ApplicationCacheStorageManager.h"

#if ENABLE(OFFLINE_WEB_APPLICATIONS)

#include "ApplicationCacheStorage.h"
#include <wtf/text/CString.h>

namespace WebCore {

ApplicationCacheStorageManager::ApplicationCacheStorageManager()
    : m_defaultStorage(0)
{
}

ApplicationCacheStorageManager::~ApplicationCacheStorageManager()
{
    delete m_defaultStorage;

    for (CacheStorageMap::iterator iter = m_cacheStorages.begin(); iter != m_cacheStorages.end(); ++iter)
        delete iter->second;

    m_cacheStorages.clear();
}

ApplicationCacheStorage& ApplicationCacheStorageManager::cacheStorage(const String& groupName)
{
    if (groupName.isEmpty()) {
        if (!m_defaultStorage)
            m_defaultStorage = new ApplicationCacheStorage();

        return *m_defaultStorage;
    }

    ApplicationCacheStorage* storage = m_cacheStorages.get(groupName);
    if (!storage) {
        storage = new ApplicationCacheStorage();
        m_cacheStorages.add(groupName, storage);
    }

    return *storage;
}

ApplicationCacheStorageManager& cacheStorageManager()
{
    DEFINE_STATIC_LOCAL(ApplicationCacheStorageManager, manager, ());

    return manager;
}

} // namespace WebCore

#endif // ENABLE(OFFLINE_WEB_APPLICATIONS)
