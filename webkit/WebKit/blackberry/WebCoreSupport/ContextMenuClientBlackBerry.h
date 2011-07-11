/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#ifndef ContextMenuClientOlympia_h
#define ContextMenuClientOlympia_h

#include "ContextMenuClient.h"

namespace WebCore {

class ContextMenuClientBlackBerry : public ContextMenuClient {
public:
    virtual void contextMenuDestroyed();
    virtual void* getCustomMenuFromDefaultItems(ContextMenu*);
    virtual void contextMenuItemSelected(ContextMenuItem*, const ContextMenu*);
    virtual void downloadURL(const KURL&);
    virtual void searchWithGoogle(const Frame*);
    virtual void lookUpInDictionary(Frame*);
    virtual bool isSpeaking();
    virtual void speak(const String&);
    virtual void stopSpeaking();
};

} // WebCore

#endif // ContextMenuClientOlympia_h
