/*
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

#ifndef BlackBerryPlatformMemory_h
#define BlackBerryPlatformMemory_h

#include <stddef.h>
#include <stdlib.h>
#ifdef __cplusplus
#include <new>
#else
#include <new.h>
#endif

#ifdef __cplusplus

namespace BlackBerry {
namespace Platform {

    void initializeMemoryManagement();
    unsigned pageSize();
    void* allocateAlignedMemory(unsigned align, unsigned size);
    void freeAlignedMemory(void*);
    void* reserveVirtualMemory(unsigned& totalBytes, bool executable);
    void releaseVirtualMemory(void*);
    void* commitVirtualMemory(void* address, unsigned totalBytes);
    void decommitVirtualMemory(void* address, unsigned totalBytes);
    void* allocateGraphicsMemory(unsigned size);
    void freeGraphicsMemory(void* address);
    bool isUnderMemoryPressure();
} // namespace Platform
} // namespace Olympia

void olympiaDebugInitialize();
void olympiaDebugDidAllocate(void*, unsigned);
void olympiaDebugDidFree(void*);

#endif // __cplusplus

#if defined(ENABLE_OLYMPIA_DEBUG_MEMORY) && ENABLE_OLYMPIA_DEBUG_MEMORY

// Must include this header before including openvg.h and egl.h
#define vgCreateMaskLayer olympiaVgCreateMaskLayer
#define vgCreatePath olympiaVgCreatePath
#define vgCreatePaint olympiaVgCreatePaint
#define vgCreateImage olympiaVgCreateImage
#define vgCreateFont olympiaVgCreateFont
#define vgDestroyMaskLayer olympiaVgDestroyMaskLayer
#define vgDestroyPath olympiaVgDestroyPath
#define vgDestroyPaint olympiaVgDestroyPaint
#define vgDestroyImage olympiaVgDestroyImage
#define vgDestroyFont olympiaVgDestroyFont
#define eglCreateWindowSurface olympiaEglCreateWindowSurface
#define eglCreateContext olympiaEglCreateContext
#define eglCreatePbufferSurface olympiaEglCreatePbufferSurface
#define eglCreatePixmapSurface olympiaEglCreatePixmapSurface
#define eglCreatePbufferFromClientBuffer olympiaEglCreatePbufferFromClientBuffer
#define eglDestroySurface olympiaEglDestroySurface
#define eglDestroyContext olympiaEglDestroyContext

#endif // defined(ENABLE_OLYMPIA_DEBUG_MEMORY) && ENABLE_OLYMPIA_DEBUG_MEMORY

#endif // BlackBerryPlatformMemory_h
