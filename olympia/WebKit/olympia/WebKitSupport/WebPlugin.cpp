/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */
#include "config.h"
#include "IntRect.h"
#include "WebPlugin.h"
#include "WebPage.h"
#include "WebPage_p.h"
#include "WebPageClient.h"

namespace Olympia {
namespace WebKit {

WebPlugin::WebPlugin(WebPage* webPage, int width, int height, WebCore::HTMLElement* node, const WebCore::String& url, bool isHtml5Video, bool hasRenderer)
    : m_webPage(webPage)
    , m_width(width)
    , m_height(height)
    , m_element(node)
    , m_url(url)
    , m_isLoaded(false)
    , m_shouldBeCancelled(false)
{
    m_playerID = reinterpret_cast<int>(this);
    if (!url.isEmpty())
        m_webPage->client()->loadPluginForMimetype(m_playerID, width, height, url.utf8().data(), isHtml5Video, hasRenderer);
}

WebPlugin::~WebPlugin()
{
    destroy();
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
    m_webPage->client()->playMedia(m_playerID);
}

void WebPlugin::pause()
{
    m_webPage->client()->pauseMedia(m_playerID);
}

}
}
