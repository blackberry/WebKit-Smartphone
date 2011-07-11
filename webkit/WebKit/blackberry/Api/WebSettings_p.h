/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef WebSettings_p_h
#define WebSettings_p_h

#include "IntSize.h"
#include <wtf/Vector.h>

namespace WebCore {
    class Page;
}

namespace Olympia {
namespace WebKit {

class WebString;
struct WebCoreSettingsState;
struct OlympiaSettingsState;

class WebSettingsPrivate {
public:
    WebSettingsPrivate(const WebString&);
    WebSettingsPrivate(WebCore::Page*, const WebString&);
    ~WebSettingsPrivate();

    /* Applies settings to the page settings */
    virtual void apply(int setting, bool global);

    bool isGlobal() { return !m_page; }

    WebCore::Page* m_page;
    WebCoreSettingsState* m_webCoreSettingsState;
    OlympiaSettingsState* m_olympiaSettingsState;
    WTF::Vector<WebSettingsPrivate*> m_pageSettings;
    int m_screenWidth;
    int m_screenHeight;
    WebCore::IntSize m_applicationViewSize; // The size of the web page view and the browser chrome.
};

}
}

#endif // WebSettings_p_h
