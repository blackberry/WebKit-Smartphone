/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef BlackBerryPlatformCookieJar_h
#define BlackBerryPlatformCookieJar_h

#include "UnsharedPointer.h"

namespace BlackBerry {
    namespace Platform {

        bool getCookieString(int playerId, const char* url, UnsharedArray<char>& result);
        bool setCookieString(int playerId, const char* url, const char* cookieString);

    } // namespace Olympia
} // namespace Platform

#endif // BlackBerryPlatformCookieJar_h
