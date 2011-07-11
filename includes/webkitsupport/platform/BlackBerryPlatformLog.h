/*
 * Copyright (C) 2009-2010 Research In Motion Limited. http://www.rim.com/
 */

#ifndef BlackBerryPlatformLog_h
#define BlackBerryPlatformLog_h

#include <stdarg.h>

namespace BlackBerry {
    namespace Platform {

        enum DebuggerMessageType {
            MessageDebug = 0,
            MessageStatus,
        };

        enum MessageLogLevel {
            LogLevelCritical = 0,
            LogLevelWarn,
            LogLevelInfo
        };

        // This logs events to wBugDisp.
        void logV(MessageLogLevel severity, const char* format, va_list);

        // This logs events to wBugDisp.
        void log(MessageLogLevel severity, const char* format, ...);

        // This logs events to on-device log file through Java EventLogger.
        void logEvent(MessageLogLevel level, const char* format, ...);

        unsigned int debugSetting();

    } // Platform

} // Olympia

#endif // BlackBerryPlatformLog_h
