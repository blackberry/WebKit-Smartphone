/*
 * Copyright (C) Research In Motion, Limited 2009. All rights reserved.
 */

#ifndef BlackBerryPlatformIDN_h
#define BlackBerryPlatformIDN_h

namespace BlackBerry {
namespace Platform {

// Return the actual result length
unsigned idnToAscii(const unsigned short* input, unsigned inputLength, char* output, unsigned outputCapacity);

} // namespace Platform
} // namespace Olympia

#endif // BlackBerryPlatformIDN_h