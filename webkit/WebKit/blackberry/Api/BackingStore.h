/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef BackingStore_h
#define BackingStore_h

#include "BlackBerryGlobal.h"

#include <egl.h>

namespace WebCore {
    class ChromeClientBlackBerry;
    class FrameLoaderClientBlackBerry;
}

namespace Olympia {
    namespace WebKit {

        class WebPage;
        class WebPagePrivate;
        class BackingStorePrivate;

        class OLYMPIA_EXPORT BackingStore {
        public:
            BackingStore(WebPage* webPage);
            virtual ~BackingStore();

            void createSurface();

            EGLDisplay display() const;

            void repaint(int x, int y, int width, int height,
                         bool contentChanged, bool immediate, bool repaintContentOnly);

            bool hasIdleJobs() const;
            void renderOnIdle();

        private:
            friend class Olympia::WebKit::WebPage;
            friend class Olympia::WebKit::WebPagePrivate; // FIXME: Hack like the rest
            friend class WebCore::ChromeClientBlackBerry;
            friend class WebCore::FrameLoaderClientBlackBerry;
            BackingStorePrivate *d;
        };
    }
}

#endif // BackingStore_h
