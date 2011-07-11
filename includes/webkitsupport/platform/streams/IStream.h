/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef Olympia_Platform_IStream_h
#define Olympia_Platform_IStream_h

#include <limits.h>
#include <stddef.h>

namespace BlackBerry {

    namespace Platform {

        class NetworkRequest;

        class IStreamListener {
        public:
            virtual void notifyOpen(int status, const char* message) = 0;
            virtual void notifyWMLOverride() = 0;
            virtual void notifyHeaderReceived(const char* key, const char* value) = 0;
            virtual void notifyDataReceived(const char *buf, size_t len) = 0;
            virtual void notifyDone() = 0;
        };

        class IStream {
        public:

            IStream() : m_listener(0) { }
            virtual ~IStream() { }

            // positive status values are HTTP error codes, negatives are platform-specific
            enum PlatformStatus {
                StatusSuccess = 0,

                // an error occurred which the platform client has already notified the user about, so webkit should not display an error message
                StatusErrorAlreadyHandled = -1,

                // there was an error in the network stack, but no further information is available
                StatusNetworkError = -2,

                // the network stack did not know how to handle this request
                StatusUnhandledProtocolError = -3,

                // a file:// url did not exist
                StatusFileNotFoundError = -4,

                // there're too many redirections (like 10) for one url
                StatusTooManyRedirects = -5,
            };

            virtual void setRequest(const NetworkRequest&) = 0;
            virtual int open() = 0;
            virtual int cancel() = 0;

            virtual void setListener(IStreamListener* listener) {
                m_listener = listener;
            }
            virtual IStreamListener* listener() const {
                return m_listener;
            }

        protected:
            IStreamListener* m_listener;
        };

    } // namespace Olympia

} // namespace Platform

#endif // Olympia_Platform_IStream_h
