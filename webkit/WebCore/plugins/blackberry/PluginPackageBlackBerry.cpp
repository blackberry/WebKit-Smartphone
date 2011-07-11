/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"

#include "PluginPackage.h"


#include "NotImplemented.h"

namespace WebCore {

bool PluginPackage::equal(PluginPackage const&, PluginPackage const&)
{
    notImplemented();
    return false;
}

bool PluginPackage::fetchInfo()
{
    notImplemented();
    return false;
}

int PluginPackage::compareFileVersion(unsigned int const&) const
{
    notImplemented();
    return 0;
}

unsigned int PluginPackage::hash() const
{
    notImplemented();
    return 0;
}

} // namespace WebCore