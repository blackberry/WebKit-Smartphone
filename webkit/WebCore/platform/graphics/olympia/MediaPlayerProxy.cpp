/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "MediaPlayerProxy.h"

#include "Frame.h"
#include "FrameLoaderClientOlympia.h"
#include "HTMLMediaElement.h"
#include "MediaPlayer.h"
#include "PlatformString.h"
#include "RenderWidget.h"
#include "WebPlugin.h"
#include "Widget.h"

namespace WebCore {

WebMediaPlayerProxy::WebMediaPlayerProxy(MediaPlayer* player)
    : m_mediaPlayer(player)
    , m_init(false)
    , m_plugin(0)
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
    if (!m_init)
        initEngine();
}

void WebMediaPlayerProxy::initEngine()
{
    // Todo: add more supports soon.
}

HTMLMediaElement* WebMediaPlayerProxy::element()
{
    return static_cast<HTMLMediaElement*>(m_mediaPlayer->mediaPlayerClient());
}

void WebMediaPlayerProxy::invokeMethod(const String& methodName)
{
    // Not used at the moment
}

void WebMediaPlayerProxy::getPlugin()
{
    Frame* frame = element()->document()->frame();

    ASSERT(frame);
    FrameLoaderClientOlympia* client = static_cast<FrameLoaderClientOlympia*>(frame->loader()->client());
    ASSERT(client);
    m_plugin = client->getCachedPlugin(reinterpret_cast<HTMLElement*>(element()));
}

void WebMediaPlayerProxy::play()
{
    if (!m_plugin)
        getPlugin();

    if (m_plugin)
        m_plugin->play();
}

void WebMediaPlayerProxy::pause()
{
    if (!m_plugin)
        getPlugin();

    if (m_plugin)
        m_plugin->pause();
}

}

