/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef PopupMenuOlympia_h
#define PopupMenuOlympia_h

#include "config.h"
#include "PopupMenu.h"
#include "PopupMenuClient.h"

namespace WebCore {


class PopupMenuBlackBerry : public PopupMenu {
public:
    PopupMenuBlackBerry(PopupMenuClient*);
    ~PopupMenuBlackBerry();

    virtual void show(const IntRect&, FrameView*, int index);
    virtual void hide();
    virtual void updateFromElement();
    virtual void disconnectClient();

private:
    PopupMenuClient* client() const { return m_popupClient; }

    PopupMenuClient* m_popupClient;
};

} // namespace WebCore

#endif
