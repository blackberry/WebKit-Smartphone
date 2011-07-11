/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef OlympiaGlobal_h
#define OlympiaGlobal_h

#ifdef _MSC_VER
    #ifdef BUILD_WEBKIT
        #define OLYMPIA_EXPORT __declspec(dllexport)
    #else
        #define OLYMPIA_EXPORT __declspec(dllimport)
    #endif
#else
    #define OLYMPIA_EXPORT
#endif

namespace Olympia {
namespace WebKit {

class WebString;

void globalInitialize();
void collectJavascriptGarbageNow();
void clearCookieCache();
void clearMemoryCaches();
void clearAppCache(const WebString& pageGroupName);
void reopenAllAppCaches();
void closeAllAppCaches();
void clearLocalStorage(const WebString& pageGroupName);
void closeAllLocalStorages();
void clearDatabase(const WebString& pageGroupName);
void reopenAllTrackerDatabases();
void closeAllTrackerDatabases();
void updateOnlineStatus(bool online);
}
}

#endif // OlympiaGlobal_h
