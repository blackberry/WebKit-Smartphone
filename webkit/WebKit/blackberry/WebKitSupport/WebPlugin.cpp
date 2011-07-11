/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */
#include "config.h"
#include "IntRect.h"
#include "MediaPlayerProxy.h"
#include "PluginWidget.h"
#include "WebPlugin.h"
#include "WebPage.h"
#include "WebPage_p.h"
#include "WebPageClient.h"

namespace Olympia {
namespace WebKit {

WebPlugin::WebPlugin(WebPage* webPage, int width, int height, WebCore::HTMLElement* node, const Vector<WTF::String>& paramNames, const Vector<WTF::String>& paramValues, const WTF::String& url)
    : m_webPage(webPage)
    , m_width(width)
    , m_height(height)
    , m_element(node)
    , m_url(url)
    , m_isLoaded(false)
    , m_shouldBeCancelled(false)
    , m_muted(false)
    , m_autoplay(false)
    , m_volume(0)
    , m_duration(0)
    , m_paramNames(paramNames)
    , m_paramValues(paramValues)
    , m_proxy(0)
{
    m_playerID = reinterpret_cast<int>(this);
    if (!url.isEmpty()) {
        SharedArray<WebString> names(new WebString[paramNames.size()]);
        SharedArray<WebString> values(new WebString[paramValues.size()]);

        for (unsigned i = 0; i < paramNames.size(); i++) {
            names[i] = paramNames[i];
            values[i] = paramValues[i];
        }

        ASSERT(paramNames.size() == paramValues.size());

        m_webPage->client()->loadPluginForMimetype(m_playerID, width, height, names, values, paramValues.size(), url.utf8().data());
    }
}

WebPlugin::~WebPlugin()
{
    destroy();
}
void WebPlugin::setWidget(RefPtr<WebCore::PluginWidget> widget)
{
    m_widget = widget;
}

PassRefPtr<WebCore::PluginWidget> WebPlugin::widget() const
{
    return m_widget;
}

void WebPlugin::detach()
{
    m_webPage = 0;

    // Remove the reference in PluginWidget.
    if (m_widget)
        m_widget->setPlatformWidget(0);
}

void WebPlugin::setGeometry(const WebCore::IntRect& r)
{
    m_webPage->d->notifyPluginRectChanged(m_playerID, r);
}

void WebPlugin::destroy()
{
    if (m_webPage && m_webPage->client())
        m_webPage->client()->destroyPlugin(m_playerID);

    Olympia::Platform::log(Olympia::Platform::LogLevelWarn, "WebPlugin::destroy(): Plugin id: %d", m_playerID);

    detach();
}

void WebPlugin::play()
{
    // The load timer will be a bit delayed so it might already call play() before actually load it.
    // Set autoplay to true so when load is called, played it immediately.
    if (!m_isLoaded)
        m_autoplay = true;
    else if (m_webPage && m_webPage->client())
        m_webPage->client()->playMedia(m_playerID);
}

void WebPlugin::pause()
{
    if (m_webPage && m_webPage->client())
        m_webPage->client()->pauseMedia(m_playerID);
}

void WebPlugin::load(const WTF::String& url, WebCore::WebMediaPlayerProxy* proxy)
{
    if (url.isEmpty() || !m_webPage || !m_webPage->client()) 
       return;

    m_proxy = proxy;
    
    ASSERT(m_element);

    // Add autostart to parameters if hasn't set it before.
    bool isAutoStartSet = false;
    for (unsigned i = 0; i < m_paramNames.size(); i++) {
        if (m_paramNames[i] == "autostart") {
            if (m_autoplay) {
                m_paramValues[i] = "true";
                isAutoStartSet = true;
                break;
            }
        }
    }
    if (!isAutoStartSet && m_autoplay) {
        m_paramNames.append("autostart");
        m_paramValues.append("true");
    }

    SharedArray<WebString> names(new WebString[m_paramNames.size()]);
    SharedArray<WebString> values(new WebString[m_paramValues.size()]);

    for (unsigned i = 0; i < m_paramNames.size(); i++) {
        names[i] = m_paramNames[i];
        values[i] = m_paramValues[i];
    }

    ASSERT(m_paramNames.size() == m_paramValues.size());

    m_webPage->client()->loadPluginForMimetype(m_playerID, m_width, m_height, names, values, m_paramValues.size(), url.utf8().data());

    m_isLoaded = true;
}

float WebPlugin::time()
{
    if (m_webPage && m_webPage->client())
        return m_webPage->client()->getTime(m_playerID);

    return 0;
}

void WebPlugin::seek(float time)
{
    if (m_webPage && m_webPage->client()) {
        m_webPage->client()->pauseMedia(m_playerID);
        m_webPage->client()->setTime(m_playerID, time);
        m_webPage->client()->playMedia(m_playerID);
    }
}

void WebPlugin::setVolume(float volume)
{
    m_volume = volume;

    // Convert volume and sent it to client
    if (m_webPage && m_webPage->client())
        m_webPage->client()->setVolume(m_playerID, volume * 100);
}

void WebPlugin::setMuted(bool muted)
{
    m_muted = muted;
    if (m_webPage && m_webPage->client())
        m_webPage->client()->setMuted(m_playerID, muted);
}

void WebPlugin::setSize(const WebCore::IntSize& size)
{
    if (m_widget)
        m_widget->setSize(size);
}

void WebPlugin::readyStateChanged(int state)
{
    if (m_proxy)
        m_proxy->readyStateChanged(state);
}

void WebPlugin::volumeChanged(int volume)
{
    float newVolume = static_cast<float>(volume) / 100.0;
    if (newVolume == m_volume)
        return;

    m_volume = newVolume;
    if (m_proxy)
        m_proxy->volumeChanged(m_volume);
}

void WebPlugin::durationChanged(float duration)
{
    if (duration == m_duration)
        return;

    m_duration = duration;
    if (m_proxy)
        m_proxy->durationChanged(m_duration);
}

}
}
