/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef WebPlugin_h
#define WebPlugin_h

#include "CString.h"
#include "PlatformString.h"
#include "PluginWidget.h"
#include "FrameLoaderClientOlympia.h"

namespace WebCore {
class IntRect;
class HTMLElement;
class PluginWidget;
class FrameLoaderClientOlympia;
}

namespace Olympia {
namespace WebKit {
class WebPage;

class WebPlugin : public RefCounted<WebPlugin> {
public:
    WebPlugin(WebPage* webPage, int width, int height, WebCore::HTMLElement* node, const WebCore::String& url = WebCore::String(), bool isHtml5Video = false, bool hasRenderer = false);
    ~WebPlugin();
    void setGeometry(const WebCore::IntRect&);
    void destroy();
    void detach();
    int playerID() const { return m_playerID; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    bool isLoaded() const { return m_isLoaded; }
    void setLoaded(bool isLoaded) { m_isLoaded = isLoaded; }
    bool shouldBeCancelled() const { return m_shouldBeCancelled; }
    void setShouldBeCancelled(bool shouldBeCancelled) { m_shouldBeCancelled = shouldBeCancelled; }
    WebCore::String url() const { return m_url; }

    // JS control.
    void play();
    void pause();

    WebCore::HTMLElement* element() const { return m_element; }
    void setElement(WebCore::HTMLElement* element) { m_element = element; }

    void setWidget(RefPtr<WebCore::PluginWidget> widget) { m_widget = widget; }
    PassRefPtr<WebCore::PluginWidget> widget() const { return m_widget; }

private:
    int m_playerID;
    WebPage* m_webPage;
    int m_width;
    int m_height;
    WebCore::HTMLElement* m_element;
    WebCore::String m_url;
    RefPtr<WebCore::PluginWidget> m_widget;
    bool m_isLoaded;
    bool m_shouldBeCancelled; 
};

}
}

#endif
