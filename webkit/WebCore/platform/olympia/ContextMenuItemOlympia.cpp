/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "ContextMenuItem.h"

#include "ContextMenu.h"
#include "PlatformString.h"

#include "NotImplemented.h"

namespace WebCore {

ContextMenuItem::ContextMenuItem(ContextMenuItemType, ContextMenuAction, String const&, ContextMenu*)
{
    notImplemented();
}

ContextMenuItem::~ContextMenuItem()
{
    notImplemented();
}

void ContextMenuItem::setChecked(bool)
{
    notImplemented();
}

void ContextMenuItem::setEnabled(bool)
{
    notImplemented();
}

void ContextMenuItem::setSubMenu(ContextMenu*)
{
    notImplemented();
}

void ContextMenuItem::setTitle(String const&)
{
    notImplemented();
}

ContextMenuAction ContextMenuItem::action() const
{
    notImplemented();
    return ContextMenuItemTagNoAction;
}

ContextMenuItemType ContextMenuItem::type() const
{
    notImplemented();
    return ActionType;
}

String ContextMenuItem::title() const
{
    notImplemented();
    return String();
}

} // namespace WebCore
