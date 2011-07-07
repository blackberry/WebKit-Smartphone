/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "OlympiaGlobal.h"

#include "ApplicationCacheStorage.h"
#include "Cache.h"
#include "CacheClientOlympia.h"
#include "CrossOriginPreflightResultCache.h"
#include "DatabaseTracker.h"
#include "GCController.h"
#include "InitializeThreading.h"
#include "ImageSource.h"
#include "Logging.h"
#include "MainThread.h"
#include "NetworkStateNotifier.h"
#include "ObjectAllocator.h"
#include "OlympiaCookieCache.h"
#include "OlympiaPlatformSettings.h"
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

    WebCore::CacheClientOlympia::get()->initialize();

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
    WebCore::OlympiaCookieCache::instance().clearAllCookies();
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

void updateOnlineStatus(bool online)
{
    WebCore::networkStateNotifier().networkStateChange(online);
}

}
}

