/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "Pasteboard.h"

#include "DocumentFragment.h"
#include "Frame.h"
#include "KURL.h"
#include "PlatformString.h"
#include "Range.h"

#include "NotImplemented.h"

namespace WebCore {

bool Pasteboard::canSmartReplace()
{
    notImplemented();
    return false;
}

void Pasteboard::clear()
{
    notImplemented();
}

void Pasteboard::writeImage(Node*, KURL const&, String const&)
{
    notImplemented();
}

void Pasteboard::writeSelection(Range*, bool, Frame*)
{
    notImplemented();
}

void Pasteboard::writeURL(KURL const&, String const&, Frame*)
{
    notImplemented();
}

void Pasteboard::writePlainText(const String& text)
{
    notImplemented();
}

Pasteboard* Pasteboard::generalPasteboard()
{
    notImplemented();
    return 0;
}

String Pasteboard::plainText(Frame*)
{
    notImplemented();
    return String();
}

WTF::PassRefPtr<DocumentFragment> Pasteboard::documentFragment(Frame*, WTF::PassRefPtr<Range>, bool, bool&)
{
    notImplemented();
    return 0;
}

} // namespace WebCore
