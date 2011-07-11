/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "Version.h"

#include "WebKitVersion.h" // Note: auto generated at build time

// Make sure we are not treated as big endian nor middle endian.
#if PLATFORM(ARM) && (PLATFORM(BIG_ENDIAN) || PLATFORM(MIDDLE_ENDIAN))
#error Our platform is little endian, but either PLATFORM(BIG_ENDIAN) or PLATFORM(MIDDLE_ENDIAN) is defined!
#endif

namespace Olympia {
namespace WebKit {

int webKitMajorVersion()
{
    return WEBKIT_MAJOR_VERSION;
}

int webKitMinorVersion()
{
    return WEBKIT_MINOR_VERSION;
}

}
}
