/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "SearchPopupMenu.h"

#include "AtomicString.h"
#include "PlatformString.h"
#include "PopupMenuClient.h"

#include "NotImplemented.h"

namespace WebCore {

SearchPopupMenu::SearchPopupMenu(PopupMenuClient* client)
    : PopupMenu(client)
{
    notImplemented();
}

bool SearchPopupMenu::enabled()
{
    notImplemented();
    return false;
}

void SearchPopupMenu::loadRecentSearches(AtomicString const&, WTF::Vector<String, 0u>&)
{
    // FIXME: remove 0 template parameter when this is implemented
    notImplemented();
}

void SearchPopupMenu::saveRecentSearches(AtomicString const&, WTF::Vector<String, 0u> const&)
{
    // FIXME: remove 0 template parameter when this is implemented
    notImplemented();
}

} // namespace WebCore
