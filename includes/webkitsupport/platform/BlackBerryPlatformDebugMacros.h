/*
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

#ifndef BlackBerryPlatformDebugMacros_h
#define BlackBerryPlatformDebugMacros_h

#include "BlackBerryPlatformMisc.h"

#ifdef _MSC_VER
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#define notImplemented() do { \
    BlackBerry::Platform::log(BlackBerry::Platform::LogLevelWarn, \
    "%s:%d UNIMPLEMENTED: %s", \
    __FILE__, __LINE__, __PRETTY_FUNCTION__); \
    } while (0)

#define functionEntry() do { \
    BlackBerry::Platform::log(BlackBerry::Platform::LogLevelInfo, \
    "Olympia: %s:%d: ENTRY: %s", \
    __FILE__, __LINE__, __FUNCTION__); \
    } while (0)

#define functionExit() do { \
    BlackBerry::Platform::log(BlackBerry::Platform::LogLevelInfo, \
    "Olympia: %s:%d: EXIT: %s", \
    __FILE__, __LINE__, __FUNCTION__); \
    } while (0)

#endif // BlackBerryPlatformDebugMacros_h
