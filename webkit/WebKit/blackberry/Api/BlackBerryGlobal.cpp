/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "BlackBerryGlobal.h"

#include "ApplicationCacheStorage.h"
#include "ApplicationCacheStorageManager.h"
#include "Cache.h"
#include "CacheClientBlackBerry.h"
#include "CrossOriginPreflightResultCache.h"
#include "DatabaseTracker.h"
#include "DatabaseTrackerManager.h"
#include "GCController.h"
#include "InitializeThreading.h"
#include "ImageSource.h"
#include "Logging.h"
#include "MainThread.h"
#include "NetworkStateNotifier.h"
#include "ObjectAllocator.h"
#include "BlackBerryCookieCache.h"
#include "OlympiaPlatformSettings.h"
#include "WebString.h"
#include "OutOfMemoryHandler.h"
#include "PageCache.h"
#include "PageGroup.h"

namespace Olympia {
namespace WebKit {

static bool gIsGlobalInitialized = false;

// global initialize of various WebKit singletons and such
void globalInitialize()
{
    if (gIsGlobalInitialized)
        return;
    gIsGlobalInitialized = true;

#if ENABLE(OLYMPIA_DEBUG_MEMORY)
    olympiaDebugInitialize();
#endif

    Olympia::Platform::initializeObjectAllocators();

    Olympia::WebKit::initializeOutOfMemoryHandler();

    // Turn on logging
    WebCore::InitializeLoggingChannelsIfNecessary();

    // Initialize threading
    JSC::initializeThreading();

    // normally this is called from initializeThreading, but we're using ThreadingNone
    // we're grabbing callOnMainThread without using the rest of the threading support
    WTF::initializeMainThread();

    // No page cache
    WebCore::pageCache()->setCapacity(0);

    // Track visited links
    WebCore::PageGroup::setShouldTrackVisitedLinks(true);

    WebCore::CacheClientBlackBerry::get()->initialize();

    Olympia::Platform::Settings* settings = Olympia::Platform::Settings::get();

    WebCore::ImageSource::setMaxPixelsPerDecodedImage(settings->maxPixelsPerDecodedImage());
}

void collectJavascriptGarbageNow()
{
    if (gIsGlobalInitialized)
        WebCore::gcController().garbageCollectNow();
}

void clearCookieCache()
{
    WebCore::BlackBerryCookieCache::instance().clearAllCookies();
}

void clearMemoryCaches()
{
    int capacity = WebCore::pageCache()->capacity();
    WebCore::pageCache()->setCapacity(0);
    WebCore::pageCache()->setCapacity(capacity);

    WebCore::CrossOriginPreflightResultCache::shared().empty();

#if !ENABLE(OLYMPIA_DEBUG_MEMORY)
    // setDisabled(true) will try to evict all cached resources
    WebCore::cache()->setDisabled(true);
    WebCore::cache()->setDisabled(false);
#endif
}

void clearAppCache(const WebString& pageGroupName)
{
    WTF::String name(pageGroupName.characters(), pageGroupName.length());

    WebCore::cacheStorage(name).empty();
}

void closeAllAppCaches()
{
    WebCore::ApplicationCacheStorageManager::closeAll();
}
void reopenAllAppCaches()
{
    WebCore::ApplicationCacheStorageManager::reopenAll();
}

void clearLocalStorage(const WebString& pageGroupName)
{
    WTF::String name(pageGroupName.characters(), pageGroupName.length());

    WebCore::PageGroup* group = WebCore::PageGroup::pageGroup(name);
    if (!group)
        return;

    group->removeLocalStorage();
}

void closeAllLocalStorages()
{
    WebCore::PageGroup::removeAllLocalStorages();
}
void clearDatabase(const WebString& pageGroupName)
{
    WTF::String name(pageGroupName.characters(), pageGroupName.length());

    WebCore::DatabaseTracker::tracker(name).deleteAllDatabases();
}
void reopenAllTrackerDatabases()
{
    WebCore::DatabaseTrackerManager::reopenAllTrackerDatabases();
}
void closeAllTrackerDatabases()
{
    WebCore::DatabaseTrackerManager::closeAllTrackerDatabases();
}

void updateOnlineStatus(bool online)
{
    WebCore::networkStateNotifier().networkStateChange(online);
}

}
}

