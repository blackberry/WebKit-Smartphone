/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "SearchPopupMenuBlackBerry.h"

#include "NotImplemented.h"
#include <wtf/text/AtomicString.h>

namespace WebCore {

SearchPopupMenuBlackBerry::SearchPopupMenuBlackBerry(PopupMenuClient* client)
    : m_popup(adoptRef(new PopupMenuBlackBerry(client)))
{
    notImplemented();
}

PopupMenu* SearchPopupMenuBlackBerry::popupMenu()
{
    return m_popup.get();
}

bool SearchPopupMenuBlackBerry::enabled()
{
    notImplemented();
    return false;
}

void SearchPopupMenuBlackBerry::saveRecentSearches(AtomicString const&, WTF::Vector<String, 0u> const&)
{
    // FIXME: remove 0 template parameter when this is implemented
    notImplemented();
}

void SearchPopupMenuBlackBerry::loadRecentSearches(AtomicString const&, WTF::Vector<String, 0u>&)
{
    // FIXME: remove 0 template parameter when this is implemented
    notImplemented();
}

} // namespace WebCore
