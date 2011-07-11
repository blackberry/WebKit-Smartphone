/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef OlympiaCookieCache_h
#define OlympiaCookieCache_h

#include "PlatformString.h"
#include "StringHash.h"
#include <wtf/HashMap.h>
#include <wtf/Noncopyable.h>
#include <wtf/Vector.h>

namespace WebCore {

struct BlackBerryCookieCacheItem {
    String cookies;
    double expireTime;
};

class KURL;

class BlackBerryCookieCache: public Noncopyable {

public:
    static BlackBerryCookieCache& instance();
    String cookies(const String& pageGroup, const KURL& url);
    bool setCookies(const String& pageGroup, const KURL& url, const String& cookies);
    void clearAllCookiesForHost(const String& pageGroup, const KURL& url);
    void clearAllCookies();

private:
    BlackBerryCookieCache(int capacity);
    int purgeStorage();

    int m_capacity;
    int m_size;
    typedef HashMap<String, BlackBerryCookieCacheItem> ItemMap;
    typedef HashMap<String, ItemMap*> HostMap;
    typedef HashMap<String, HostMap*> PageGroupMap;
    PageGroupMap m_groupMap;
};

}
#endif
