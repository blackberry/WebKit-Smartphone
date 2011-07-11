/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef Olympia_Platform_FilterStream_h
#define Olympia_Platform_FilterStream_h

#include "IStream.h"

namespace BlackBerry {

    namespace Platform {

        // put IStreamListener first because its methods will be called more
        // often than IStream's
        class FilterStream : public IStreamListener, public IStream {
        public:

            FilterStream();
            virtual ~FilterStream();

            // takes ownership of stream
            // FIXME: use smart pointer to enforce this
            virtual void setWrappedStream(IStream* stream);
            virtual IStream* wrappedStream() const;

            // IStreamListener
            virtual void notifyOpen(int status, const char* message);
            virtual void notifyWMLOverride();
            virtual void notifyHeaderReceived(const char* key, const char* value);
            virtual void notifyDataReceived(const char *buf, size_t len);
            virtual void notifyDone();

            // IStream
            virtual void setRequest(const NetworkRequest&);
            virtual int open();
            virtual int cancel();

        protected:
            void clearWrappedStream();

            IStream* m_wrappedStream;
        };

    } // namespace Olympia

} // namespace Platform

#endif // Olympia_Platform_FilterStream_h
