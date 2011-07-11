/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef WebPlugin_h
#define WebPlugin_h

#include "CString.h"
#include "PlatformString.h"

namespace WebCore {
class HTMLElement;
class IntRect;
class IntSize;
class PluginWidget;
class WebMediaPlayerProxy;
}

namespace Olympia {
namespace WebKit {
class WebPage;

class WebPlugin : public RefCounted<WebPlugin> {
public:
    WebPlugin(WebPage* webPage, int width, int height, WebCore::HTMLElement* node, const Vector<WTF::String>& paramNames, const Vector<WTF::String>& paramValues, const WTF::String& url = WTF::String());
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
    WTF::String url() const { return m_url; }

    // JS control.
    void play();
    void pause();
    void load(const WTF::String& url, WebCore::WebMediaPlayerProxy* proxy);

    float time();
    float duration() const { return m_duration;}
    void seek(float time);

    void setVolume(float volume);
    float volume() const { return m_volume;}

    void setMuted(bool muted);
    bool muted() const { return m_muted; }

    void setSize(const WebCore::IntSize& size);

    bool autoplay() const { return m_autoplay; }
    void setAutoplay(bool autoplay) { m_autoplay = autoplay; }

    WebCore::HTMLElement* element() const { return m_element; }
    void setElement(WebCore::HTMLElement* element) { m_element = element; }

    void setWidget(RefPtr<WebCore::PluginWidget> widget);
    PassRefPtr<WebCore::PluginWidget> widget() const;

    void readyStateChanged(int state); 
    void volumeChanged(int volume);
    void durationChanged(float duration);

private:
    int m_playerID;
    WebPage* m_webPage;
    int m_width;
    int m_height;
    WebCore::HTMLElement* m_element;
    WebCore::WebMediaPlayerProxy * m_proxy;
    WTF::String m_url;
    RefPtr<WebCore::PluginWidget> m_widget;
    bool m_isLoaded;
    bool m_shouldBeCancelled;
    bool m_muted;
    bool m_autoplay;
    float m_volume;
    float m_duration;
    Vector<WTF::String> m_paramNames;
    Vector<WTF::String> m_paramValues;
};

}
}

#endif
