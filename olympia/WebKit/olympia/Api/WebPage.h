/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef WebPage_h
#define WebPage_h

#include "OlympiaContext.h"
#include "OlympiaGlobal.h"
#include "OlympiaPlatformInputEvents.h"
#include "OlympiaPlatformKeyboardEvent.h"
#include "OlympiaPlatformMisc.h"
#include "OlympiaPlatformPrimitives.h"
#include "OlympiaPlatformReplaceText.h"
#include "OlympiaString.h"
#include "SharedPointer.h"
#include "streams/NetworkRequest.h"

#include <egl.h>

struct OpaqueJSContext;
typedef const struct OpaqueJSContext* JSContextRef;

struct OpaqueJSValue;
typedef const struct OpaqueJSValue* JSValueRef;

namespace WebCore {
    class ChromeClientOlympia;
    class EditorClientOlympia;
    class Element;
    class Frame;
    class FrameLoaderClientOlympia;
    class JavaScriptDebuggerOlympia;
    class Node;
    class RenderObject;
}

class WebDOMDocument;
class WebDOMNode;

namespace Olympia {
    namespace WebKit {

        class BackingStore;
        class BackingStorePrivate;
        class DumpRenderTree;
        class RenderQueue;
        class ResourceHolder;
        class WebPageClient;
        class WebPageGroupLoadDeferrer;
        class WebPagePrivate;
        class WebPlugin;
        class WebSettings;

        enum MouseEventType { MouseEventMoved, MouseEventPressed, MouseEventReleased, MouseEventAborted };

        class OLYMPIA_EXPORT WebPage {
        public:
            WebPage(WebPageClient*, const String& pageGroupName, const Platform::IntRect&);
            ~WebPage();

            WebPageClient* client() const;

            void load(const char* url, const char* networkToken, bool isInitial = false);

            void loadExtended(const char* url, const char* networkToken, const char* method, Platform::NetworkRequest::CachePolicy cachePolicy = Platform::NetworkRequest::UseProtocolCachePolicy, const char* data = 0, size_t dataLength = 0, const char* const* headers = 0, size_t headersLength = 0);

            void stopLoading();

            typedef intptr_t BackForwardId;

            // returns false if there is no page for the given delta (eg.
            // attempt to go back with -1 when on the first page)
            bool goBackOrForward(int delta);
            void goToBackForwardEntry(BackForwardId);

            void reload();
            void reloadFromCache();

            WebSettings* settings() const;

            int width() const;
            int height() const;

            void setVisible(bool visible);
            bool isVisible() const;

            void setActualVisibleSize(int width, int height);

            void resetVirtualViewportOnCommitted(bool reset);
            void setVirtualViewportSize(int width, int height);

            // Used for default layout size unless overridden by web content or by other APIs
            void setDefaultLayoutSize(int width, int height);

            void setScreenRotated(const Platform::IntSize& screenSize, const Platform::IntSize& defaultLayoutSize, const Platform::IntRect& viewportRect);

            void setScreenOrientation(int orientation);

            // FIXME: setPlatformScreenSize, and setApplicationViewSize don't pertain to an actual web page.
            // Instead, these methods belong in somekind of window/view class.
            void setPlatformScreenSize(int width, int height);
            void setApplicationViewSize(int width, int height);

            void setFocused(bool focused);

            void mouseEvent(MouseEventType, const Platform::IntPoint& pos, const Platform::IntPoint& globalPos);

            bool touchEvent(Olympia::Platform::TouchEvent&);

            // Returns true if the key stroke was handled by webkit.
            bool keyEvent(Olympia::Platform::KeyboardEvent::Type type, const unsigned short character, bool shiftDown);
            void navigationMoveEvent(const unsigned short character, bool shiftDown, bool altDown);

            Olympia::WebKit::String selectedText() const;

            // scroll position returned is in transformed coordinates
            Platform::IntPoint scrollPosition() const;
            // scroll position provided should be in transformed coordinates
            void setScrollPosition(const Platform::IntPoint&);

            BackingStore* backingStore() const;

            bool zoomIn();
            bool zoomOut();
            bool zoomToFit();
            bool zoomToOneOne();
            void zoomToInitialScale();
            bool bitmapZoom(int x, int y, double scale, bool shouldZoomAboutPoint = true);
            bool blockZoom(int x, int y);
            bool isAtInitialZoom() const;
            bool isMaxZoomed() const;
            bool isMinZoomed() const;

            double initialScale() const;
            void setInitialScale(double);
            double minimumScale() const;
            void setMinimumScale(double);
            double maximumScale() const;
            void setMaximumScale(double);
            double currentZoomLevel() const; // FIXME - needs to be renamed currentScale()

            bool moveToNextField(Olympia::Platform::ScrollDirection, int desiredScrollAmount);
            Olympia::Platform::IntRect focusNodeRect();
            bool focusField(bool focus);
            bool linkToLinkOnClick();

            void runLayoutTests();

            /**
             * Finds and selects the next utf8 string that is a case sensitive
             * match in the web page. It will wrap the web page if it reaches
             * the end. An empty string will result in no match and no selection.
             *
             * Returns true if the string matched and false if not.
             */
            bool findNextString(const char*);

            /**
             * Finds and selects the next unicode string. This is a case
             * sensitive search that will wrap if it reaches the end. An empty
             * string will result in no match and no selection.
             *
             * Returns true if the string matched and false if not.
             */
            bool findNextUnicodeString(const unsigned short*);

            /* JavaScriptDebugger interface */
            bool enableScriptDebugger();
            bool disableScriptDebugger();

            JSContextRef scriptContext() const;
            JSValueRef windowObject() const;

            /* Media Plugin interface */
            void cancelLoadingPlugin(int id);
            void removePluginsFromList();
            void cleanPluginListFromFrame(WebCore::Frame* frame);

            WebDOMDocument document() const;

            void addBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber, const unsigned short* condition, unsigned conditionLength);
            void updateBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber, const unsigned short* condition, unsigned conditionLength);
            void removeBreakpoint(const unsigned short* url, unsigned urlLength, unsigned lineNumber);

            bool pauseOnExceptions();
            void setPauseOnExceptions(bool pause);

            void pauseInDebugger();
            void resumeDebugger();

            void stepOverStatementInDebugger();
            void stepIntoStatementInDebugger();
            void stepOutOfFunctionInDebugger();

            void requestElementText(int requestedFrameId, int requestedElementId, int offset, int length);
            void selectionChanged();
            void setCaretPosition(int requestedFrameId, int requestedElementId, int caretPosition);
            void setCaretHighlightStyle(Platform::CaretHighlightStyle);
            Olympia::Platform::IntRect rectForCaret(int index);
            Olympia::Platform::ReplaceTextErrorCode replaceText(const Olympia::Platform::ReplaceArguments&, const Olympia::Platform::AttributedText&);

            void setSelection(const Platform::IntPoint& startPoint, const Platform::IntPoint& endPoint);
            void selectAtPoint(const Platform::IntPoint&);
            Olympia::Platform::IntRect selectionStartCaretRect();
            Olympia::Platform::IntRect selectionEndCaretRect();
            void selectionCancelled();

            void popupListClosed(const int size, bool* selecteds);
            void popupListClosed(const int index);
            void setDateTimeInput(const Olympia::WebKit::String& value);
            void setColorInput(const Olympia::WebKit::String& value);

            Olympia::WebKit::Context getContext() const;

            struct BackForwardEntry {
                String url;
                String title;
                String networkToken;
                BackForwardId id;
            };
            void getBackForwardList(SharedArray<BackForwardEntry>& result, unsigned int& resultLength) const;
            void releaseBackForwardEntry(BackForwardId) const;

            ResourceHolder* getImageFromContext();

            void addVisitedLink(const unsigned short* url, unsigned int length);

            WebDOMNode nodeAtPoint(int x, int y);
            bool getNodeRect(const WebDOMNode&, Platform::IntRect& result);
            bool setNodeFocus(const WebDOMNode&, bool on);
            bool setNodeHovered(const WebDOMNode&, bool on);
            bool nodeHasHover(const WebDOMNode&);

            bool defersLoading() const;

            bool willFireTimer();

            void clearStorage();

        private:
            friend class Olympia::WebKit::BackingStorePrivate;
            friend class Olympia::WebKit::DumpRenderTree;
            friend class Olympia::WebKit::RenderQueue;
            friend class Olympia::WebKit::WebPageGroupLoadDeferrer;
            friend class Olympia::WebKit::WebPlugin;
            friend class WebCore::ChromeClientOlympia;
            friend class WebCore::EditorClientOlympia;
            friend class WebCore::FrameLoaderClientOlympia;
            friend class WebCore::JavaScriptDebuggerOlympia;
            WebPagePrivate* d;
        };
    }
}

#endif // WebPage_h
