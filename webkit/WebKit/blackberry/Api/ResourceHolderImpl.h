/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef ResourceHolderImpl_h
#define ResourceHolderImpl_h

#include "WebString.h"
#include "ResourceHolder.h"
#include "ScopePointer.h"
#include <string>
#include <vector>

namespace WebCore {
class CachedResource;
}

namespace Olympia {
namespace WebKit {

class ResourceHolderImpl: public ResourceHolder {
public:
    static ResourceHolderImpl* create(const WebCore::CachedResource& resource);
    
    virtual void ref();
    virtual void deref();
    virtual const char* url() const;
    virtual const char* mimeType() const;
    virtual const char* suggestedFileName() const;
    virtual unsigned getMoreData(const unsigned char*& data, unsigned position = 0) const;
    virtual unsigned size() const;
private:
    ResourceHolderImpl();

    unsigned m_refCount;
    std::string m_url;
    std::string m_mimeType;
    std::string m_suggestedFileName;
    // FIXME: Ideally we could share the SharedBuffer object in WebKit. Let's try that later.
    ScopeArray<unsigned char> m_buffer;
    unsigned m_bufferSize;
};

}
}

#endif // ResourceHolderImpl_h
