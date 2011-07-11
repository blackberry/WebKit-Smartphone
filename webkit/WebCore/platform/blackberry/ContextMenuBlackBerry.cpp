/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "ContextMenu.h"

#include "ContextMenuItem.h"
#include "HitTestResult.h"

#include "NotImplemented.h"

namespace WebCore {

ContextMenu::ContextMenu(HitTestResult const& result)
    : m_hitTestResult(result)
{
    notImplemented();
}

ContextMenu::~ContextMenu()
{
    notImplemented();
}

void ContextMenu::appendItem(ContextMenuItem&)
{
    notImplemented();
}

void ContextMenu::setPlatformDescription(void*)
{
    notImplemented();
}

} // namespace WebCore
