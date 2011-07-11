/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "CookieJar.h"

#include "CString.h"
#include "Document.h"
#include "Frame.h"
#include "FrameLoaderClientBlackBerry.h"
#include "KURL.h"
#include "BlackBerryCookieCache.h"
#include "OlympiaPlatformCookieJar.h"
#include "Page.h"
#include "PageGroupLoadDeferrer.h"

#include "NotImplemented.h"
#include "WebSettings.h"

namespace WebCore {

static int getPlayerId(Frame* frame)
{
    return static_cast<FrameLoaderClientBlackBerry*>(frame->loader()->client())->playerId();
}

WTF::String cookies(Document const* document, KURL const& url)
{
    Frame* frame = document->frame();
    if (!frame)
        return WTF::String();

    Page* page = frame->page();
    if (!page)
        return WTF::String();

    bool isCookieCacheEnabled = Olympia::WebKit::WebSettings::pageGroupSettings(page->groupName())->isCookieCacheEnabled();

    if (isCookieCacheEnabled) {
        WTF::String cachedCookies = BlackBerryCookieCache::instance().cookies(page->groupName(), url);
        if (!cachedCookies.isEmpty())
            return cachedCookies;
    }

    PageGroupLoadDeferrer deferrer(page, true);

    UnsharedArray<char> cookies;
    if (!Olympia::Platform::getCookieString(getPlayerId(frame), url.string().latin1().data(), cookies))
        return WTF::String();

    WTF::String result = WTF::String(cookies.get());

    if (isCookieCacheEnabled)
        BlackBerryCookieCache::instance().setCookies(page->groupName(), url, result);

    return result;
}

void setCookies(Document* document, KURL const& url, String const& value)
{
    Frame* frame = document->frame();
    if (!frame)
        return;

    Page* page = frame->page();
    if (!page)
        return;

    if (Olympia::Platform::setCookieString(getPlayerId(frame), url.string().latin1().data(), value.latin1().data())
        && Olympia::WebKit::WebSettings::pageGroupSettings(page->groupName())->isCookieCacheEnabled())
        BlackBerryCookieCache::instance().clearAllCookiesForHost(page->groupName(), url);
}

bool cookiesEnabled(Document const* document)
{
    // Fix me. Currently cookie is enabled by default, no setting on property page.
    return true;
}

bool getRawCookies(const Document* document, const KURL& url, Vector<Cookie>& rawCookies)
{
    notImplemented();
    return false;
}

void deleteCookie(const Document* document, const KURL& url, const WTF::String& cookieName)
{
    notImplemented();
}

WTF::String cookieRequestHeaderFieldValue(const Document* document, const KURL &url)
{
    notImplemented();
    return WTF::String();
}

} // namespace WebCore
