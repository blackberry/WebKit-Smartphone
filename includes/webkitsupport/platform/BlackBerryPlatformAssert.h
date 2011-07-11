 /*
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */


#ifndef BlackBerryPlatformAssert_h
#define BlackBerryPlatformAssert_h

void webkitCrash(const char* file, int line, const char* function);

#define OLYMPIA_CRASH() webkitCrash(__FILE__, __LINE__, __FUNCTION__)

#ifndef NDEBUG
#define OLYMPIA_ASSERT(exp) do { if (!(exp)) OLYMPIA_CRASH(); } while(false)
#else
#define OLYMPIA_ASSERT(exp) (void(0))
#endif

#endif // BlackBerryPlatformAssert_h
