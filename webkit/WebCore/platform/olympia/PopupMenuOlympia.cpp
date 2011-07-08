/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "PopupMenu.h"

namespace WebCore {

PopupMenu::PopupMenu(PopupMenuClient* client)
    : m_popupClient(client)
{
}

PopupMenu::~PopupMenu()
{
    // no-op
}

void PopupMenu::show(const IntRect& r, FrameView* v, int index)
{
    // no-op
}

void PopupMenu::hide()
{
    m_popupClient->popupDidHide();
}

void PopupMenu::updateFromElement()
{
    // no-op
}

bool PopupMenu::itemWritingDirectionIsNatural()
{
    return false;
}

} // namespace WebCore
