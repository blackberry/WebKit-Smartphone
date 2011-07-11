/* 
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef _STDINT_H_RIM
#define _STDINT_H_RIM

#ifdef __ARMCC_VERSION
#define STDINT_HF <SYSTEMINCLUDE/stdint.h>
#include STDINT_HF

#if __ARMCC_VERSION > 310000
// RVCT 3.1 and later define these types
#define HAVE_UINT8_T
#define HAVE_INT8_T
#define HAVE_UINT16_T
#define HAVE_INT16_T
#define HAVE_UINT32_T
#define HAVE_INT32_T
#define HAVE_UINT64_T
#define HAVE_INT64_T
#define HAVE_UINTPTR_T
#define HAVE_INTPTR_T
#endif
#endif

#ifdef _MSC_VER
#ifndef HAVE_UINT8_T
#define HAVE_UINT8_T 1
typedef unsigned char uint8_t;
#endif

#ifndef HAVE_INT8_T
#define HAVE_INT8_T 1
typedef signed char int8_t;
#endif

#ifndef HAVE_UINT16_T
#define HAVE_UINT16_T 1
typedef unsigned short uint16_t;
#endif

#ifndef HAVE_INT16_T
#define HAVE_INT16_T 1
typedef signed short int16_t;
#endif

#ifndef HAVE_UINT32_T
#define HAVE_UINT32_T 1
typedef unsigned __int32 uint32_t;
#endif

#ifndef HAVE_INT32_T
#define HAVE_INT32_T 1
typedef signed __int32 int32_t;
#endif

#ifndef HAVE_UINT64_T
#define HAVE_UINT64_T 1
    typedef unsigned __int64 uint64_t;
#endif

#ifndef HAVE_INT64_T
#define HAVE_INT64_T 1
    typedef signed __int64 int64_t;
#endif

#endif

#endif
