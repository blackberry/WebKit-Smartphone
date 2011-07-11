/*
 * Copyright (C) 2009-2010 Research In Motion Limited. http://www.rim.com/
 */

#ifndef BlackBerryPlatformWorker_h
#define BlackBerryPlatformWorker_h

namespace BlackBerry {
namespace Platform {

    bool willStartWorker();
    void didStartWorker();
    void didCancelStartingWorker();
    void increaseWorkerCount();
    void decreaseWorkerCount();
} // Platform
} // Olympia

#endif // BlackBerryPlatformWorker_h
