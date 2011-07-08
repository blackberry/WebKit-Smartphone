/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "Icon.h"

#include "GraphicsContext.h"
#include "IntRect.h"
#include "PassRefPtr.h"
#include "PlatformString.h"
#include "Vector.h"

#include "NotImplemented.h"

namespace WebCore {

Icon::~Icon()
{
    notImplemented();
}

void Icon::paint(GraphicsContext*, IntRect const&)
{
    notImplemented();
}

WTF::PassRefPtr<Icon> Icon::createIconForFiles(const Vector<String>& filenames)
{
    // FIXME: remove 0 template param when this is implemented
    notImplemented();
    return 0;
}

} // namespace WebCore
