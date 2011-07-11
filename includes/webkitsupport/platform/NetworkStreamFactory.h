/*
 * Copyright (C) Research In Motion Limited 2009-2010. http://www.rim.com/
 */

#ifndef Olympia_Platform_NetworkStreamFactory_h
#define Olympia_Platform_NetworkStreamFactory_h

namespace BlackBerry {

    namespace Platform {

        class IStream;
        class NetworkRequest;

        class NetworkStreamFactory {
        public:

            virtual IStream* createNetworkStream(const NetworkRequest&, int playerId) = 0;
        };

    } // namespace Platform

} // namespace Olympia

#endif // Olympia_Platform_NetworkStreamFactory_h
