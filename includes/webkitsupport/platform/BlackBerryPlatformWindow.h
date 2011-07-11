/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef BlackBerryPlatformWindow_h
#define BlackBerryPlatformWindow_h

#include "BlackBerryPlatformPrimitives.h"
#include <graphics/egl.h>
#include <wm/windowmanager.h>

#include <vector>

namespace blackberry {
namespace bbnsl {
        class Mutex;
    }
}

namespace BlackBerry {
namespace Platform {
namespace Graphics {

struct Buffer;

class Window {
public:
    Window(int width, int height);
    Window(WMWindow_t existingWindowHandle);
    ~Window();

    /*! \brief Locks the window using a mutex.
     * This method will lock the window so that different threads can access
     * unsafe methods like childWindows below.
     */
    void lockWindow();

    /*! \brief Unlocks the window using a mutex.
     * This method will unlock the window so that different threads can access
     * unsafe methods like childWindows below.
     */
    void unlockWindow();

    /*! \brief Returns a vector of the child windows for this window.
     * This method is not thread safe so it is necessary to lock/unlock the
     * window using the methods above before and immediately after calling this
     * method.
     */
    const std::vector<Window*> childWindows() const;

    void addChildWindow(void* platformHandle);
    void removeChildWindow(void* platformHandle);
    void addChildWindow(Window* window);
    void removeChildWindow(Window* window);

    enum TouchDelegate { Owner, Pass, Parent, JVM };

    void setTouchDelegate(TouchDelegate);
    void setTouchDelegate(int pid);

    enum SwapBehavior { PreserveBufferContents, DestroyBufferContents };

    void setSwapBehavior(SwapBehavior);

    WMWindow_t handle() const { return m_handle; }
    bool createSurface();
    bool createSurfaceOpenGL();
    void makeSurfaceCurrent(); // Only need to call this for OpenGL surface
    BlackBerry::Platform::IntSize surfaceSize() const { return size(); }

    WMWindow_t parentHandle() const;
    void setParent(WMWindow_t);
    void invalidate(int x, int y, int width, int height);

    BlackBerry::Platform::IntPoint position() const;
    void setPosition(const BlackBerry::Platform::IntPoint&);

    BlackBerry::Platform::IntSize size() const;
    int width() const { return size().width(); }
    int height() const { return size().height(); }
    void setSize(const BlackBerry::Platform::IntSize& sz);

    BlackBerry::Platform::IntPoint anchor() const;
    void setAnchor(const BlackBerry::Platform::IntPoint&);

    BlackBerry::Platform::IntRect bounds() const;
    void setBounds(const BlackBerry::Platform::IntRect&);

    void setGlobalAlpha(int alpha);

    bool visible() const;
    void setVisible(bool);

    void raiseToTop();
    void raiseAbove(const Window&);
    void lowerToBottom();
    void lowerBelow(const Window&);

    // New interfaces required by libwebview
    BlackBerry::Platform::IntSize viewportSize() const { return bounds().size(); }
    BlackBerry::Platform::IntPoint windowLocation() const { return position(); }
    void setWindowLocation(const BlackBerry::Platform::IntPoint& location) { setPosition(location); }
    BlackBerry::Platform::IntSize windowSize() const { return size(); }
    void setWindowSize(const BlackBerry::Platform::IntSize&) {} // FIXME: Implement using scale factor property
    BlackBerry::Platform::IntRect windowRect() const { return BlackBerry::Platform::IntRect(position(), size()); }

    void lockWindow() const {}
    void unlockWindow() const {}

    void post(const BlackBerry::Platform::IntRect& rect) { invalidate(rect.x(), rect.y(), rect.width(), rect.height()); }

    void* platformHandle() const { return m_handle; }

    Buffer* lockBuffer();
    void unlockBuffer();

    bool isValid() const { return m_surface != EGL_NO_SURFACE; }
    void flush() { /* no-op */ }

private:
    WMWindow_t m_handle;
    WMWindow_t m_parentHandle;
    EGLDisplay m_display;
    EGLSurface m_surface;
    EGLContext m_context;

    Buffer* m_buffer;
    std::vector<Window*> m_childWindows;
    mutable blackberry::bbnsl::Mutex* m_windowMutex;
};

struct WindowMutexLocker {
    WindowMutexLocker(Window* window)
        : m_window(window)
    {
        m_window->lockWindow();
    }

    ~WindowMutexLocker()
    {
        m_window->unlockWindow();
    }

    Window* m_window;
};

class BufferLocker {
public:
    BufferLocker(Window* window)
        : m_window(window)
    {
        m_buffer = m_window->lockBuffer();
    }

    ~BufferLocker()
    {
        m_window->unlockBuffer();
    }

    Buffer* buffer() const { return m_buffer; }

private:
    Window* m_window;
    Buffer* m_buffer;
};

} // namespace Graphics
} // namespace Platform
} // namespace Olympia

#endif // BlackBerryPlatformWindow_h
