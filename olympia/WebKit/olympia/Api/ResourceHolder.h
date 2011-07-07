/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef ResourceHolder_h
#define ResourceHolder_h

#include "OlympiaGlobal.h"

namespace Olympia {
namespace WebKit {

class ResourceHolder {
public:
    virtual void ref() = 0;
    virtual void deref() = 0;
    virtual const char* url() const = 0;
    virtual const char* mimeType() const = 0;
    virtual const char* suggestedFileName() const = 0;
    virtual unsigned getMoreData(const unsigned char*& data, unsigned position = 0) const = 0;
    virtual unsigned size() const = 0;
};

}
}

#endif // ResourceHolder_h
