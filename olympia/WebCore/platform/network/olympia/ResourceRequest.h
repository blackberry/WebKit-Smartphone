/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef ResourceRequest_h
#define ResourceRequest_h

#include "ResourceRequestBase.h"

namespace Olympia {
namespace Platform {
class NetworkRequest;
}
}

namespace WebCore {

class ResourceRequest : public ResourceRequestBase {
public:
    ResourceRequest(const String& url)
        : ResourceRequestBase(KURL(ParsedURLString, url), UseProtocolCachePolicy)
        , m_isXMLHTTPRequest(false)
        , m_mustHandleInternally(false)
    {
    }

    ResourceRequest(const KURL& url)
        : ResourceRequestBase(url, UseProtocolCachePolicy)
        , m_isXMLHTTPRequest(false)
        , m_mustHandleInternally(false)
    {
    }

    ResourceRequest(const KURL& url, const String& referrer, ResourceRequestCachePolicy policy = UseProtocolCachePolicy)
        : ResourceRequestBase(url, policy)
        , m_isXMLHTTPRequest(false)
        , m_mustHandleInternally(false)
    {
        setHTTPReferrer(referrer);
    }

    ResourceRequest()
        : ResourceRequestBase(KURL(), UseProtocolCachePolicy)
        , m_isXMLHTTPRequest(false)
        , m_mustHandleInternally(false)
    {
    }

    void setToken(const String& token) { m_token = token; }
    String token() const { return m_token; }

    // FIXME: For RIM Bug #452. The BlackBerry application wants the anchor text for a clicked hyperlink so as to
    // make an informed decision as to whether to allow the navigation. We should move this functionality into a
    // UI/Policy delegate.
    void setAnchorText(const String& anchorText) { m_anchorText = anchorText; }
    String anchorText() const { return m_anchorText; }

    void setIsXMLHTTPRequest(bool isXMLHTTPRequest) { m_isXMLHTTPRequest = isXMLHTTPRequest; }
    bool isXMLHTTPRequest() const { return m_isXMLHTTPRequest; }

    // marks requests which must be handled by webkit, even if LinksHandledExternally is set
    void setMustHandleInternally(bool mustHandleInternally) { m_mustHandleInternally = mustHandleInternally; }
    bool mustHandleInternally() const { return m_mustHandleInternally; }

    void initializePlatformRequest(Olympia::Platform::NetworkRequest& request, bool isInitial = false) const;

    private:
        friend class ResourceRequestBase;

        String m_token;
        String m_anchorText;
        bool m_isXMLHTTPRequest;
        bool m_mustHandleInternally;

        void doUpdatePlatformRequest() {}
        void doUpdateResourceRequest() {}
};

} // namespace WebCore

#endif // ResourceRequest_h
