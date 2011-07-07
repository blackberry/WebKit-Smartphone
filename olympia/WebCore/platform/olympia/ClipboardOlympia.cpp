/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "ClipboardOlympia.h"
#include "FileList.h"
#include "NotImplemented.h"

namespace WebCore {

ClipboardOlympia::ClipboardOlympia(ClipboardAccessPolicy policy)
: Clipboard(policy, false)
{
}


ClipboardOlympia::~ClipboardOlympia()
{
}


void ClipboardOlympia::clearData(const String& type)
{
    notImplemented();
}


void ClipboardOlympia::clearAllData()
{
    notImplemented();
}


String ClipboardOlympia::getData(const String& type, bool& success) const
{
    notImplemented();
    return String();
}


bool ClipboardOlympia::setData(const String& type, const String& data)
{
    notImplemented();
    return false;
}



HashSet<String> ClipboardOlympia::types() const
{
    notImplemented();
    return HashSet<String>();
}


PassRefPtr<FileList> ClipboardOlympia::files() const
{
    notImplemented();
    return 0;
}


DragImageRef ClipboardOlympia::createDragImage(IntPoint& dragLoc) const
{
    notImplemented();
    return 0;
}


void ClipboardOlympia::declareAndWriteDragImage(Element*, const KURL&, const String& title, Frame*)
{
    notImplemented();
}


void ClipboardOlympia::writeURL(const KURL&, const String&, Frame*)
{
    notImplemented();
}


void ClipboardOlympia::writeRange(Range*, Frame*)
{
    notImplemented();
}

void ClipboardOlympia::writePlainText(const String&)
{
    notImplemented();
}

bool ClipboardOlympia::hasData()
{
    notImplemented();
    return false;
}


void ClipboardOlympia::setDragImage(CachedImage*, const IntPoint&)
{
    notImplemented();
}


void ClipboardOlympia::setDragImageElement(Node*, const IntPoint&)
{
    notImplemented();
}

}
