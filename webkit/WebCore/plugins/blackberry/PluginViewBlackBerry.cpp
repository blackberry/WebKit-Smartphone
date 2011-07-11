/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"

#include "PluginView.h"

#include "GraphicsContext.h"
#include "IntRect.h"
#include "KeyboardEvent.h"
#include "MouseEvent.h"
#include "ScrollView.h"

#include "NotImplemented.h"

namespace WebCore {

PluginView::~PluginView()
{
    notImplemented();
}

void PluginView::setFocus()
{
    notImplemented();
}

void PluginView::show()
{
    notImplemented();
}

void PluginView::hide()
{
    notImplemented();
}

void PluginView::paint(GraphicsContext* context, const IntRect& rect)
{
    notImplemented();
}

void PluginView::setParent(ScrollView* parent)
{
    notImplemented();
}

void PluginView::setParentVisible(bool visible)
{
    notImplemented();
}

void PluginView::invalidateRect(const IntRect& rect)
{
    notImplemented();
}

NPError PluginView::handlePostReadFile(WTF::Vector<char, 0u>&, unsigned long, char const*)
{
    // FIXME: remove explicit 0 size param on template when this is implemented
    notImplemented();
    return 0;
}

void PluginView::handleKeyboardEvent(KeyboardEvent*)
{
    notImplemented();
}

void PluginView::handleMouseEvent(MouseEvent*)
{
    notImplemented();
}

void PluginView::updatePluginWidget()
{
    notImplemented();
}

} // namespace WebCore