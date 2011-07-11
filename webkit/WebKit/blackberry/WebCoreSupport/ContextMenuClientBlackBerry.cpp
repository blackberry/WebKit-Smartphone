/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "ContextMenuClientBlackBerry.h"

#include "NotImplemented.h"

namespace WebCore {

void ContextMenuClientBlackBerry::contextMenuDestroyed()
{
    delete this;
}

void* ContextMenuClientBlackBerry::getCustomMenuFromDefaultItems(ContextMenu*)
{
    notImplemented();
    return 0;
}

void ContextMenuClientBlackBerry::contextMenuItemSelected(ContextMenuItem*, const ContextMenu*)
{
    notImplemented();
}

void ContextMenuClientBlackBerry::downloadURL(const KURL&)
{
    notImplemented();
}

void ContextMenuClientBlackBerry::searchWithGoogle(const Frame*)
{
    notImplemented();
}

void ContextMenuClientBlackBerry::lookUpInDictionary(Frame*)
{
    notImplemented();
}

bool ContextMenuClientBlackBerry::isSpeaking()
{
    notImplemented();
    return false;
}

void ContextMenuClientBlackBerry::speak(const String&)
{
    notImplemented();
}

void ContextMenuClientBlackBerry::stopSpeaking()
{
    notImplemented();
}

} // namespace WebCore
