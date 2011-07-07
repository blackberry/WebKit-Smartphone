/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef ChromeClientOlympia_h
#define ChromeClientOlympia_h

#include "ChromeClient.h"

namespace Olympia {
    namespace WebKit {
        class WebPage;
        class WebPagePrivate;
    }
}

namespace WebCore {

class ChromeClientOlympia : public ChromeClient {
public:
    ChromeClientOlympia(Olympia::WebKit::WebPage* page);

    virtual void chromeDestroyed();
    virtual void setWindowRect(const FloatRect&);
    virtual FloatRect windowRect();
    virtual void setPlatformScreenSize(const IntSize& platformScreenSize) { m_platformScreenSize = platformScreenSize; };
    virtual IntSize platformScreenSize() { return m_platformScreenSize; };
    virtual FloatRect pageRect();
    virtual float scaleFactor();
    virtual void focus();
    virtual void unfocus();
    virtual bool canTakeFocus(FocusDirection);
    virtual void takeFocus(FocusDirection);
    virtual void focusedNodeChanged(Node*);
    virtual Page* createWindow(Frame*, const FrameLoadRequest&, const WindowFeatures&);
    virtual void show();
    virtual bool canRunModal();
    virtual void runModal();
    virtual void setToolbarsVisible(bool);
    virtual bool toolbarsVisible();
    virtual void setStatusbarVisible(bool);
    virtual bool statusbarVisible();
    virtual void setScrollbarsVisible(bool);
    virtual bool scrollbarsVisible();
    virtual void setMenubarVisible(bool);
    virtual bool menubarVisible();
    virtual void setResizable(bool);
    virtual void addMessageToConsole(MessageSource, MessageType, MessageLevel, const String& message, unsigned int lineNumber, const String& sourceID);
    virtual bool canRunBeforeUnloadConfirmPanel();
    virtual bool runBeforeUnloadConfirmPanel(const String&, Frame*);
    virtual void closeWindowSoon();
    virtual void runJavaScriptAlert(Frame*, const String&);
    virtual bool runJavaScriptConfirm(Frame*, const String&);
    virtual bool runJavaScriptPrompt(Frame*, const String&, const String&, String&);
    virtual void setStatusbarText(const String&);
    virtual bool shouldInterruptJavaScript();
    virtual bool tabsToLinks() const;
    virtual IntRect windowResizerRect() const;
    virtual void invalidateContents(const IntRect&, bool);
    virtual void invalidateWindow(const IntRect&, bool);
    virtual void invalidateContentsAndWindow(const IntRect&, bool);
    virtual void invalidateContentsForSlowScroll(const IntSize&, const IntRect&, bool, const ScrollView*);
    virtual void scroll(const IntSize&, const IntRect&, const IntRect&);
    virtual IntPoint screenToWindow(const IntPoint&) const;
    virtual IntRect windowToScreen(const IntRect&) const;
    virtual void* platformWindow() const;
    virtual void contentsSizeChanged(Frame*, const IntSize&) const;
    virtual void scrollRectIntoView(const IntRect&, const ScrollView*) const;
    virtual void mouseDidMoveOverElement(const HitTestResult&, unsigned int);
    virtual void setToolTip(const String&, TextDirection);
    virtual void didReceiveViewportArguments(Frame*, const ViewportArguments&);
    virtual void print(Frame*);
    virtual void exceededDatabaseQuota(Frame*, const String&);
    virtual void requestGeolocationPermissionForFrame(Frame*, Geolocation*);
    virtual void cancelGeolocationPermissionRequestForFrame(Frame*, Geolocation*);
    virtual void runOpenPanel(Frame*, WTF::PassRefPtr<FileChooser>);
    virtual bool setCursor(PlatformCursorHandle);
    virtual void formStateDidChange(const Node*);
    virtual PassOwnPtr<HTMLParserQuirks> createHTMLParserQuirks();
    virtual void scrollbarsModeDidChange() const;
    virtual PlatformPageClient platformPageClient() const;

#if ENABLE(TOUCH_EVENTS)
    virtual void needTouchEvents(bool) { }
#endif

    virtual void reachedMaxAppCacheSize(int64_t spaceNeeded);

    virtual void layoutFinished(Frame*) const;
    virtual void overflowExceedsContentsSize(Frame*) const;
    virtual void didDiscoverFrameSet(Frame*) const;

    virtual int reflowWidth() const;

    virtual void chooseIconForFiles(const Vector<String>&, FileChooser*);

#if ENABLE(SVG)
    virtual void didSetSVGZoomAndPan(Frame*, unsigned short zoomAndPan);
#endif

private:
    Olympia::WebKit::WebPage* m_webPage;
    IntSize m_platformScreenSize; // The physical-device screen size in portrait orientation.
};

} // WebCore

#endif // ChromeClientOlympia_h
