/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#ifndef ResourceError_h
#define ResourceError_h

#include "ResourceErrorBase.h"

namespace WebCore {

class ResourceError : public ResourceErrorBase {
public:

    static const char* httpErrorDomain;
    static const char* platformErrorDomain;

    ResourceError()
    {
    }

    ResourceError(const String& domain, int errorCode, const String& failingURL, const String& localizedDescription)
        : ResourceErrorBase(domain, errorCode, failingURL, localizedDescription)
    {
    }
};

}

#endif // ResourceError_h_
