/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "SSLKeyGenerator.h"

#include "KURL.h"
#include "PlatformString.h"
#include "Vector.h"

#include "NotImplemented.h"

namespace WebCore {

void getSupportedKeySizes(WTF::Vector<String, 0u>&)
{
    // FIXME: remove 0 template param when this is implemented
    notImplemented();
}

String signedPublicKeyAndChallengeString(unsigned int, String const&, KURL const&)
{
    notImplemented();
    return String();
}

} // namespace WebCore
