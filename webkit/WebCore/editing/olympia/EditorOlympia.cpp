/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "Editor.h"

#include "Clipboard.h"
#include "ClipboardOlympia.h"
#include "ClipboardAccessPolicy.h"
#include "PassRefPtr.h"

#include "NotImplemented.h"

namespace WebCore {

WTF::PassRefPtr<Clipboard> Editor::newGeneralClipboard(ClipboardAccessPolicy policy)
{
    return ClipboardOlympia::create(policy);
}

} // namespace WebCore
