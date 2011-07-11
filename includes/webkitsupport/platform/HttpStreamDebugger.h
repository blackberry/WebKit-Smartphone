/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef Olympia_Platform_HttpStreamDebugger_h
#define Olympia_Platform_HttpStreamDebugger_h

#include "stdlib.h"

namespace BlackBerry {

    namespace Platform {

        class IStreamListener;

        class HttpStreamDebugger {
        public:
            virtual void notifyOpen(int playerId, int resourceId, int status, const char* message) = 0;
            virtual void notifyHeaderReceived(int playerId, int resourceId, const char* key, const char* value) = 0;
            virtual void notifyDataReceived(int playerId, int resourceId, const char *buf, size_t len) = 0;
            virtual void notifyDone(int playerId, int resourceId) = 0;

            virtual int resourceForUrl(const char* url) = 0;
            virtual bool shouldUseResource(int playerId, int resourceId) = 0;
            virtual void setListener(int playerId, int resourceId, IStreamListener* listener) = 0;

            virtual bool loadResource(int playerId, int resourceId) = 0;
            virtual bool cancel(int playerId, int resourceId) = 0;
        };

    } // namespace Platform

} // namespace Olympia

#endif // Olympia_Platform_HttpStreamDebugger_h
