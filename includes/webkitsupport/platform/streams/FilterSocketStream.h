/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef Olympia_Platform_FilterSocketStream_h
#define Olympia_Platform_FilterSocketStream_h

#include "ISocketStream.h"

// FIXME: This should be merged with FilterStream when ISocketStream and IStream are merged

namespace BlackBerry {

    namespace Platform {

        // put ISocketStreamListener first because its methods will be called more
        // often than ISocketStream's
        class FilterSocketStream : public ISocketStreamListener, public ISocketStream {
        public:

            FilterSocketStream();
            virtual ~FilterSocketStream();

            // takes ownership of stream
            // FIXME: use smart pointer to enforce this
            virtual void setWrappedStream(ISocketStream* stream);
            virtual ISocketStream* wrappedStream() const;

            // ISocketStreamListener
            virtual void notifyOpen();
            virtual void notifyClose();
            virtual void notifyFail(PlatformStatus status);
            virtual void notifyDataReceived(const char *buf, size_t len);
            virtual void notifyReadyToSendData();

            // ISocketStream
            virtual void open(const char* url, unsigned int urlLength);
            virtual void close();

            virtual int sendData(const char* buf, int len);

        protected:
            void clearWrappedStream();

            ISocketStream* m_wrappedStream;
        };

    } // namespace Platform

} // namespace Olympia

#endif // Olympia_Platform_FilterSocketStream_h
