/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "PopupMenuBlackBerry.h"

namespace WebCore {

PopupMenuBlackBerry::PopupMenuBlackBerry(PopupMenuClient* client)
    : m_popupClient(client)
{
}

PopupMenuBlackBerry::~PopupMenuBlackBerry()
{
    // no-op
}

void PopupMenuBlackBerry::show(const IntRect& r, FrameView* v, int index)
{
    // no-op
}

void PopupMenuBlackBerry::hide()
{
    m_popupClient->popupDidHide();
}

void PopupMenuBlackBerry::updateFromElement()
{
    // no-op
}

void PopupMenuBlackBerry::disconnectClient()
{
    m_popupClient = 0;
}


} // namespace WebCore
