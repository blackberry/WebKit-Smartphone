/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "ContextMenuClientOlympia.h"

#include "NotImplemented.h"

namespace WebCore {

void ContextMenuClientOlympia::contextMenuDestroyed()
{
    delete this;
}

void* ContextMenuClientOlympia::getCustomMenuFromDefaultItems(ContextMenu*)
{
    notImplemented();
    return 0;
}

void ContextMenuClientOlympia::contextMenuItemSelected(ContextMenuItem*, const ContextMenu*)
{
    notImplemented();
}

void ContextMenuClientOlympia::downloadURL(const KURL&)
{
    notImplemented();
}

void ContextMenuClientOlympia::searchWithGoogle(const Frame*)
{
    notImplemented();
}

void ContextMenuClientOlympia::lookUpInDictionary(Frame*)
{
    notImplemented();
}

bool ContextMenuClientOlympia::isSpeaking()
{
    notImplemented();
    return false;
}

void ContextMenuClientOlympia::speak(const String&)
{
    notImplemented();
}

void ContextMenuClientOlympia::stopSpeaking()
{
    notImplemented();
}

} // namespace WebCore
