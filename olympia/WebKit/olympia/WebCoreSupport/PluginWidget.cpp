/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */
#include "config.h"
#include "PluginWidget.h"
#include "WebPlugin.h"

using namespace Olympia::WebKit;

namespace WebCore {

PluginWidget::~PluginWidget()
{
}

void PluginWidget::setFrameRect(const IntRect& rect)
{
    if (rect == frameRect())
        return;
    Widget::setFrameRect(rect); 
}

void PluginWidget::frameRectsChanged()
{
    if (!platformWidget())
        return;

    RefPtr<WebPlugin> plugin = static_cast<WebPlugin*>(platformWidget());

    if (parent()) {
        IntRect windowRect = convertToContainingWindow(IntRect(0, 0, width(), height()));
        plugin->setGeometry(windowRect);
    } else 
        plugin->setGeometry(frameRect());
}

void PluginWidget::show()
{
    Widget::show();
}
}
