/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef OutOfMemoryHandler_h
#define OutOfMemoryHandler_h

#include "OutOfMemoryHandlerInterface.h"

namespace Olympia {
namespace WebKit {
    class OutOfMemoryHandler : public Olympia::Platform::OutOfMemoryHandlerInterface {
    public:
        virtual unsigned handleOutOfMemory(unsigned size);
        virtual void shrinkMemoryUsage();
    };
    
    void initializeOutOfMemoryHandler();        
}
}
#endif // OutOfMemoryHandlerWrapper_h
