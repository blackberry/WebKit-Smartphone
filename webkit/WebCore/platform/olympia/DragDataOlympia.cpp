/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "DragData.h"

#include "Clipboard.h"
#include "Color.h"
#include "Document.h"
#include "DocumentFragment.h"
#include "PlatformString.h"
#include "wtf/Vector.h"

#include "NotImplemented.h"

namespace WebCore {

bool DragData::canSmartReplace() const
{
    notImplemented();
    return false;
}

bool DragData::containsColor() const
{
    notImplemented();
    return false;
}

bool DragData::containsCompatibleContent() const
{
    notImplemented();
    return false;
}

bool DragData::containsFiles() const
{
    notImplemented();
    return false;
}

bool DragData::containsPlainText() const
{
    notImplemented();
    return false;
}

bool DragData::containsURL() const
{
    notImplemented();
    return false;
}

void DragData::asFilenames(WTF::Vector<String, 0u>& result) const
{
    // FIXME: remove explicit 0 size in result template once this is implemented
    notImplemented();
}

Color DragData::asColor() const
{
    notImplemented();
    return Color();
}

String DragData::asPlainText() const
{
    notImplemented();
    return String();
}

String DragData::asURL(String* title) const
{
    notImplemented();
    return String();
}

WTF::PassRefPtr<Clipboard> DragData::createClipboard(ClipboardAccessPolicy policy) const
{
    notImplemented();
    return 0;
}

WTF::PassRefPtr<DocumentFragment> DragData::asFragment(Document* doc) const
{
    notImplemented();
    return 0;
}


} // namespace WebCore
