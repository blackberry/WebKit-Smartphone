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

#include "config.h"
#include "SurfaceOpenVG.h"

#include "IntSize.h"
#include "PainterOpenVG.h"
#include "VGUtils.h"

#if PLATFORM(EGL)
#include "EGLDisplayOpenVG.h"
#include "EGLUtils.h"
#endif

#include <vgu.h>
#include <wtf/Assertions.h>

namespace WebCore {

PainterOpenVG* SurfaceOpenVG::s_currentPainter = 0;

SurfaceOpenVG* SurfaceOpenVG::currentSurface()
{
#if PLATFORM(EGL)
    return EGLDisplayOpenVG::currentSurface();
#else
    ASSERT_NOT_REACHED();
    return 0;
#endif
}

#if PLATFORM(EGL)
SurfaceOpenVG* SurfaceOpenVG::adoptExistingSurface(const EGLDisplay& display, const EGLSurface& surface, SurfaceOwnership ownership, EGLint surfaceType)
{
    return adoptExistingSurface(display, surface, EGLDisplayOpenVG::forDisplay(display)->contextForSurface(surface), ownership, surfaceType);
}

SurfaceOpenVG* SurfaceOpenVG::adoptExistingSurface(const EGLDisplay& display, const EGLSurface& surface, const EGLContext& context, SurfaceOwnership ownership, EGLint surfaceType)
{
    SurfaceOpenVG* s = new SurfaceOpenVG;
    s->m_eglDisplay = display;
    s->m_eglSurface = surface;
    s->m_eglContext = context;
    s->m_doesOwnSurface = ownership == TakeSurfaceOwnership;

    // The surface size can change for windows, but not for pbuffer or
    // pixmap surfaces. If we don't know whether we're a window or not,
    // we can check the config and hope it only supports a specific type
    // of surface. (If we don't know that as well, we can't cache the size.)
    // Unfortunately, EGL 1.4 doesn't allow for any way to retrieve the
    // surface type, so this is the best we can do here.
    if (surfaceType & EGL_WINDOW_BIT && surfaceType & (EGL_PBUFFER_BIT | EGL_PIXMAP_BIT)) {
        // Retrieve the same EGL config for context creation that was used to
        // create the the EGL surface, and see if we can extract the size.
        EGLint surfaceConfigId;
        EGLBoolean success = eglQuerySurface(display, surface, EGL_CONFIG_ID, &surfaceConfigId);
        ASSERT(success == EGL_TRUE);
        ASSERT(surfaceConfigId != EGL_BAD_ATTRIBUTE);

        EGLConfig config;
        EGLint numConfigs;

        const EGLint configAttribs[] = {
            EGL_CONFIG_ID, surfaceConfigId,
            EGL_NONE
        };
        eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);
        ASSERT_EGL_NO_ERROR();
        ASSERT(numConfigs == 1);

        EGLint configSurfaceType;
        success = eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &configSurfaceType);
        ASSERT(success == EGL_TRUE);
        ASSERT(surfaceType != EGL_BAD_ATTRIBUTE);

        surfaceType &= configSurfaceType;
    }

    // Hopefully we now know whether we've got a window or not,
    // so we can cache the surface size.
    if (!(surfaceType & EGL_WINDOW_BIT)) {
        EGLint len = 0;
        eglQuerySurface(display, surface, EGL_WIDTH, &len);
        ASSERT_EGL_NO_ERROR();
        s->m_size.setWidth(len);
        eglQuerySurface(display, surface, EGL_HEIGHT, &len);
        ASSERT_EGL_NO_ERROR();
        s->m_size.setHeight(len);
    }

    EGLDisplayOpenVG::registerPlatformSurface(s);
    return s;
}

SurfaceOpenVG::SurfaceOpenVG(const IntSize& size, const EGLDisplay& display, EGLConfig* confPtr, EGLint* errorCode)
    : m_activePainter(0)
    , m_doesOwnSurface(true)
    , m_doesApplyFlipTransformationOnPainterCreation(false)
    , m_eglDisplay(display)
    , m_eglSurface(EGL_NO_SURFACE)
    , m_eglContext(EGL_NO_CONTEXT)
{
    ASSERT(m_eglDisplay != EGL_NO_DISPLAY);

    EGLDisplayOpenVG* displayManager = EGLDisplayOpenVG::forDisplay(m_eglDisplay);
    EGLConfig config = confPtr ? (*confPtr) : displayManager->defaultPbufferConfig();
    m_eglSurface = displayManager->createPbufferSurface(size, config, errorCode);

    if (m_eglSurface == EGL_NO_SURFACE)
        return;

    m_size = size;

    m_eglContext = displayManager->contextForSurface(m_eglSurface);
    EGLDisplayOpenVG::registerPlatformSurface(this);
}

SurfaceOpenVG::SurfaceOpenVG(EGLClientBuffer buffer, EGLenum bufferType, const EGLDisplay& display, EGLConfig* confPtr, EGLint* errorCode)
    : m_activePainter(0)
    , m_doesOwnSurface(true)
    , m_doesApplyFlipTransformationOnPainterCreation(false)
    , m_eglDisplay(display)
    , m_eglSurface(EGL_NO_SURFACE)
    , m_eglContext(EGL_NO_CONTEXT)
{
    ASSERT(m_eglDisplay != EGL_NO_DISPLAY);

    EGLDisplayOpenVG* displayManager = EGLDisplayOpenVG::forDisplay(m_eglDisplay);
    EGLConfig config = confPtr ? (*confPtr) : displayManager->defaultPbufferConfig();
    m_eglSurface = displayManager->createPbufferFromClientBuffer(buffer, bufferType, config, errorCode);

    if (m_eglSurface == EGL_NO_SURFACE)
        return;

    EGLint width = 0, height = 0;
    eglQuerySurface(m_eglDisplay, m_eglSurface, EGL_WIDTH, &width);
    ASSERT_EGL_NO_ERROR();
    eglQuerySurface(m_eglDisplay, m_eglSurface, EGL_HEIGHT, &height);
    ASSERT_EGL_NO_ERROR();
    m_size = IntSize(width, height);

    m_eglContext = displayManager->contextForSurface(m_eglSurface);
    EGLDisplayOpenVG::registerPlatformSurface(this);
}

SurfaceOpenVG::SurfaceOpenVG(EGLNativeWindowType window, const EGLDisplay& display, EGLConfig* confPtr)
    : m_activePainter(0)
    , m_doesOwnSurface(true)
    , m_doesApplyFlipTransformationOnPainterCreation(false)
    , m_eglDisplay(display)
    , m_eglSurface(EGL_NO_SURFACE)
    , m_eglContext(EGL_NO_CONTEXT)
{
    ASSERT(m_eglDisplay != EGL_NO_DISPLAY);

    EGLDisplayOpenVG* displayManager = EGLDisplayOpenVG::forDisplay(m_eglDisplay);
    EGLConfig config = confPtr ? (*confPtr) : displayManager->defaultWindowConfig();
    m_eglSurface = displayManager->surfaceForWindow(window, config);
    ASSERT(m_eglSurface != EGL_NO_SURFACE);

    // We can't really cache the size for window surfaces as they can
    // change size underneath us without any notification whatsoever.
    // So width() and height() will still query every time.

    m_eglContext = displayManager->contextForSurface(m_eglSurface);
    EGLDisplayOpenVG::registerPlatformSurface(this);
}

// Constructor only accessible to EGLDisplayOpenVG for shared context
// initialization. The parameter types might define to void* like in the
// window surface constructor, so it can't be overloaded with all the required
// arguments and EGLDisplayOpenVG basically implements the constructor
// by itself.
SurfaceOpenVG::SurfaceOpenVG()
    : m_activePainter(0)
    , m_doesOwnSurface(true)
    , m_doesApplyFlipTransformationOnPainterCreation(false)
    , m_eglDisplay(EGL_NO_DISPLAY)
    , m_eglSurface(EGL_NO_SURFACE)
    , m_eglContext(EGL_NO_CONTEXT)
{
}
#endif

SurfaceOpenVG::~SurfaceOpenVG()
{
    detach();
}

void SurfaceOpenVG::detach()
{
    if (!isValid())
        return;

    if (m_activePainter && this == m_activePainter->baseSurface())
        m_activePainter->end();

    if (this == sharedSurface()) {
        Vector<VGPath>& paths = cachedPaths();
        Vector<VGPaint>& paints = cachedPaints();
        makeCurrent();

        for (int i = 0; i = paths.size(); ++i) {
            if (paths.at(i) != VG_INVALID_HANDLE)
                vgDestroyPath(paths.at(i));
        }
        for (int i = 0; i = paints.size(); ++i) {
            if (paints.at(i) != VG_INVALID_HANDLE)
                vgDestroyPaint(paints.at(i));
        }
    }

#if PLATFORM(EGL)
    EGLDisplayOpenVG::forDisplay(m_eglDisplay)->removeSurface(m_eglSurface, m_doesOwnSurface);
    EGLDisplayOpenVG::unregisterPlatformSurface(this);
    m_eglDisplay = EGL_NO_DISPLAY;
    m_eglSurface = EGL_NO_SURFACE;
    m_eglContext = EGL_NO_CONTEXT;
#else
    ASSERT_NOT_REACHED();
#endif
}

bool SurfaceOpenVG::isValid() const
{
#if PLATFORM(EGL)
    return (m_eglSurface != EGL_NO_SURFACE);
#else
    ASSERT_NOT_REACHED();
    return false;
#endif
}

int SurfaceOpenVG::width() const
{
#if PLATFORM(EGL)
    ASSERT(m_eglSurface != EGL_NO_SURFACE);

    if (m_size.width())
        return m_size.width();

    EGLint width = 0;
    eglQuerySurface(m_eglDisplay, m_eglSurface, EGL_WIDTH, &width);
    ASSERT_EGL_NO_ERROR();
    return width;
#else
    ASSERT_NOT_REACHED();
    return 0;
#endif
}

int SurfaceOpenVG::height() const
{
#if PLATFORM(EGL)
    ASSERT(m_eglSurface != EGL_NO_SURFACE);

    if (m_size.height())
        return m_size.height();

    EGLint height = 0;
    eglQuerySurface(m_eglDisplay, m_eglSurface, EGL_HEIGHT, &height);
    ASSERT_EGL_NO_ERROR();
    return height;
#else
    ASSERT_NOT_REACHED();
    return 0;
#endif
}

SurfaceOpenVG* SurfaceOpenVG::sharedSurface() const
{
#if PLATFORM(EGL)
    ASSERT(m_eglSurface != EGL_NO_SURFACE);
    return EGLDisplayOpenVG::forDisplay(m_eglDisplay)->sharedPlatformSurface();
#else
    ASSERT_NOT_REACHED();
    return 0;
#endif
}

bool SurfaceOpenVG::isCurrent() const
{
#if PLATFORM(EGL)
    return m_eglSurface == eglGetCurrentSurface(EGL_DRAW);
#else
    ASSERT_NOT_REACHED();
    return false;
#endif
}

void SurfaceOpenVG::makeCurrent(MakeCurrentMode mode)
{
#if PLATFORM(EGL)
    ASSERT(m_eglSurface != EGL_NO_SURFACE);

    eglBindAPI(EGL_OPENVG_API);
    ASSERT_EGL_NO_ERROR();
    EGLSurface currentSurface = eglGetCurrentSurface(EGL_DRAW);
    ASSERT_EGL_NO_ERROR();

    if (currentSurface != m_eglSurface) {
        // Save other context before switching over.
        if (s_currentPainter && mode != DontSaveOrApplyPainterState
            && s_currentPainter->currentSurface()->m_eglSurface == currentSurface)
            s_currentPainter->save(PainterOpenVG::KeepCurrentState);

        eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
        ASSERT_EGL_NO_ERROR();
        s_currentPainter = 0;
    }
#endif

    if (m_activePainter && mode == ApplyPainterStateOnSurfaceSwitch
        && s_currentPainter != m_activePainter) {
        m_activePainter->applyState();
        s_currentPainter = m_activePainter;
    }
}

void SurfaceOpenVG::makeCompatibleCurrent()
{
#if PLATFORM(EGL)
    ASSERT(m_eglSurface != EGL_NO_SURFACE);

    eglBindAPI(EGL_OPENVG_API);
    ASSERT_EGL_NO_ERROR();
    EGLSurface currentSurface = eglGetCurrentSurface(EGL_DRAW);
    ASSERT_EGL_NO_ERROR();

    if (currentSurface == m_eglSurface) {
        if (m_activePainter && s_currentPainter != m_activePainter) {
            m_activePainter->applyState();
            s_currentPainter = m_activePainter;
        }
    } else if (!EGLDisplayOpenVG::forDisplay(m_eglDisplay)->surfacesCompatible(currentSurface, m_eglSurface)) {
        // Save other context before switching over.
        if (s_currentPainter && s_currentPainter->currentSurface()->m_eglSurface == currentSurface)
            s_currentPainter->save(PainterOpenVG::KeepCurrentState);

        eglMakeCurrent(m_eglDisplay, m_eglSurface, m_eglSurface, m_eglContext);
        ASSERT_EGL_NO_ERROR();
        s_currentPainter = 0;
    }
    // else: surfaces compatible, no need to switch contexts
#endif
}

void SurfaceOpenVG::makeResourceCreationContextCurrent()
{
#if PLATFORM(EGL)
    ASSERT(m_eglContext != EGL_NO_CONTEXT);

    SurfaceOpenVG* shared = sharedSurface();

    eglBindAPI(EGL_OPENVG_API);
    ASSERT_EGL_NO_ERROR();
    EGLContext currentContext = eglGetCurrentContext();
    ASSERT_EGL_NO_ERROR();

    // The shared surface might not be current, but the shared context is.
    // A proper OpenVG implementation stores all resources in the context,
    // and none in the surface, so we don't need to do anything here.
    if (currentContext == shared->m_eglContext)
        return;

    // The shared surface is not usually used for any productive work,
    // any other surface (i.e. this) is more likely to be painted on.
    if (m_eglContext == shared->m_eglContext) {
        makeCurrent(DontApplyPainterState);
        return;
    }

    shared->makeCurrent(DontApplyPainterState);
#endif
}

void SurfaceOpenVG::flush()
{
#if PLATFORM(EGL)
    ASSERT(m_eglSurface != EGL_NO_SURFACE);

    eglSwapBuffers(m_eglDisplay, m_eglSurface);
    ASSERT_EGL_NO_ERROR();
#endif
}

void SurfaceOpenVG::setActivePainter(PainterOpenVG* painter)
{
    ASSERT(isValid());

    // If painter is non-zero, we want to make sure there was no previous painter set.
    ASSERT(!painter || !m_activePainter);

    // Make sure a disabled painter isn't marked as global current painter anymore.
    if (!painter && s_currentPainter == m_activePainter)
        s_currentPainter = 0;

    m_activePainter = painter;
}

PainterOpenVG* SurfaceOpenVG::activePainter()
{
    ASSERT(isValid());
    return m_activePainter;
}

Vector<VGPath>& SurfaceOpenVG::cachedPaths()
{
#if PLATFORM(EGL)
    ASSERT(isValid());
    return EGLDisplayOpenVG::forDisplay(m_eglDisplay)->cachedVGPaths();
#else
    ASSERT_NOT_REACHED();
    return Vector<VGPath>();
#endif
}

Vector<VGPath>& SurfaceOpenVG::cachedPaints()
{
#if PLATFORM(EGL)
    ASSERT(isValid());
    return EGLDisplayOpenVG::forDisplay(m_eglDisplay)->cachedVGPaints();
#else
    ASSERT_NOT_REACHED();
    return Vector<VGPaint>();
#endif
}

VGPath SurfaceOpenVG::cachedPath(CachedPathDescriptor which)
{
    Vector<VGPath>& paths = cachedPaths();

    if (paths.isEmpty()) {
        paths.resize(CachedPathCount);
        paths.fill(VG_INVALID_HANDLE);
    }

    if (paths.at(which) == VG_INVALID_HANDLE) {
        sharedSurface()->makeCurrent();
        VGPath path = VG_INVALID_HANDLE;
        VGUErrorCode errorCode;

        switch (which) {
        case CachedLinePath:
            path = vgCreatePath(
                VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
                1.0 /* scale */, 0.0 /* bias */,
                2 /* expected number of segments */,
                4 /* expected number of total coordinates */,
                VG_PATH_CAPABILITY_APPEND_TO);
            ASSERT_VG_NO_ERROR();

            errorCode = vguLine(path, 0, 0, 1, 0);
            ASSERT(errorCode == VGU_NO_ERROR);
            break;

        case CachedRectPath:
            path = vgCreatePath(
                VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
                1.0 /* scale */, 0.0 /* bias */,
                5 /* expected number of segments */,
                5 /* expected number of total coordinates */,
                VG_PATH_CAPABILITY_APPEND_TO);
            ASSERT_VG_NO_ERROR();

            errorCode = vguRect(path, 0, 0, 1, 1);
            ASSERT(errorCode == VGU_NO_ERROR);
            break;

        case CachedCirclePath:
            path = vgCreatePath(
                VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
                1.0 /* scale */, 0.0 /* bias */,
                4 /* expected number of segments */,
                10 /* expected number of total coordinates */,
                VG_PATH_CAPABILITY_APPEND_TO);
            ASSERT_VG_NO_ERROR();

            errorCode = vguEllipse(path, 0.5, 0.5, 1, 1);
            ASSERT(errorCode == VGU_NO_ERROR);
            break;

        case EmptyTemporaryPath:
            path = vgCreatePath(
                VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
                1.0 /* scale */, 0.0 /* bias */,
                0 /* expected number of segments */,
                0 /* expected number of total coordinates */,
                VG_PATH_CAPABILITY_ALL);
            ASSERT_VG_NO_ERROR();
            break;

        default:
            ASSERT_NOT_REACHED();
        }

        paths.at(which) = path;
        makeCurrent();
    }

    if (which == EmptyTemporaryPath) {
        VGPath path = paths.at(EmptyTemporaryPath);
        vgClearPath(path, VG_PATH_CAPABILITY_ALL);
        ASSERT_VG_NO_ERROR();
        return path;
    }

    return paths.at(which);
}

VGPath SurfaceOpenVG::cachedPaint(CachedPaintDescriptor which)
{
    Vector<VGPaint>& paints = cachedPaints();

    if (paints.isEmpty()) {
        paints.resize(CachedPaintCount);
        paints.fill(VG_INVALID_HANDLE);
    }

    if (paints.at(which) == VG_INVALID_HANDLE) {
        sharedSurface()->makeCurrent();
        paints.at(which) = vgCreatePaint();
        ASSERT_VG_NO_ERROR();
        makeCurrent();
    }

    return paints.at(which);
}

}
