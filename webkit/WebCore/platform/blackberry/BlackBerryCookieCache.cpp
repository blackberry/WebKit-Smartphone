/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "BlackBerryCookieCache.h"

#include "CString.h"
#include "KURL.h"
#include "OlympiaPlatformMisc.h"
#include <wtf/Assertions.h>
#include <wtf/CurrentTime.h>
#include <wtf/Vector.h>

namespace WebCore {

using Olympia::Platform::log;
using Olympia::Platform::LogLevelInfo;

static const int cookieCacheCapacity = 1 * 1024 * 1024; // 1M bytes
static const int cookieCacheExpireTime = 60; // seconds

BlackBerryCookieCache& BlackBerryCookieCache::instance()
{
    static BlackBerryCookieCache cache(cookieCacheCapacity);
    return cache;
}

BlackBerryCookieCache::BlackBerryCookieCache(int capacity)
    : m_capacity(capacity)
    , m_size(0)
{
}

String BlackBerryCookieCache::cookies(const String& pageGroup, const KURL& url)
{
    String key = url.string().lower();
    String host = url.host().lower();
    if (pageGroup.isEmpty() || key.isEmpty() || host.isEmpty())
        return String();

    // Lookup cookie by page group name, host name and URL in 3 level
    // hash map.
    PageGroupMap::iterator groupIterator = m_groupMap.find(pageGroup);
    if (groupIterator != m_groupMap.end()) {
        HostMap::iterator hostIterator = groupIterator->second->find(host);
        if (hostIterator != groupIterator->second->end()) {
            ItemMap::iterator itemIterator = hostIterator->second->find(key);
            if (itemIterator != hostIterator->second->end() && currentTime() < itemIterator->second.expireTime) {
                log(LogLevelInfo, "URL:%s:cookies:%s hit in cookie cache\n", key.latin1().data(), itemIterator->second.cookies.latin1().data());
                return itemIterator->second.cookies;
            }

            // If the cookies is "expired", just let it be there for quick
            // return of this call. purge will remove "expired" cookies
            // when there isn't enough room.
        }
    }
    log(LogLevelInfo, "URL:%s miss in cookie cache\n", key.latin1().data());
    return String();
}

bool BlackBerryCookieCache::setCookies(const String& pageGroup, const KURL& url, const String& cookies)
{
    String key = url.string().lower();
    String host = url.host().lower();
    if (pageGroup.isEmpty() || key.isEmpty() || host.isEmpty())
        return false;

    int maxRequiredSize = (host.length() + key.length() + cookies.length()) * sizeof(UChar);

    // Just purge storage when there isn't enough room.
    // For simplicity, we use maximum required size here.
    if (m_size + maxRequiredSize > m_capacity) {
        purgeStorage();
        if (m_size + maxRequiredSize > m_capacity)
            return false;
    }

    // Add a host map for specified page group name if there isn't suitable one.
    PageGroupMap::iterator groupIterator = m_groupMap.find(pageGroup);
    if (groupIterator == m_groupMap.end()) {
        HostMap* map = new HostMap;
        pair<PageGroupMap::iterator, bool> result = m_groupMap.add(pageGroup, map);
        if (result.second)
            groupIterator = result.first;
        else {
            delete map;
            return false;
        }
    }

    ASSERT(groupIterator != m_groupMap.end());

    // Add a item map for speicified host map if there isn't suitable one.
    HostMap::iterator hostIterator = groupIterator->second->find(host);
    if (hostIterator == groupIterator->second->end()) {
        ItemMap* map = new ItemMap;
        pair<HostMap::iterator, bool> result = groupIterator->second->add(host, map);
        if (result.second)
            hostIterator = result.first;
        else {
            delete map;
            return false;
        }
    } else {
        // There is a entry for the host, no need to count host length.
        maxRequiredSize -= host.length() * sizeof(UChar);
    }

    ASSERT(hostIterator != groupIterator->second->end());

    BlackBerryCookieCacheItem item;
    item.cookies = cookies;
    item.expireTime = currentTime() + cookieCacheExpireTime;
    m_size += maxRequiredSize;
    hostIterator->second->set(key, item);

    return true;
}

int BlackBerryCookieCache::purgeStorage()
{
    // purge storage based on "expire" time.
    PageGroupMap::iterator groupIterator = m_groupMap.begin();
    PageGroupMap::iterator groupIteratorEnd = m_groupMap.end();
    int reduced = 0;
    Vector<String> groupNames;
    for (; groupIterator != groupIteratorEnd; ++groupIterator) {
        HostMap::iterator hostIterator = groupIterator->second->begin();
        HostMap::iterator hostIteratorEnd = groupIterator->second->end();
        Vector<String> hosts;
        for (; hostIterator != hostIteratorEnd; ++hostIterator) {
            ItemMap::iterator itemIterator = hostIterator->second->begin();
            ItemMap::iterator itemIteratorEnd = hostIterator->second->end();
            Vector<String> urls;
            for (; itemIterator != itemIteratorEnd; ++itemIterator) {
                if (currentTime() >= itemIterator->second.expireTime) {
                    reduced += (itemIterator->first.length() + itemIterator->second.cookies.length()) * sizeof(UChar);
                    urls.append(itemIterator->first);
                }
            }
            for (int i = 0; i < urls.size(); ++i)
                hostIterator->second->remove(urls[i]);

            if (hostIterator->second->isEmpty())
                hosts.append(hostIterator->first);
        }
        for (int i = 0; i < hosts.size(); ++i) {
            HostMap::iterator hostIterator = groupIterator->second->find(hosts[i]);
            delete hostIterator->second;
            groupIterator->second->remove(hostIterator);
            reduced += hosts[i].length() * sizeof(UChar);
        }

        if (groupIterator->second->isEmpty())
            groupNames.append(groupIterator->first);
    }
    for (int i = 0; i < groupNames.size(); ++i) {
        PageGroupMap::iterator groupIterator = m_groupMap.find(groupNames[i]);
        delete groupIterator->second;
        m_groupMap.remove(groupIterator);
    }

    m_size -= reduced;
    log(LogLevelInfo, "%d bytes cachedcookie purged\n", reduced);
    return reduced;
}


void BlackBerryCookieCache::clearAllCookiesForHost(const String& pageGroup, const KURL& url)
{
    // FIXME: actually, we only need to clear the cookies for which the protocol+host match the url
    String host = url.host().lower();
    if (pageGroup.isEmpty() || host.isEmpty())
        return;

    // Clear all cookie for specified page group name and host.
    PageGroupMap::iterator groupIterator = m_groupMap.find(pageGroup);
    if (groupIterator != m_groupMap.end()) {
        HostMap::iterator hostIterator = groupIterator->second->find(host);
        if (hostIterator != groupIterator->second->end()) {
            ItemMap::iterator itemIterator = hostIterator->second->begin();
            ItemMap::iterator itemIteratorEnd = hostIterator->second->end();
            // Count down memory counter
            for (; itemIterator != itemIteratorEnd; ++itemIterator)
                m_size -= (itemIterator->first.length() + itemIterator->second.cookies.length()) * sizeof(UChar);
            m_size -= hostIterator->first.length() * sizeof(UChar);

            // Delete the host map pointer and remove it
            // from the group map.
            delete hostIterator->second;
            groupIterator->second->remove(hostIterator);
        }
    }
}

void BlackBerryCookieCache::clearAllCookies()
{
    PageGroupMap::iterator groupIterator = m_groupMap.begin();
    PageGroupMap::iterator groupIteratorEnd = m_groupMap.end();
    for (; groupIterator != groupIteratorEnd; ++groupIterator) {
        HostMap::iterator hostIterator = groupIterator->second->begin();
        HostMap::iterator hostIteratorEnd = groupIterator->second->end();
        for (; hostIterator != hostIteratorEnd; ++hostIterator)
            delete hostIterator->second;
        delete groupIterator->second;
    }
    m_groupMap.clear();
    m_size = 0;
}

}
