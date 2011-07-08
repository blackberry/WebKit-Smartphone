/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef Olympia_Network_Manager_h
#define Olympia_Network_Manager_h

#include "HTTPHeaderMap.h"
#include "PlatformString.h"
#include "ResourceHandle.h"
#include "SharedBuffer.h"
#include "wtf/HashMap.h"
#include "wtf/RefPtr.h"
#include "wtf/Vector.h"

namespace WebCore {
    class Frame;
}

namespace Olympia {

    namespace Platform {
        class NetworkStreamFactory;
    }

    namespace WebKit {

        class NetworkJob;

        class NetworkManager
        {
        public:
            static NetworkManager *instance();

            void setInitialURL(const WebCore::KURL& url) { m_initialURL = url; }
            WebCore::KURL initialURL() { return m_initialURL; }

            bool startJob(int playerId, WTF::PassRefPtr<WebCore::ResourceHandle> job, const WebCore::Frame& frame, bool defersLoading);
            bool startJob(int playerId, WTF::PassRefPtr<WebCore::ResourceHandle> job, const WebCore::ResourceRequest& request, const WebCore::Frame& frame, bool defersLoading);

            bool stopJob(WTF::PassRefPtr<WebCore::ResourceHandle> job);

            void setDefersLoading(WTF::PassRefPtr<WebCore::ResourceHandle> job, bool defersLoading);

        private:
            friend class NetworkJob;

            NetworkJob* findJobForHandle(WTF::PassRefPtr<WebCore::ResourceHandle>);
            void deleteJob(NetworkJob* job);
            bool startJob(int playerId, const WebCore::String& pageGroupName, WTF::PassRefPtr<WebCore::ResourceHandle> job, const WebCore::ResourceRequest& request, Platform::NetworkStreamFactory* streamFactory, int deferLoadingCount = 0, int redirectCount = 0);

            WTF::Vector<NetworkJob*> m_jobs;
            WebCore::KURL m_initialURL;
        };

    } // System

} // Olympia

#endif // Olympia_Network_Manager_h
