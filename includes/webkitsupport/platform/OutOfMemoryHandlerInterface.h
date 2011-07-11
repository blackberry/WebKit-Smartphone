/*
* Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
*/

#ifndef OutOfMemoryHandlerInterface_h
#define OutOfMemoryHandlerInterface_h


#ifndef BRIDGE_IMPORT
#ifdef _MSC_VER
#define BRIDGE_IMPORT __declspec(dllimport)
#else
#define BRIDGE_IMPORT
#endif
#endif

namespace BlackBerry {
namespace Platform {

enum OOMHandledResult {
    OOMHandledRetry = 1,
    OOMHandledFail = 0,
};

struct OutOfMemoryHandlerInterface {
    virtual unsigned handleOutOfMemory(unsigned size) = 0;
    virtual void shrinkMemoryUsage() = 0;
};

void setOOMHandler(OutOfMemoryHandlerInterface* instance);
OOMHandledResult handleOOM(unsigned size);
void reportLowMemory();
void lowMemoryConfirmedByUser(bool keepReporting);
void shrinkMemoryUsageIfScheduled();

} // Platform
} // Olympia

#endif // OlympiaPlatformOutOfMemoryHandler_h
