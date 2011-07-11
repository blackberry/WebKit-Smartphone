/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "ClipboardBlackBerry.h"
#include "FileList.h"
#include "NotImplemented.h"

namespace WebCore {

ClipboardBlackBerry::ClipboardBlackBerry(ClipboardAccessPolicy policy)
: Clipboard(policy, false)
{
}


ClipboardBlackBerry::~ClipboardBlackBerry()
{
}


void ClipboardBlackBerry::clearData(const String& type)
{
    notImplemented();
}


void ClipboardBlackBerry::clearAllData()
{
    notImplemented();
}


String ClipboardBlackBerry::getData(const String& type, bool& success) const
{
    notImplemented();
    return String();
}


bool ClipboardBlackBerry::setData(const String& type, const String& data)
{
    notImplemented();
    return false;
}



HashSet<String> ClipboardBlackBerry::types() const
{
    notImplemented();
    return HashSet<String>();
}


PassRefPtr<FileList> ClipboardBlackBerry::files() const
{
    notImplemented();
    return 0;
}


DragImageRef ClipboardBlackBerry::createDragImage(IntPoint& dragLoc) const
{
    notImplemented();
    return 0;
}


void ClipboardBlackBerry::declareAndWriteDragImage(Element*, const KURL&, const String& title, Frame*)
{
    notImplemented();
}


void ClipboardBlackBerry::writeURL(const KURL&, const String&, Frame*)
{
    notImplemented();
}


void ClipboardBlackBerry::writeRange(Range*, Frame*)
{
    notImplemented();
}

void ClipboardBlackBerry::writePlainText(const String&)
{
    notImplemented();
}

bool ClipboardBlackBerry::hasData()
{
    notImplemented();
    return false;
}


void ClipboardBlackBerry::setDragImage(CachedImage*, const IntPoint&)
{
    notImplemented();
}


void ClipboardBlackBerry::setDragImageElement(Node*, const IntPoint&)
{
    notImplemented();
}

}
