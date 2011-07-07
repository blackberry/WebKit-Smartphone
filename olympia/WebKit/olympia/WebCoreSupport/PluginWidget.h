/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef PluginWidget_h
#define PluginWidget_h

#include "Widget.h"
#include "WebPlugin.h"

using namespace Olympia::WebKit;

namespace WebCore {

class PluginWidget: public Widget
{
public:
    PluginWidget(): Widget() {}
    ~PluginWidget();
    virtual void invalidateRect(const IntRect& r) {/* no-op */};
    void frameRectsChanged();
    void setFrameRect(const IntRect& rect);
    void show();
};

}

#endif
