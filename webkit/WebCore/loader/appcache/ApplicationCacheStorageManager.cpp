/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "ApplicationCacheStorageManager.h"

#if ENABLE(OFFLINE_WEB_APPLICATIONS)

#include "ApplicationCacheStorage.h"
#include <wtf/text/CString.h>

#if OS(OLYMPIA)
#include "ThreadingPrimitives.h"
static Mutex& databaseMutex()
{
    static Mutex* s_databaseMutex = new Mutex;
    return *s_databaseMutex;
}
#endif // OS(OLYMPIA)

namespace WebCore {

ApplicationCacheStorageManager::ApplicationCacheStorageManager()
    : m_defaultStorage(0)
{
}

ApplicationCacheStorageManager::~ApplicationCacheStorageManager()
{
#if OS(OLYMPIA)
    MutexLocker lock(databaseMutex());
#endif // OS(OLYMPIA)
    delete m_defaultStorage;

    for (CacheStorageMap::iterator iter = m_cacheStorages.begin(); iter != m_cacheStorages.end(); ++iter)
        delete iter->second;

    m_cacheStorages.clear();
}

ApplicationCacheStorage& ApplicationCacheStorageManager::cacheStorage(const String& groupName)
{
#if OS(OLYMPIA)
    MutexLocker lock(databaseMutex());
#endif // OS(OLYMPIA)
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

#if OS(OLYMPIA)
void ApplicationCacheStorageManager::closeAll()
{
    MutexLocker lock(databaseMutex());
    ApplicationCacheStorageManager& manager = cacheStorageManager();
    if (manager.m_defaultStorage)
        manager.m_defaultStorage->closeDatabase();

    for (CacheStorageMap::iterator iter = manager.m_cacheStorages.begin(); iter != manager.m_cacheStorages.end(); ++iter) {
        if (iter->second)
            iter->second->closeDatabase();
    }

}
void ApplicationCacheStorageManager::reopenAll()
{
    MutexLocker lock(databaseMutex());
    ApplicationCacheStorageManager& manager = cacheStorageManager();
    if (manager.m_defaultStorage)
        manager.m_defaultStorage->reopenDatabase();

    for (CacheStorageMap::iterator iter = manager.m_cacheStorages.begin(); iter != manager.m_cacheStorages.end(); ++iter) {
        if (iter->second)
            iter->second->reopenDatabase();
    }

}
#endif // OS(OLYMPIA)


} // namespace WebCore

#endif // ENABLE(OFFLINE_WEB_APPLICATIONS)
