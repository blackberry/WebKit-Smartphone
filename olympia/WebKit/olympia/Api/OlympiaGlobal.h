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

class String;

void globalInitialize();
void collectJavascriptGarbageNow();
void clearCookieCache();
void clearMemoryCaches();
void clearAppCache(const String& pageGroupName);
void clearLocalStorage(const String& pageGroupName);
void clearDatabase(const String& pageGroupName);
void updateOnlineStatus(bool online);

}
}

#endif // OlympiaGlobal_h
