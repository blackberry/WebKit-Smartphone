/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SurfaceOpenVG_h
#define SurfaceOpenVG_h

#include "IntSize.h"

#if PLATFORM(EGL)
#include <egl.h>
#endif

#include <openvg.h>
#include <wtf/Noncopyable.h>
#include <wtf/Vector.h>

namespace WebCore {

#if PLATFORM(EGL)
class EGLDisplayOpenVG;
#endif
class PainterOpenVG;
class IntSize;

/**
 * SurfaceOpenVG provides the functionality of surfaces and contexts that are
 * underlying the OpenVG implementation. In the vast majority of cases, that
 * underlying technology is EGL, but OpenVG doesn't depend on EGL per se.
 * Wrapping surface/context functionality into a separate class avoids lots
 * of #ifdefs and should make it easy to add different surface/context
 * implementations than EGL.
 */
class SurfaceOpenVG : public Noncopyable {
public:
    enum MakeCurrentMode {
        ApplyPainterStateOnSurfaceSwitch,
        DontApplyPainterState,
        DontSaveOrApplyPainterState
    };
    enum CachedPathDescriptor {
        CachedLinePath = 0, /// A line from (0,0) to (1,0).
        CachedRectPath,     /// A rectangle from (0,0) to (0,1), (1,1), (1,0), and back.
        CachedPathCount     /// Enum value count, internal use only.
    };

    /** Note that these VGPaint objects are shared among different surfaces,
     * as only a single surface can be painted on at a time. The active
     * PainterOpenVG object will change these paints to reflect its current
     * fill and stroke paint state. Also note that the paints are probably
     * assigned to the current context, so if they're changed, they'll be
     * applied to the stroke even without a prior vgSetPaint() call. */
    enum CachedPaintDescriptor {
        CachedStrokePaint = 0,
        CachedFillPaint,
        CachedPaintCount       /// Enum value count, internal use only.
    };

    static SurfaceOpenVG* currentSurface();

#if PLATFORM(EGL)
    friend class EGLDisplayOpenVG;

    /**
     * Create a new EGL pbuffer surface with the specified size and config on
     * the given display. If config is not specified, the display's default
     * pbuffer config is used.
     *
     * This constructor will trigger an assertion if creation of the surface
     * fails, unless you pledge to manually process the error code by passing
     * a non-zero pointer as errorCode parameter. The error code returned by
     * eglGetError() will be written to that variable.
     */
    SurfaceOpenVG(const IntSize& size, const EGLDisplay& display, EGLConfig* config = 0, EGLint* errorCode = 0);

    /**
     * Create a new EGL pbuffer surface that will be bound to the given
     * client buffer (read: VGImage), with the specified config on the
     * given display. If config is not specified, the display's default
     * pbuffer config is used.
     *
     * After the surface is created, you will only be able to access the
     * client buffer image if the surface is not current. The recommended way
     * to ensure this is to call surface->sharedSurface()->makeCurrent() if you
     * simply want to access the image's pixel contents, or if you intend to
     * draw the image directly, making the draw target surface current.
     *
     * This constructor will trigger an assertion if creation of the surface
     * fails, unless you pledge to manually process the error code by passing
     * a non-zero pointer as errorCode parameter. The error code returned by
     * eglGetError() will be written to that variable.
     */
    SurfaceOpenVG(EGLClientBuffer buffer, EGLenum bufferType,
        const EGLDisplay& display, EGLConfig* config = 0, EGLint* errorCode = 0);

    /**
     * Create a new EGL window surface with the specified native window handle
     * and config on the given display. If config is not specified, the
     * display's default window config is used.
     */
    SurfaceOpenVG(EGLNativeWindowType window, const EGLDisplay& display, EGLConfig* config = 0);

    EGLDisplay eglDisplay() const { return m_eglDisplay; }
    EGLSurface eglSurface() const { return m_eglSurface; }
    EGLContext eglContext() const { return m_eglContext; }
#endif

    ~SurfaceOpenVG();

    /**
     * If a surface is invalid (could not be created), all method calls will
     * crash horribly.
     */
    bool isValid() const;

    int width() const;
    int height() const;

    SurfaceOpenVG* sharedSurface() const;

    /**
     * Make the associated GL/EGL context the current one, so that subsequent
     * OpenVG commands apply to it.
     */
    void makeCurrent(MakeCurrentMode mode = ApplyPainterStateOnSurfaceSwitch);

    /**
     * Make a surface/context combination current that is "compatible"
     * (i.e. can access its shared resources) to the given one. If no
     * surface/context is current, the given one is made current.
     *
     * This method is meant to avoid context changes if they're not
     * necessary, particularly tailored for the case where something
     * compatible to the shared surface is requested while actual painting
     * happens on another surface.
     */
    void makeCompatibleCurrent();

    /**
     * Empty the OpenVG pipeline and make sure all the performed paint
     * operations show up on the surface as actual drawn pixels.
     */
    void flush();

    void setActivePainter(PainterOpenVG*);
    PainterOpenVG* activePainter();

    /**
     * Returns the handle for one of the standard paths from the
     * SurfaceOpenVG::CachedPathDescriptor enum. When this method is called
     * for the first time for the given path descriptor, the VGPath will be
     * newly created as resource of the shared context/surface.
     */
    VGPath cachedPath(CachedPathDescriptor);

    /**
     * Returns the handle for one of the persistent paints from the
     * SurfaceOpenVG::CachedPaintDescriptor enum. When this method is called
     * for the first time for the given path descriptor, the VGPaint will be
     * newly created as resource of the shared context/surface.
     */
    VGPaint cachedPaint(CachedPaintDescriptor);

private:
    PainterOpenVG* m_activePainter;
    static PainterOpenVG* s_currentPainter; // global currently active painter
    IntSize m_size; // cached so we don't need to fetch it from EGL

    Vector<VGPath>& cachedPaths();
    Vector<VGPaint>& cachedPaints();

#if PLATFORM(EGL)
    SurfaceOpenVG(); // for EGLDisplayOpenVG

    EGLDisplay m_eglDisplay;
    EGLSurface m_eglSurface;
    EGLContext m_eglContext;
#endif
};

}

#endif
