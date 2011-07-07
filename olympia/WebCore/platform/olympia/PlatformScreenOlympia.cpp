/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "PlatformScreen.h"

#include "FloatRect.h"
#include "Widget.h"

#include "NotImplemented.h"

//FIXME: layering violation!
#include "WebSettings.h"

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

FloatRect screenAvailableRect(Widget*)
{
    //FIXME: layering violation!
    return FloatRect(0, 0, Olympia::WebKit::WebSettings::screenWidth(), Olympia::WebKit::WebSettings::screenHeight());
}

FloatRect screenRect(Widget*)
{
    //FIXME: layering violation!
    return FloatRect(0, 0, Olympia::WebKit::WebSettings::screenWidth(), Olympia::WebKit::WebSettings::screenHeight());
}

} // namespace WebCore
