/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "CookieJar.h"

#include "CString.h"
#include "Document.h"
#include "Frame.h"
#include "FrameLoaderClientOlympia.h"
#include "KURL.h"
#include "OlympiaCookieCache.h"
#include "OlympiaPlatformCookieJar.h"
#include "Page.h"
#include "PageGroupLoadDeferrer.h"

#include "NotImplemented.h"
#include "WebSettings.h"

namespace WebCore {

static int getPlayerId(Frame* frame)
{
    return static_cast<FrameLoaderClientOlympia*>(frame->loader()->client())->playerId();
}

String cookies(Document const* document, KURL const& url)
{
    Frame* frame = document->frame();
    if (!frame)
        return String();

    Page* page = frame->page();
    if (!page)
        return String();

    bool isCookieCacheEnabled = Olympia::WebKit::WebSettings::pageGroupSettings(page->groupName())->isCookieCacheEnabled();

    if (isCookieCacheEnabled) {
        String cachedCookies = OlympiaCookieCache::instance().cookies(page->groupName(), url);
        if (!cachedCookies.isEmpty())
            return cachedCookies;
    }

    PageGroupLoadDeferrer deferrer(page, true);

    UnsharedArray<char> cookies;
    if (!Olympia::Platform::getCookieString(getPlayerId(frame), url.string().latin1().data(), cookies))
        return String();

    String result = String(cookies.get());

    if (isCookieCacheEnabled)
        OlympiaCookieCache::instance().setCookies(page->groupName(), url, result);

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
        OlympiaCookieCache::instance().clearAllCookiesForHost(page->groupName(), url);
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

void deleteCookie(const Document* document, const KURL& url, const String& cookieName)
{
    notImplemented();
}

String cookieRequestHeaderFieldValue(const Document* document, const KURL &url)
{
    notImplemented();
    return String();
}

} // namespace WebCore
