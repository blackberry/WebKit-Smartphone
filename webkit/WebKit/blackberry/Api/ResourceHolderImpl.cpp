/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "ResourceHolderImpl.h"

#include "CachedResource.h"
#include "CString.h"
#include "PlatformString.h"
#include "SharedBuffer.h"
#include "wtf/RefPtr.h"

namespace Olympia {
namespace WebKit {

ResourceHolderImpl* ResourceHolderImpl::create(const WebCore::CachedResource& cachedResource)
{
    ResourceHolderImpl* instance = new ResourceHolderImpl();
    instance->m_url = cachedResource.url().latin1().data();
    instance->m_mimeType = cachedResource.response().mimeType().latin1().data();
    instance->m_suggestedFileName = cachedResource.response().suggestedFilename().latin1().data();
 
    if (!cachedResource.isLoaded() || !cachedResource.data())
        return instance;

    // FIXME: Ideally we should be able to shared the same object. But for safety, we
    // make another copy. Let's get back to this later.

    RefPtr<WebCore::SharedBuffer> sharedBuffer = cachedResource.data();

    instance->m_bufferSize = sharedBuffer->size();
    if (!instance->m_bufferSize)
        return instance;

    instance->m_buffer.reset(new unsigned char[instance->m_bufferSize]);
    unsigned char* dest = instance->m_buffer.get();
   
    const char* moreData;
    unsigned offset = 0;
    while (unsigned moreDataLength = sharedBuffer->getSomeData(moreData, offset)) {
        memcpy(dest, moreData, moreDataLength);
        dest += moreDataLength;
        offset += moreDataLength;
    }

    return instance;
}

ResourceHolderImpl::ResourceHolderImpl()
    : m_refCount(1)
    , m_bufferSize(0)
{
}

void ResourceHolderImpl::ref()
{
    ++m_refCount;
}

void ResourceHolderImpl::deref()
{
    if (!--m_refCount)
        delete this;
}

const char* ResourceHolderImpl::url() const
{
    return m_url.c_str();
}

const char* ResourceHolderImpl::mimeType() const
{
    return m_mimeType.c_str();
}

const char* ResourceHolderImpl::suggestedFileName() const
{
    return m_suggestedFileName.c_str();
}

unsigned ResourceHolderImpl::getMoreData(const unsigned char*& data, unsigned position /* = 0 */) const
{
    if (position >= m_bufferSize) {
        data = 0;
        return 0;
    }

    data = m_buffer.get() + position;
    return m_bufferSize - position;
}

unsigned ResourceHolderImpl::size() const
{
    return m_bufferSize;
}

} // namespace WebKit
} // namespace Olympia
