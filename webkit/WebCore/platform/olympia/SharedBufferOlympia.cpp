/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "SharedBuffer.h"

#include "PassRefPtr.h"
#include "PlatformString.h"

#include "NotImplemented.h"

namespace WebCore {

WTF::PassRefPtr<SharedBuffer> SharedBuffer::createWithContentsOfFile(String const&)
{
    notImplemented();
    return 0;
}

} // namespace WebCore
