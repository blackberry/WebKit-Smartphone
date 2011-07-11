/*
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

#ifndef BlackBerryPlatformGraphics_h
#define BlackBerryPlatformGraphics_h

#include "BlackBerryPlatformMemory.h"
#include "BlackBerryPlatformPrimitives.h"

#if defined(USE_SKIA)
#include <skia/ext/platform_canvas.h>
#else
#include <egl.h>
#endif

namespace BlackBerry {
namespace Platform {
namespace Graphics {

enum BufferType {
    TileBuffer,
    TemporaryBuffer,
    TemporaryBufferWithAlpha,
};

struct Buffer;
#if defined(USE_SKIA)
typedef skia::PlatformCanvas Drawable;
#else
class Drawable {
public:
    Drawable(EGLSurface surface, EGLint surfaceType, bool isFlipTransformationRequired = false)
        : m_surface(surface)
        , m_surfaceType(surfaceType)
        , m_isFlipTransformationRequired(isFlipTransformationRequired)
    {
    }

    EGLSurface surface() const { return m_surface; }
    EGLint surfaceType() const { return m_surfaceType; }
    bool isFlipTransformationRequired() const { return m_isFlipTransformationRequired; }

private:
    EGLSurface m_surface;
    EGLint m_surfaceType;
    bool m_isFlipTransformationRequired;
};
#endif

void initialize();
void shutdown();

#if !defined(USE_SKIA)
EGLDisplay& display();
Drawable* drawingSurface();
#endif

Buffer* createBuffer(const BlackBerry::Platform::IntSize&, BufferType);
void destroyBuffer(Buffer*);
#if defined(USE_SKIA)
Drawable* lockBufferDrawable(Buffer*);
void releaseBufferDrawable(Buffer*);
#else
// Returns 0 for EGL implementation, you have to use temporary drawing surface
inline Drawable* lockBufferDrawable(Buffer*) { return 0; }
inline void releaseBufferDrawable(Buffer*) {}
#endif

void clearBufferWhite(Buffer*);
void clearBufferTransparent(Buffer*);
void clearBuffer(Buffer*, const BlackBerry::Platform::IntRect&, unsigned char red, unsigned char green, unsigned char blue);
#if !defined(USE_SKIA)
void blitToBuffer(Buffer* dst, const BlackBerry::Platform::IntPoint& dstPoint, Drawable* drawable, const BlackBerry::Platform::IntRect& srcRect);
#endif
void blitToBuffer(Buffer* dst, const BlackBerry::Platform::IntPoint& dstPoint, const Buffer* src, const BlackBerry::Platform::IntRect& srcRect);
void stretchBlitToBuffer(Buffer* dst, const BlackBerry::Platform::IntRect& dstRect, const Buffer* src, const BlackBerry::Platform::IntRect& srcRect);
void blendOntoBuffer(Buffer* dst, const BlackBerry::Platform::IntPoint& dstPoint, const Buffer* src, const BlackBerry::Platform::IntRect& srcRect, const unsigned char globalAlpha);
void flush();

} // namespace Graphics
} // namespace Platform
} // namespace Olympia

#endif // BlackBerryPlatformGraphics_h
