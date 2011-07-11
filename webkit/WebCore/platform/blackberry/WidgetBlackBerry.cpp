/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "Widget.h"

#include "Cursor.h"
#include "GraphicsContext.h"
#include "HostWindow.h"
#include "IntRect.h"
#include "ScrollView.h"

#include "NotImplemented.h"

namespace WebCore {

Widget::Widget(PlatformWidget widget)
{
    init(widget);
}

Widget::~Widget()
{
}

void Widget::hide()
{
    notImplemented();
}

void Widget::paint(GraphicsContext*, IntRect const&)
{
    notImplemented();
}

void Widget::setCursor(Cursor const& cursor)
{
    PlatformPageClient pageClient = root()->hostWindow()->platformPageClient();

    if (pageClient)
        pageClient->setCursor(cursor.impl());
}

void Widget::setFocus(bool)
{
    notImplemented();
}

void Widget::setFrameRect(const IntRect& rect)
{
    m_frame = rect;

    frameRectsChanged();
}

void Widget::setIsSelected(bool)
{
    notImplemented();
}

void Widget::show()
{
    notImplemented();
}

IntRect Widget::frameRect() const
{
    return m_frame;
}

} // namespace WebCore
