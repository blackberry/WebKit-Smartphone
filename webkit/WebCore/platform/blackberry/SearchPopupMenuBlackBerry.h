/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef SearchPopupMenuOlympia_h
#define SearchPopupMenuOlympia_h

#include "PopupMenuBlackBerry.h"
#include "SearchPopupMenu.h"

namespace WebCore {

class SearchPopupMenuBlackBerry : public SearchPopupMenu {
public:
    SearchPopupMenuBlackBerry(PopupMenuClient*);

    virtual PopupMenu* popupMenu();
    virtual void saveRecentSearches(const AtomicString& name, const Vector<String>& searchItems);
    virtual void loadRecentSearches(const AtomicString& name, Vector<String>& searchItems);
    virtual bool enabled();

private:
    RefPtr<PopupMenuBlackBerry> m_popup;
};

}

#endif // SearchPopupMenuOlympia_h
