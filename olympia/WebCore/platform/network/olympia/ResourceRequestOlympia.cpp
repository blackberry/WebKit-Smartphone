/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "ResourceRequest.h"
#include "streams/NetworkRequest.h"

using Olympia::Platform::NetworkRequest;

namespace WebCore {

unsigned initializeMaximumHTTPConnectionCountPerHost()
{
    return 40;
}

static inline NetworkRequest::CachePolicy platformCachePolicyForRequest(const ResourceRequest& request)
{
    switch (request.cachePolicy()) {
    case WebCore::UseProtocolCachePolicy:
        return NetworkRequest::UseProtocolCachePolicy;
    case WebCore::ReloadIgnoringCacheData:
        return NetworkRequest::ReloadIgnoringCacheData;
    case WebCore::ReturnCacheDataElseLoad:
        return NetworkRequest::ReturnCacheDataElseLoad;
    case WebCore::ReturnCacheDataDontLoad:
        return NetworkRequest::ReturnCacheDataDontLoad;
    default:
        ASSERT_NOT_REACHED();
        return NetworkRequest::UseProtocolCachePolicy;
    }
}

static inline NetworkRequest::TargetType platformTargetTypeForRequest(const ResourceRequest& request)
{
    if (request.isXMLHTTPRequest())
        return NetworkRequest::TargetIsXMLHTTPRequest;

    switch (request.targetType()) {
    case ResourceRequest::TargetIsMainFrame:
        return NetworkRequest::TargetIsMainFrame;
    case ResourceRequest::TargetIsSubframe:
        return NetworkRequest::TargetIsSubframe;
    case ResourceRequest::TargetIsSubresource:
        return NetworkRequest::TargetIsSubresource;
    case ResourceRequest::TargetIsStyleSheet:
        return NetworkRequest::TargetIsStyleSheet;
    case ResourceRequest::TargetIsScript:
        return NetworkRequest::TargetIsScript;
    case ResourceRequest::TargetIsFontResource:
        return NetworkRequest::TargetIsFontResource;
    case ResourceRequest::TargetIsImage:
        return NetworkRequest::TargetIsImage;
    case ResourceRequest::TargetIsObject:
        return NetworkRequest::TargetIsObject;
    case ResourceRequest::TargetIsMedia:
        return NetworkRequest::TargetIsMedia;
    case ResourceRequest::TargetIsWorker:
        return NetworkRequest::TargetIsWorker;
    case ResourceRequest::TargetIsSharedWorker:
        return NetworkRequest::TargetIsSharedWorker;
    default:
        ASSERT_NOT_REACHED();
        return NetworkRequest::TargetIsUnknown;
    }
}

void ResourceRequest::initializePlatformRequest(NetworkRequest& platformRequest, bool isInitial) const
{
    // if this is the initial load, skip the request body and headers
    if (isInitial)
        platformRequest.setRequestInitial(timeoutInterval());
    else {
        platformRequest.setRequestUrl(url().string().latin1().data(),
                httpMethod().latin1().data(),
                platformCachePolicyForRequest(*this),
                platformTargetTypeForRequest(*this),
                timeoutInterval());

        if (httpBody() && !httpBody()->isEmpty()) {
            const Vector<FormDataElement>& elements = httpBody()->elements();
            // use setData for simple forms because it is slightly more efficient
            if (elements.size() == 1 && elements[0].m_type == FormDataElement::data)
                platformRequest.setData(elements[0].m_data.data(), elements[0].m_data.size());
            else {
                for (int i = 0; i < elements.size(); ++i) {
                    const FormDataElement& element = elements[i];
                    if (element.m_type == FormDataElement::data)
                        platformRequest.addMultipartData(element.m_data.data(), element.m_data.size());
                    else if (element.m_type == FormDataElement::encodedFile)
                        platformRequest.addMultipartFilename(element.m_filename.characters(), element.m_filename.length());
                    else
                        ASSERT_NOT_REACHED(); // unknown type
                }
            }
        }

        for (HTTPHeaderMap::const_iterator it = httpHeaderFields().begin();
                it != httpHeaderFields().end();
                ++it)
        {
            WebCore::String key = it->first;
            WebCore::String value = it->second;
            if (!key.isEmpty() && !value.isEmpty())
                platformRequest.addHeader(key.latin1().data(), value.latin1().data());
        }
    }
}

} // namespace WebCore
