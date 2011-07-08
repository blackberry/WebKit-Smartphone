/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef ResourceResponse_h
#define ResourceResponse_h

#include "ResourceResponseBase.h"

namespace WebCore {

class ResourceResponse : public ResourceResponseBase {
public:
    ResourceResponse()
        : m_isWML(false)
    {
    }

    ResourceResponse(const KURL& url, const String& mimeType, long long expectedLength, const String& textEncodingName, const String& filename)
        : ResourceResponseBase(url, mimeType, expectedLength, textEncodingName, filename)
        , m_isWML(false)
    {
    }

    void setIsWML(bool isWML) { m_isWML = isWML; }
    bool isWML() const { return m_isWML; }

private:
    bool m_isWML;
};

} // namespace WebCore

#endif // ResourceResponse_h
