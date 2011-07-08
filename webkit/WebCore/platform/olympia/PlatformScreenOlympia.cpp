/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "PlatformScreen.h"

#include "FloatRect.h"
#include "HostWindow.h"
#include "PageClientOlympia.h"
#include "ScrollView.h"
#include "Widget.h"

#include "NotImplemented.h"

namespace WebCore {

bool screenIsMonochrome(Widget*)
{
    notImplemented();
    return false;
}

int screenDepthPerComponent(Widget*)
{
    notImplemented();
    return 8;
}

int screenDepth(Widget*)
{
    notImplemented();
    return 24;
}

FloatRect screenAvailableRect(Widget* widget)
{
    ScrollView* view = widget->root() ? widget->root() : widget->parent();
    if (!view)
        return FloatRect(0, 0, 0, 0);

    PageClientOlympia* pageClient = view->hostWindow()->platformPageClient();

    return pageClient->screenAvailableRect();
}

FloatRect screenRect(Widget* widget)
{
    ScrollView* view = widget->root() ? widget->root() : widget->parent();
    if (!view)
        return FloatRect(0, 0, 0, 0);

    PageClientOlympia* pageClient = view->hostWindow()->platformPageClient();

    return pageClient->screenRect();
}

} // namespace WebCore
