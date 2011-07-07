/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef WebPageClient_h
#define WebPageClient_h

#include "OlympiaGlobal.h"

#include "OlympiaContext.h"
#include "OlympiaPlatformInputEvents.h"
#include "OlympiaPlatformMisc.h"
#include "OlympiaPlatformPrimitives.h"
#include "OlympiaString.h"
#include "WebDOMDocument.h"
#include "JavaScriptCore/JSValueRef.h"

#include <openvg.h>

template<typename T> class ScopeArray;
template<typename T> class SharedArray;

typedef void* WebFrame;

namespace Olympia {

    namespace Platform {
        class GeoTracker;
        class HttpStreamDebugger;
        class IntPoint;
        class IntRect;
        class NetworkRequest;
        class NetworkStreamFactory;
    }

    namespace WebKit {
        class WebPage;

        class OLYMPIA_EXPORT WebPageClient {
        public:
            enum WindowStyleFlag {
                FlagWindowHasMenuBar = 0x00000001,
                FlagWindowHasToolBar = 0x00000002,
                FlagWindowHasLocationBar = 0x00000004,
                FlagWindowHasStatusBar = 0x00000008,
                FlagWindowHasScrollBar = 0x00000010,
                FlagWindowIsResizable = 0x00000020,
                FlagWindowIsFullScreen = 0x00000040,
                FlagWindowIsDialog = 0x00000080,
                FlagWindowDefault = 0xFFFFFFFF,
            };

            virtual int getInstanceId() const = 0;

            virtual void notifyLoadStarted() = 0;
            virtual void notifyLoadCommitted(const unsigned short* originalUrl, unsigned int originalUrlLength, const unsigned short* finalUrl, unsigned int finalUrlLength, const unsigned short* networkToken, unsigned int networkTokenLength) = 0;
            virtual void notifyLoadFailedBeforeCommit(const unsigned short* originalUrl, unsigned int originalUrlLength, const unsigned short* finalUrl, unsigned int finalUrlLength, const unsigned short* networkToken, unsigned int networkTokenLength) = 0;
            virtual void notifyLoadToAnchor(const unsigned short* url, unsigned int urlLength, const unsigned short* networkToken, unsigned int networkTokenLength) = 0;
            virtual void notifyLoadProgress(int percentage) = 0;
            virtual void notifyLoadReadyToRender() = 0;
            virtual void notifyLoadFinished(int status) = 0;
            virtual void notifyClientRedirect(const unsigned short* originalUrl, unsigned int originalUrlLength, const unsigned short* finalUrl, unsigned int finalUrlLength) = 0;

            virtual void notifyDocumentCreatedForFrame(const WebFrame frame, const bool isMainFrame, const WebDOMDocument& document, const JSValueRef window) = 0;
            virtual void notifyFrameDetached(const WebFrame frame) = 0;

            virtual void notifyRunLayoutTestsFinished() = 0;

            virtual void addMessageToConsole(const unsigned short* message, unsigned messageLength, const unsigned short* source, unsigned sourceLength, unsigned lineNumber) = 0;
            virtual void runJavaScriptAlert(const unsigned short* message, unsigned messageLength) = 0;
            virtual bool runJavaScriptConfirm(const unsigned short* message, unsigned messageLength) = 0;
            virtual bool runJavaScriptPrompt(const unsigned short* message, unsigned messageLength, const unsigned short* defaultValue, unsigned defaultValueLength, String& result) = 0;
            virtual bool shouldInterruptJavaScript() = 0;

            virtual void javascriptSourceParsed(const unsigned short* url, unsigned urlLength, const unsigned short* script, unsigned scriptLength) = 0;
            virtual void javascriptParsingFailed(const unsigned short* url, unsigned urlLength, const unsigned short* error, unsigned errorLength, int lineNumber) = 0;
            virtual void javascriptPaused(const unsigned short* stack, unsigned stackLength) = 0;
            virtual void javascriptContinued() = 0;

            // All of these methods use transformed coordinates
            virtual void contentsSizeChanged(const int width, const int height) const = 0;
            virtual void scrollChanged(const int scrollX, const int scrollY) const = 0;
            virtual void zoomChanged(const bool isMinZoomed, const bool isMaxZoomed, const bool isAtInitialZoom, const double newZoom) const = 0;

            virtual void setPageTitle(const unsigned short* title, unsigned titleLength) = 0;
            virtual void blitToCanvas(const int dstX, const int dstY, const int dstWidth, const int dstHeight,
                                      const int srcX, const int srcY, const int srcWidth, const int srcHeight) = 0;
            virtual void blitToCanvas(const int dstX, const int dstY, const unsigned char* srcImage, const int srcStride,
                                      const int srcX, const int srcY, const int srcWidth, const int srcHeight) = 0;
            virtual void blitFromBufferToBuffer(unsigned char* dst, const int dstStride, const int dstX, const int dstY,
                                                const unsigned char* src, const int srcStride, const int srcX, const int srcY, const int srcWidth, const int srcHeight) = 0;
            virtual void blendOntoCanvas(const int dstX, const int dstY, const unsigned char* srcImage, const int srcStride,
                                         const unsigned char* srcAlphaImage, const int srcAlphaStride, const char globalAlpha,
                                         const int srcX, const int srcY, const int srcWidth, const int srcHeight) = 0;
            virtual void invalidateWindow(const int dstX, const int dstY, const int dstWidth, const int dstHeight) = 0;
            virtual void lockCanvas() = 0;
            virtual void unlockCanvas() = 0;
            virtual void clearCanvas() = 0;

            virtual void inputFocusGained(unsigned int frameId, unsigned int inputFieldId, Olympia::Platform::OlympiaInputType type, unsigned int characterCount, unsigned int selectionStart, unsigned int selectionEnd) = 0;
            virtual void inputFocusLost(unsigned int frameId, unsigned int inputFieldId) = 0;
            virtual void inputTextChanged(unsigned int frameId, unsigned int inputFieldId, const unsigned short* text, unsigned int textLength, unsigned int selectionStart, unsigned int selectionEnd) = 0;
            virtual void inputTextForElement(unsigned int requestedFrameId, unsigned int requestedElementId, unsigned int offset, int length, int selectionStart, int selectionEnd, const unsigned short* text, unsigned int textLength) = 0;
            virtual void inputFrameCleared(unsigned int frameId) = 0;
            virtual void inputSelectionChanged(unsigned int frameId, unsigned int inputFieldId, unsigned int selectionStart, unsigned int selectionEnd) = 0;
            virtual void inputSetNavigationMode(bool) = 0;

            virtual void selectionBounds(Olympia::Platform::IntRect start, Olympia::Platform::IntRect end) = 0;

            virtual void cursorChanged(Olympia::Platform::CursorType cursorType, const char* url, const int x, const int y) = 0;

            virtual void requestGeolocationPermission(Olympia::Platform::GeoTracker*) = 0;
            virtual void cancelGeolocationPermission(Olympia::Platform::GeoTracker*) = 0;
            virtual Olympia::Platform::NetworkStreamFactory* networkStreamFactory() = 0;
            virtual Olympia::Platform::HttpStreamDebugger* httpStreamDebugger() = 0;
            virtual bool runMessageLoopForJavaScript() = 0;

            virtual void handleStringPattern(const unsigned short* pattern, unsigned int length) = 0;
            virtual void handleExternalLink(const Platform::NetworkRequest&, const unsigned short* context, unsigned int contextLength) = 0;

            virtual void resetBackForwardList(unsigned int listSize, unsigned int currentIndex) = 0;

            virtual void openPopupList(bool multiple, const int size, const ScopeArray<String>& labels, bool* enableds, const int* itemType, bool* selecteds) = 0;
            virtual void popupListClosed(const int size, bool* selecteds) = 0;
            virtual void popupListClosed(const int index) = 0;
            virtual void openDateTimePopup(const int type, const String& value, const String& min, const String& max, const double step) = 0; 
            virtual void setDateTimeInput(const Olympia::WebKit::String& value) = 0;

            virtual void contextChanged(const Context&) = 0;

            virtual bool chooseFilenames(bool allowMultiple, const String& acceptTypes, const SharedArray<String>& initialFiles, unsigned int initialFileSize, SharedArray<String>& chosenFiles, unsigned int& chosenFileSize) = 0;

            virtual void loadPluginForMimetype(int, int width, int height, const char* url, bool isHtml5Video, bool hasRenderer) = 0;
            virtual void notifyPluginRectChanged(int, Platform::IntRect rectChanged) = 0;
            virtual void destroyPlugin(int) = 0;
            virtual void playMedia(int) = 0;
            virtual void pauseMedia(int) = 0;

            virtual WebPage* createWindow(int x, int y, int width, int height, unsigned flags) = 0;
            virtual void scheduleCloseWindow() = 0;

            // Database interface.
            virtual unsigned long long databaseQuota(const unsigned short* origin, unsigned originLength,
                                                     const unsigned short* databaseName, unsigned databaseNameLength,
                                                     unsigned long long totalUsage, unsigned long long originUsage,
                                                     unsigned long long estimatedSize) = 0;

            virtual void setIconForUrl(const char* originalPageUrl, const char* finalPageUrl, const char* iconUrl) = 0;

            virtual Olympia::WebKit::String getErrorPage(int errorCode, const char* errorMessage, const char* url) = 0;

            virtual void willDeferLoading() = 0;
            virtual void didResumeLoading() = 0;

            // headers is a list of alternating key and value
            virtual void setMetaHeaders(const ScopeArray<String>& headers, unsigned int headersSize) = 0;
        };
    }
}

#endif // WebPageClient_h
