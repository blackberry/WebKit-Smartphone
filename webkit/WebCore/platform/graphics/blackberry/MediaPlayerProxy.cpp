/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "MediaPlayerProxy.h"

#include "Frame.h"
#include "FrameLoaderClientBlackBerry.h"
#include "HTMLMediaElement.h"
#include "MediaPlayerPrivateBlackberry.h"
#include "PlatformString.h"
#include "RenderWidget.h"
#include "WebPlugin.h"
#include "Widget.h"

namespace WebCore {

WebMediaPlayerProxy::WebMediaPlayerProxy(MediaPlayerPrivate* player)
    : m_mediaPlayer(player)
    , m_init(false)
{
    if (!m_init)
        initEngine();
}

WebMediaPlayerProxy::~WebMediaPlayerProxy()
{
}

ScriptInstance WebMediaPlayerProxy::pluginInstance()
{
    if (!m_instance) {
        RenderObject* r = element()->renderer();
        if (!r || !r->isWidget())
            return 0;

        Frame* frame = element()->document()->frame();

        RenderWidget* renderWidget = static_cast<RenderWidget*>(element()->renderer());
        if (renderWidget && renderWidget->widget())
            m_instance = frame->script()->createScriptInstanceForWidget(renderWidget->widget());
    }

    return m_instance;
}

void WebMediaPlayerProxy::load(const String& url)
{
    RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin();

    if (plugin)
        plugin->load(url, this);
}

void WebMediaPlayerProxy::initEngine()
{
    // Todo: add more supports soon.
}

HTMLMediaElement* WebMediaPlayerProxy::element()
{
    return static_cast<HTMLMediaElement*>(m_mediaPlayer->mediaPlayer()->mediaPlayerClient());
}

void WebMediaPlayerProxy::invokeMethod(const String& methodName)
{
    // Not used at the moment
}

PassRefPtr<Olympia::WebKit::WebPlugin> WebMediaPlayerProxy::getPlugin()
{
    if (!element()->document())
        return 0;

    Frame* frame = element()->document()->frame();

    if (!frame)
        return 0;

    FrameLoaderClientBlackBerry* client = static_cast<FrameLoaderClientBlackBerry*>(frame->loader()->client());
    ASSERT(client);
    return client->getCachedPlugin(reinterpret_cast<HTMLElement*>(element()));
}

void WebMediaPlayerProxy::play()
{
    if (RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin())
        plugin->play();
}

void WebMediaPlayerProxy::pause()
{
    if (RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin())
        plugin->pause();
}

float WebMediaPlayerProxy::time()
{
    if (RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin())
        return plugin->time(); 
    return 0;
}

void WebMediaPlayerProxy::seek(float time)
{
    if (RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin())
        plugin->seek(time);
}

void WebMediaPlayerProxy::setVolume(float volume)
{
    if (RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin())
        plugin->setVolume(volume);
}

float WebMediaPlayerProxy::volume()
{
    if (RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin())
        return plugin->volume();
    return 0;
}

void WebMediaPlayerProxy::setMuted(bool muted)
{
    if (RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin())
        plugin->setMuted(muted);
}

bool WebMediaPlayerProxy::muted()
{
    if (RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin())
        return plugin->muted();
    return false;
}

void WebMediaPlayerProxy::setSize(const IntSize& size)
{
    if (RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin())
        plugin->setSize(size);
}

bool WebMediaPlayerProxy::autoplay()
{
    if (RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin())
        return plugin->autoplay(); 
    return false;
}

void WebMediaPlayerProxy::setAutoplay(bool autoplay)
{
    if (RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin())
        plugin->setAutoplay(autoplay);
}

void WebMediaPlayerProxy::readyStateChanged(int state)
{
    m_mediaPlayer->setReadyState(state);
}

void WebMediaPlayerProxy::volumeChanged(float volume)
{
    m_mediaPlayer->volumeChanged(volume);
}

void WebMediaPlayerProxy::durationChanged(float duration)
{
    m_mediaPlayer->durationChanged(duration);
}

float WebMediaPlayerProxy::duration()
{
    if (RefPtr<Olympia::WebKit::WebPlugin> plugin = getPlugin())
        return plugin->duration(); 
    return 0;
}

}

