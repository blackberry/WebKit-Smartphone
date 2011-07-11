/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef Olympia_Platform_ISocketStream_h
#define Olympia_Platform_ISocketStream_h

#include <stddef.h>

// FIXME: This should be merged with IStream (whose interface needs to be
// made more general to handle both HTTP and raw streams)

namespace BlackBerry {

    namespace Platform {

        class ISocketStreamListener {
        public:
            enum PlatformStatus {
                StatusSuccess = 0,
                StatusErrorInvalidUrl = -1,
                StatusErrorConnectionFailed = -2,
                StatusErrorNotReady = -3,
                StatusErrorIO = -4,
            };

            virtual void notifyOpen() = 0;
            virtual void notifyClose() = 0;
            virtual void notifyFail(PlatformStatus status) = 0;
            virtual void notifyDataReceived(const char *buf, size_t len) = 0;
            virtual void notifyReadyToSendData() = 0;
        };

        class ISocketStream {
        public:

            ISocketStream() : m_listener(0) { }
            virtual ~ISocketStream() { }

            virtual void open(const char* url, unsigned int urlLength) = 0;
            virtual void close() = 0;

            virtual int sendData(const char* buf, int len) = 0;

            virtual void setListener(ISocketStreamListener* listener) {
                m_listener = listener;
            }
            virtual ISocketStreamListener* listener() const {
                return m_listener;
            }

        protected:
            ISocketStreamListener* m_listener;
        };

    } // namespace Olympia

} // namespace Platform

#endif // Olympia_Platform_ISocketStream_h
