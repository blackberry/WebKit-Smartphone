/*
 * Copyright (C) Research In Motion Limited 2010. http://www.rim.com/
 */

#ifndef Olympia_Platform_SocketStreamFactory_h
#define Olympia_Platform_SocketStreamFactory_h

namespace BlackBerry {

    namespace Platform {

        class ISocketStream;

        class SocketStreamFactory {
        public:

            virtual ISocketStream* createSocketStream(int playerId) = 0;
        };

    } // namespace Platform

} // namespace Olympia

#endif // Olympia_Platform_SocketStreamFactory_h
