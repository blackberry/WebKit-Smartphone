/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef Olympia_Platform_DebugFilterStream_h
#define Olympia_Platform_DebugFilterStream_h

#include "HttpStreamDebugger.h"
#include "streams/FilterStream.h"

namespace BlackBerry {

    namespace Platform {

        class DebugFilterStream : public FilterStream {
        public:
            DebugFilterStream(int playerId, HttpStreamDebugger* debugger);
            virtual ~DebugFilterStream();

            // IStreamListener
            virtual void notifyOpen(int status, const char* message);
            virtual void notifyHeaderReceived(const char* key, const char* value);
            virtual void notifyDataReceived(const char *buf, size_t len);
            virtual void notifyDone();

            // IStream
            virtual void setRequest(const NetworkRequest&);
            virtual int open();
            virtual int cancel();


        private:
            int m_playerId;
            HttpStreamDebugger* m_debugger;
            int m_resourceIdInDebugger;
            bool m_useResourceFromDebugger;
        };

    } // namespace Olympia

} // namespace Platform

#endif // Olympia_Platform_DebugFilterStream_h
