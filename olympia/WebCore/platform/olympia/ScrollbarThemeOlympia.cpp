/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "ScrollbarTheme.h"

#include "NotImplemented.h"

namespace WebCore {

ScrollbarTheme* ScrollbarTheme::nativeTheme()
{
    notImplemented();
    static ScrollbarTheme theme;
    return &theme;
}

} // namespace WebCore
