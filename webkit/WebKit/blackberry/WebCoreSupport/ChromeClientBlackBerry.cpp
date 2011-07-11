/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "ChromeClientBlackBerry.h"

#include "BackingStore.h"
#include "BackingStore_p.h"
#include "CString.h"
#include "DatabaseTracker.h"
#include "PageGroupLoadDeferrer.h"

#include "Document.h"
#include "DumpRenderTreeBlackBerry.h"
#include "FileChooser.h"
#include "Frame.h"
#include "FrameLoadRequest.h"
#include "FrameLoader.h"
#include "Geolocation.h"
#include "GeolocationServiceBlackBerry.h"
#include "HitTestResult.h"
#include "InputHandler.h"
#include "KURL.h"
#include "NotImplemented.h"
#include "OlympiaPlatformMisc.h"
#include "OlympiaPlatformSettings.h"
#include "WebString.h"
#include "Page.h"
#include "PageGroup.h"
#include "PlatformString.h"
#include "PopupMenuBlackBerry.h"
#include "RenderView.h"
#include "SearchPopupMenuBlackBerry.h"
#include "SecurityOrigin.h"
#include "SharedPointer.h"
#include "SVGZoomAndPan.h"
#include "ViewportArguments.h"
#include "WebPage.h"
#include "WebPageClient.h"
#include "WebPage_p.h"
#include "WebSettings.h"
#include "WindowFeatures.h"

#define DEBUG_OVERFLOW_DETECTION 0

using Olympia::WebKit::WebString;

namespace WebCore {

static CString frameOrigin(Frame* frame) {
    DOMWindow* window = frame->domWindow();
    SecurityOrigin* origin = window->securityOrigin();
    CString latinOrigin = origin->toString().latin1();
    return latinOrigin;
}

ChromeClientBlackBerry::ChromeClientBlackBerry(Olympia::WebKit::WebPage* page)
    : m_webPage(page)
{
}

void ChromeClientBlackBerry::addMessageToConsole(MessageSource, MessageType, MessageLevel, const WTF::String& message, unsigned int lineNumber, const WTF::String& sourceID)
{
    if (m_webPage->d->m_dumpRenderTree)
        m_webPage->d->m_dumpRenderTree->addMessageToConsole(message, lineNumber, sourceID);
    m_webPage->client()->addMessageToConsole(message.characters(), message.length(), sourceID.characters(), sourceID.length(), lineNumber);
}

void ChromeClientBlackBerry::runJavaScriptAlert(Frame* frame, const WTF::String& message)
{
    if (m_webPage->d->m_dumpRenderTree) {
        m_webPage->d->m_dumpRenderTree->runJavaScriptAlert(message);
        return;
    }
    TimerBase::fireTimersInNestedEventLoop();

    CString latinOrigin = frameOrigin(frame);
    m_webPage->client()->runJavaScriptAlert(message.characters(), message.length(), latinOrigin.data(), latinOrigin.length());
}

bool ChromeClientBlackBerry::runJavaScriptConfirm(Frame* frame, const WTF::String& message)
{
    if (m_webPage->d->m_dumpRenderTree)
        return m_webPage->d->m_dumpRenderTree->runJavaScriptConfirm(message);
    TimerBase::fireTimersInNestedEventLoop();

    CString latinOrigin = frameOrigin(frame);
    return m_webPage->client()->runJavaScriptConfirm(message.characters(), message.length(), latinOrigin.data(), latinOrigin.length());

}

bool ChromeClientBlackBerry::runJavaScriptPrompt(Frame* frame, const WTF::String& message, const WTF::String& defaultValue, WTF::String& result)
{
    WebString clientResult;

    if (m_webPage->d->m_dumpRenderTree) {
        result = m_webPage->d->m_dumpRenderTree->runJavaScriptPrompt(message, defaultValue);
        return true;
    }

    TimerBase::fireTimersInNestedEventLoop();

    CString latinOrigin = frameOrigin(frame);
    if (m_webPage->client()->runJavaScriptPrompt(message.characters(), message.length(), defaultValue.characters(), defaultValue.length(), latinOrigin.data(), latinOrigin.length(), clientResult)) {
        result = clientResult;
        return true;
    }
    return false;
}

void ChromeClientBlackBerry::chromeDestroyed()
{
    delete this;
}

void ChromeClientBlackBerry::setWindowRect(const FloatRect&)
{
    return; // The window dimensions are fixed in the RIM port.
}

FloatRect ChromeClientBlackBerry::windowRect()
{
    IntSize applicationViewSize = Olympia::WebKit::WebSettings::pageGroupSettings(m_webPage->d->m_page->groupName())->applicationViewSize();
    return FloatRect(0, 0, applicationViewSize.width(), applicationViewSize.height());;
}

FloatRect ChromeClientBlackBerry::pageRect()
{
    notImplemented();
    return FloatRect();
}

float ChromeClientBlackBerry::scaleFactor()
{
    return 1.0f;
}

void ChromeClientBlackBerry::focus()
{
    notImplemented();
}

void ChromeClientBlackBerry::unfocus()
{
    notImplemented();
}

bool ChromeClientBlackBerry::canTakeFocus(FocusDirection)
{
    notImplemented();
    return false;
}

void ChromeClientBlackBerry::takeFocus(FocusDirection)
{
    notImplemented();
}

void ChromeClientBlackBerry::focusedNodeChanged(Node* node)
{
    notImplemented();
}

bool ChromeClientBlackBerry::shouldForceDocumentStyleSelectorUpdate()
{
    return !m_webPage->settings()->isJavaScriptEnabled() && !m_webPage->d->m_inputHandler->processingChange();
}

Page* ChromeClientBlackBerry::createWindow(Frame*, const FrameLoadRequest& request, const WindowFeatures& features)
{
    PageGroupLoadDeferrer deferrer(m_webPage->d->m_page, true);
    TimerBase::fireTimersInNestedEventLoop();

    int x = features.xSet ? features.x : 0;
    int y = features.ySet ? features.y : 0;
    int width = features.widthSet? features.width : -1;
    int height = features.heightSet ? features.height : -1;
    unsigned flags = 0;

    if (features.menuBarVisible)
        flags |= Olympia::WebKit::WebPageClient::FlagWindowHasMenuBar;
    if (features.statusBarVisible)
        flags |= Olympia::WebKit::WebPageClient::FlagWindowHasStatusBar;
    if (features.toolBarVisible)
        flags |= Olympia::WebKit::WebPageClient::FlagWindowHasToolBar;
    if (features.locationBarVisible)
        flags |= Olympia::WebKit::WebPageClient::FlagWindowHasLocationBar;
    if (features.scrollbarsVisible)
        flags |= Olympia::WebKit::WebPageClient::FlagWindowHasScrollBar;
    if (features.resizable)
        flags |= Olympia::WebKit::WebPageClient::FlagWindowIsResizable;
    if (features.fullscreen)
        flags |= Olympia::WebKit::WebPageClient::FlagWindowIsFullScreen;
    if (features.dialog)
        flags |= Olympia::WebKit::WebPageClient::FlagWindowIsDialog;

    Olympia::WebKit::WebPage* webPage = m_webPage->client()->createWindow(x, y, width, height, flags);
    if (!webPage)
        return 0;

    Page* newPage = webPage->d->m_page;
    if (!request.resourceRequest().isNull())
        newPage->mainFrame()->loader()->load(request.resourceRequest(), false);
    return newPage;
}

void ChromeClientBlackBerry::show()
{
    notImplemented();
}

bool ChromeClientBlackBerry::canRunModal()
{
    notImplemented();
    return false;
}

void ChromeClientBlackBerry::reachedApplicationCacheOriginQuota(SecurityOrigin*)
{
    notImplemented();
}

void ChromeClientBlackBerry::runModal()
{
    notImplemented();
}

bool ChromeClientBlackBerry::selectItemWritingDirectionIsNatural()
{
    return false;
}

PassRefPtr<PopupMenu> ChromeClientBlackBerry::createPopupMenu(PopupMenuClient* client) const
{
    return adoptRef(new PopupMenuBlackBerry(client));
}

PassRefPtr<SearchPopupMenu> ChromeClientBlackBerry::createSearchPopupMenu(PopupMenuClient* client) const
{
    return adoptRef(new SearchPopupMenuBlackBerry(client));
}


void ChromeClientBlackBerry::setToolbarsVisible(bool)
{
    notImplemented();
}

bool ChromeClientBlackBerry::toolbarsVisible()
{
    notImplemented();
    return false;
}

void ChromeClientBlackBerry::setStatusbarVisible(bool)
{
    notImplemented();
}

bool ChromeClientBlackBerry::statusbarVisible()
{
    notImplemented();
    return false;
}

void ChromeClientBlackBerry::setScrollbarsVisible(bool)
{
    notImplemented();
}

bool ChromeClientBlackBerry::scrollbarsVisible()
{
    notImplemented();
    return false;
}

void ChromeClientBlackBerry::setMenubarVisible(bool)
{
    notImplemented();
}

bool ChromeClientBlackBerry::menubarVisible()
{
    notImplemented();
    return false;
}

void ChromeClientBlackBerry::setResizable(bool)
{
    notImplemented();
}

bool ChromeClientBlackBerry::canRunBeforeUnloadConfirmPanel()
{
    notImplemented();
    return false;
}

bool ChromeClientBlackBerry::runBeforeUnloadConfirmPanel(const WTF::String& message, Frame*)
{
    if (m_webPage->d->m_dumpRenderTree)
        return m_webPage->d->m_dumpRenderTree->runBeforeUnloadConfirmPanel(message);

    notImplemented();
    return false;
}

void ChromeClientBlackBerry::closeWindowSoon()
{
    m_webPage->client()->scheduleCloseWindow();
}

void ChromeClientBlackBerry::setStatusbarText(const WTF::String& status)
{
    if (m_webPage->d->m_dumpRenderTree)
        return m_webPage->d->m_dumpRenderTree->setStatusText(status);
    notImplemented();
}

bool ChromeClientBlackBerry::tabsToLinks() const
{
    // FIXME: We should probably make it configurable
    return true;
}

IntRect ChromeClientBlackBerry::windowResizerRect() const
{
    notImplemented();
    return IntRect();
}

IntPoint ChromeClientBlackBerry::screenToWindow(const IntPoint&) const
{
    notImplemented();
    return IntPoint();
}

IntRect ChromeClientBlackBerry::windowToScreen(const IntRect&) const
{
    notImplemented();
    return IntRect();
}

void* ChromeClientBlackBerry::platformWindow() const
{
    notImplemented();
    return 0;
}

void ChromeClientBlackBerry::mouseDidMoveOverElement(const HitTestResult& result, unsigned int modifierFlags)
{
    notImplemented();
}

void ChromeClientBlackBerry::setToolTip(const WTF::String&, TextDirection)
{
    notImplemented();
}

void ChromeClientBlackBerry::didReceiveViewportArguments(Frame* frame, const ViewportArguments& arguments) const
{
    if (m_webPage->d->m_mainFrame != frame)
        return;

    m_webPage->d->didReceiveViewportArguments(arguments);
}

void ChromeClientBlackBerry::print(Frame*)
{
    notImplemented();
}

void ChromeClientBlackBerry::exceededDatabaseQuota(Frame* frame, const WTF::String& name)
{
#if ENABLE(DATABASE)
    Document* document = frame->document();
    if (!document)
        return;

    SecurityOrigin* origin = document->securityOrigin();
    if (m_webPage->d->m_dumpRenderTree) {
        m_webPage->d->m_dumpRenderTree->exceededDatabaseQuota(origin, name);
        return;
    }

    DatabaseTracker& tracker = DatabaseTracker::tracker(document->groupName());

    unsigned long long totalUsage = tracker.totalDatabaseUsage();
    unsigned long long originUsage = tracker.usageForOrigin(origin);

    DatabaseDetails details = tracker.detailsForNameAndOrigin(name, origin);
    unsigned long long estimatedSize = details.expectedUsage();
    const WTF::String& nameStr = details.displayName();

    WTF::String originStr = origin->databaseIdentifier();

    unsigned long long quota = m_webPage->client()->databaseQuota(originStr.characters(), originStr.length(),
        nameStr.characters(), nameStr.length(), totalUsage, originUsage, estimatedSize);

    tracker.setQuota(origin, quota);
#endif
}

void ChromeClientBlackBerry::requestGeolocationPermissionForFrame(Frame* frame, Geolocation* geolocation)
{
    if (!m_webPage->settings()->isGeolocationEnabled()) {
        geolocation->setIsAllowed(false);
        return;
    }
    DOMWindow* window = frame->domWindow();
    if (!window)
        return;

    CString latinOrigin = frameOrigin(frame);

    GeolocationServiceBlackBerry* service = static_cast<GeolocationServiceBlackBerry*>(geolocation->getGeolocationService());
    m_webPage->client()->requestGeolocationPermission(service->tracker(), latinOrigin.data());
}

void ChromeClientBlackBerry::cancelGeolocationPermissionRequestForFrame(Frame*, Geolocation* geolocation)
{
    GeolocationServiceBlackBerry* service = static_cast<GeolocationServiceBlackBerry*>(geolocation->getGeolocationService());
    m_webPage->client()->cancelGeolocationPermission(service->tracker());
}

void ChromeClientBlackBerry::runOpenPanel(WebCore::Frame*, WTF::PassRefPtr<WebCore::FileChooser> chooser)
{
    PageGroupLoadDeferrer deferrer(m_webPage->d->m_page, true);
    TimerBase::fireTimersInNestedEventLoop();

    SharedArray<WebString> initialFiles;
    unsigned int initialFileSize = chooser->filenames().size();
    if (initialFileSize > 0)
        initialFiles.reset(new WebString[initialFileSize]);
    for (int i = 0; i < initialFileSize; ++i)
        initialFiles[i] = chooser->filenames()[i];

    SharedArray<WebString> chosenFiles;
    unsigned int chosenFileSize;

    if (m_webPage->client()->chooseFilenames(chooser->allowsMultipleFiles(), chooser->acceptTypes(), initialFiles, initialFileSize, chosenFiles, chosenFileSize)) {
        Vector<WTF::String> files(chosenFileSize);
        for (int i = 0; i < chosenFileSize; ++i)
            files[i] = chosenFiles[i];
        chooser->chooseFiles(files);
    }
}

void ChromeClientBlackBerry::setCursor(const Cursor&)
{
    notImplemented();
}

void ChromeClientBlackBerry::formStateDidChange(const Node* node)
{
    m_webPage->d->m_inputHandler->nodeTextChanged(node);
}

PassOwnPtr<HTMLParserQuirks> ChromeClientBlackBerry::createHTMLParserQuirks()
{
    notImplemented();
    return 0;
}

void ChromeClientBlackBerry::scrollbarsModeDidChange() const
{
    notImplemented();
}

void ChromeClientBlackBerry::contentsSizeChanged(WebCore::Frame* frame, const WebCore::IntSize& size) const
{
    if (frame != m_webPage->d->m_mainFrame)
        return;

    m_webPage->d->contentsSizeChanged(size);
}

void ChromeClientBlackBerry::invalidateContents(const IntRect& updateRect, bool immediate)
{
    m_webPage->backingStore()->d->repaint(updateRect, true /*contentChanged*/, immediate, true /*repaintContentOnly*/);
}

void ChromeClientBlackBerry::invalidateWindow(const IntRect& updateRect, bool immediate)
{
    m_webPage->backingStore()->d->repaint(updateRect, false /*contentChanged*/, immediate, false /*repaintContentOnly*/);
}

void ChromeClientBlackBerry::invalidateContentsAndWindow(const IntRect& updateRect, bool immediate)
{
    m_webPage->backingStore()->d->repaint(updateRect, true /*contentChanged*/, immediate, false /*repaintContentOnly*/);
}

void ChromeClientBlackBerry::invalidateContentsForSlowScroll(const IntSize& delta, const IntRect& updateRect, bool immediate, const ScrollView* scrollView)
{
    if (scrollView != m_webPage->d->m_mainFrame->view())
        m_webPage->backingStore()->d->repaint(updateRect, true /*contentChanged*/, true /*immediate*/, false /*repaintContentOnly*/);
    else {
        m_webPage->d->checkOriginOfCurrentScrollOperation();
        m_webPage->backingStore()->d->slowScroll(delta, updateRect, immediate);
    }
}

void ChromeClientBlackBerry::scroll(const IntSize& delta, const IntRect& scrollViewRect, const IntRect& clipRect)
{
    // FIXME: There's a chance the function is called indirectly by FrameView's dtor
    // when the Frame's view() is null. We probably want to fix it in another way, but
    // at this moment let's do a quick fix.
    if (!m_webPage->d->m_mainFrame->view())
        return;

    m_webPage->d->checkOriginOfCurrentScrollOperation();
    m_webPage->backingStore()->d->scroll(delta, scrollViewRect, clipRect);
}

void ChromeClientBlackBerry::scrollRectIntoView(const IntRect&, const ScrollView*) const
{
    m_webPage->d->notifyTransformedScrollChanged();
}

bool ChromeClientBlackBerry::shouldInterruptJavaScript()
{
    TimerBase::fireTimersInNestedEventLoop();
    return m_webPage->client()->shouldInterruptJavaScript();
}

PlatformPageClient ChromeClientBlackBerry::platformPageClient() const
{
    return m_webPage->d;
}

void ChromeClientBlackBerry::reachedMaxAppCacheSize(int64_t spaceNeeded)
{
    notImplemented();
}

void ChromeClientBlackBerry::layoutFinished(WebCore::Frame* frame) const
{
    if (frame != m_webPage->d->m_mainFrame)
        return;

    m_webPage->d->layoutFinished();
}

void ChromeClientBlackBerry::overflowExceedsContentsSize(WebCore::Frame* frame) const
{
    if (frame != m_webPage->d->m_mainFrame)
        return;

#if DEBUG_OVERFLOW_DETECTION
    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "ChromeClientBlackBerry::overflowExceedsContentsSize contents=%dx%d overflow=%dx%d",
                           frame->contentRenderer()->rightLayoutOverflow(),
                           frame->contentRenderer()->bottomLayoutOverflow(),
                           frame->contentRenderer()->rightAbsoluteVisibleOverflow(),
                           frame->contentRenderer()->bottomAbsoluteVisibleOverflow());
#endif
    m_webPage->d->overflowExceedsContentsSize();
}

void ChromeClientBlackBerry::didDiscoverFrameSet(WebCore::Frame* frame) const
{
    if (frame != m_webPage->d->m_mainFrame)
        return;

    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "ChromeClientBlackBerry::didDiscoverFrameSet");
    if (m_webPage->d->loadState() == Olympia::WebKit::WebPagePrivate::Committed) {
        m_webPage->d->setShouldUseFixedDesktopMode(true);
        m_webPage->d->zoomToInitialScaleOnLoad();
    }
}

int ChromeClientBlackBerry::reflowWidth() const
{
    return m_webPage->d->reflowWidth();
}

void ChromeClientBlackBerry::chooseIconForFiles(const Vector<WTF::String>&, FileChooser*)
{
    notImplemented();
}

#if ENABLE(SVG)
void ChromeClientBlackBerry::didSetSVGZoomAndPan(Frame* frame, unsigned short zoomAndPan)
{
    // For top-level SVG document, there is no viewport tag, we use viewport's user-scalable
    // to enable/disable zoom when top-level SVG document's zoomAndPan changed. Because there is no viewport
    // tag, other fields with default value in ViewportArguments are ok.
    if (frame == m_webPage->d->m_page->mainFrame()) {
        ViewportArguments arguments;
        switch (zoomAndPan) {
        case SVGZoomAndPan::SVG_ZOOMANDPAN_DISABLE:
            arguments.userScalable = 0;
            break;
        case SVGZoomAndPan::SVG_ZOOMANDPAN_MAGNIFY:
            arguments.userScalable = 1;
            break;
        default:
            return;
        }
        didReceiveViewportArguments(frame, arguments);
    }
}
#endif

} // namespace WebCore
