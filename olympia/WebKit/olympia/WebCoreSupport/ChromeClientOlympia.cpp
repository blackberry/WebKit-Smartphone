/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "ChromeClientOlympia.h"

#include "BackingStore.h"
#include "BackingStore_p.h"
#include "CString.h"
#include "DatabaseTracker.h"
#include "PageGroupLoadDeferrer.h"

#include "Document.h"
#include "DumpRenderTreeOlympia.h"
#include "FileChooser.h"
#include "Frame.h"
#include "FrameLoadRequest.h"
#include "FrameLoader.h"
#include "Geolocation.h"
#include "GeolocationServiceOlympia.h"
#include "HitTestResult.h"
#include "InputHandler.h"
#include "KURL.h"
#include "NotImplemented.h"
#include "OlympiaPlatformMisc.h"
#include "OlympiaPlatformSettings.h"
#include "OlympiaString.h"
#include "Page.h"
#include "PageGroup.h"
#include "PlatformString.h"
#include "RenderView.h"
#include "SharedPointer.h"
#include "SVGZoomAndPan.h"
#include "ViewportArguments.h"
#include "WebPage.h"
#include "WebPageClient.h"
#include "WebPage_p.h"
#include "WebSettings.h"
#include "WindowFeatures.h"

#define DEBUG_OVERFLOW_DETECTION 0

namespace WebCore {

ChromeClientOlympia::ChromeClientOlympia(Olympia::WebKit::WebPage* page)
    : m_webPage(page)
{
}

void ChromeClientOlympia::addMessageToConsole(MessageSource, MessageType, MessageLevel, const String& message, unsigned int lineNumber, const String& sourceID)
{
    if (m_webPage->d->m_dumpRenderTree)
        m_webPage->d->m_dumpRenderTree->addMessageToConsole(message, lineNumber, sourceID);
    m_webPage->client()->addMessageToConsole(message.characters(), message.length(), sourceID.characters(), sourceID.length(), lineNumber);
}

void ChromeClientOlympia::runJavaScriptAlert(Frame*, const String& message)
{
    if (m_webPage->d->m_dumpRenderTree) {
        m_webPage->d->m_dumpRenderTree->runJavaScriptAlert(message);
        return;
    }
    m_webPage->client()->runJavaScriptAlert(message.characters(), message.length());
}

bool ChromeClientOlympia::runJavaScriptConfirm(Frame*, const String& message)
{
    if (m_webPage->d->m_dumpRenderTree)
        return m_webPage->d->m_dumpRenderTree->runJavaScriptConfirm(message);
    return m_webPage->client()->runJavaScriptConfirm(message.characters(), message.length());
}

bool ChromeClientOlympia::runJavaScriptPrompt(Frame*, const String& message, const String& defaultValue, String& result)
{
    Olympia::WebKit::String clientResult;

    if (m_webPage->d->m_dumpRenderTree) {
        result = m_webPage->d->m_dumpRenderTree->runJavaScriptPrompt(message, defaultValue);
        return true;
    }

    if (m_webPage->client()->runJavaScriptPrompt(message.characters(), message.length(), defaultValue.characters(), defaultValue.length(), clientResult)) {
        result = clientResult;
        return true;
    }
    return false;
}

void ChromeClientOlympia::chromeDestroyed()
{
    delete this;
}

void ChromeClientOlympia::setWindowRect(const FloatRect&)
{
    return; // The window dimensions are fixed in the RIM port.
}

FloatRect ChromeClientOlympia::windowRect()
{
    IntSize applicationViewSize = m_webPage->settings()->applicationViewSize();
    return FloatRect(0, 0, applicationViewSize.width(), applicationViewSize.height());;
}

FloatRect ChromeClientOlympia::pageRect()
{
    notImplemented();
    return FloatRect();
}

float ChromeClientOlympia::scaleFactor()
{
    return 1.0f;
}

void ChromeClientOlympia::focus()
{
    notImplemented();
}

void ChromeClientOlympia::unfocus()
{
    notImplemented();
}

bool ChromeClientOlympia::canTakeFocus(FocusDirection)
{
    notImplemented();
    return false;
}

void ChromeClientOlympia::takeFocus(FocusDirection)
{
    notImplemented();
}

void ChromeClientOlympia::focusedNodeChanged(Node* node)
{
    m_webPage->d->sendContextIfChanged(node);
}

Page* ChromeClientOlympia::createWindow(Frame*, const FrameLoadRequest& request, const WindowFeatures& features)
{
    PageGroupLoadDeferrer deferrer(m_webPage->d->m_page, true);

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

void ChromeClientOlympia::show()
{
    notImplemented();
}

bool ChromeClientOlympia::canRunModal()
{
    notImplemented();
    return false;
}

void ChromeClientOlympia::runModal()
{
    notImplemented();
}

void ChromeClientOlympia::setToolbarsVisible(bool)
{
    notImplemented();
}

bool ChromeClientOlympia::toolbarsVisible()
{
    notImplemented();
    return false;
}

void ChromeClientOlympia::setStatusbarVisible(bool)
{
    notImplemented();
}

bool ChromeClientOlympia::statusbarVisible()
{
    notImplemented();
    return false;
}

void ChromeClientOlympia::setScrollbarsVisible(bool)
{
    notImplemented();
}

bool ChromeClientOlympia::scrollbarsVisible()
{
    notImplemented();
    return false;
}

void ChromeClientOlympia::setMenubarVisible(bool)
{
    notImplemented();
}

bool ChromeClientOlympia::menubarVisible()
{
    notImplemented();
    return false;
}

void ChromeClientOlympia::setResizable(bool)
{
    notImplemented();
}

bool ChromeClientOlympia::canRunBeforeUnloadConfirmPanel()
{
    notImplemented();
    return false;
}

bool ChromeClientOlympia::runBeforeUnloadConfirmPanel(const String& message, Frame*)
{
    if (m_webPage->d->m_dumpRenderTree)
        return m_webPage->d->m_dumpRenderTree->runBeforeUnloadConfirmPanel(message);

    notImplemented();
    return false;
}

void ChromeClientOlympia::closeWindowSoon()
{
    m_webPage->client()->scheduleCloseWindow();
}

void ChromeClientOlympia::setStatusbarText(const String& status)
{
    if (m_webPage->d->m_dumpRenderTree)
        return m_webPage->d->m_dumpRenderTree->setStatusText(status);
    notImplemented();
}

bool ChromeClientOlympia::tabsToLinks() const
{
    // FIXME: We should probably make it configurable
    return true;
}

IntRect ChromeClientOlympia::windowResizerRect() const
{
    notImplemented();
    return IntRect();
}

IntPoint ChromeClientOlympia::screenToWindow(const IntPoint&) const
{
    notImplemented();
    return IntPoint();
}

IntRect ChromeClientOlympia::windowToScreen(const IntRect&) const
{
    notImplemented();
    return IntRect();
}

void* ChromeClientOlympia::platformWindow() const
{
    notImplemented();
    return 0;
}

void ChromeClientOlympia::mouseDidMoveOverElement(const HitTestResult& result, unsigned int modifierFlags)
{
    m_webPage->d->sendContextIfChanged(result.innerNode());
}

void ChromeClientOlympia::setToolTip(const String&, TextDirection)
{
    notImplemented();
}

void ChromeClientOlympia::didReceiveViewportArguments(Frame* frame, const ViewportArguments& arguments)
{
    if (m_webPage->d->m_mainFrame != frame)
        return;

    m_webPage->d->didReceiveViewportArguments(arguments);
}

void ChromeClientOlympia::print(Frame*)
{
    notImplemented();
}

void ChromeClientOlympia::exceededDatabaseQuota(Frame* frame, const String& name)
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
    const String& nameStr = details.displayName();

    String originStr = origin->databaseIdentifier();

    unsigned long long quota = m_webPage->client()->databaseQuota(originStr.characters(), originStr.length(),
        nameStr.characters(), nameStr.length(), totalUsage, originUsage, estimatedSize);

    tracker.setQuota(origin, quota);
#endif
}

void ChromeClientOlympia::requestGeolocationPermissionForFrame(Frame*, Geolocation* geolocation)
{
    if (!m_webPage->settings()->isGeolocationEnabled()) {
        geolocation->setIsAllowed(false);
        return;
    }

    GeolocationServiceOlympia* service = static_cast<GeolocationServiceOlympia*>(geolocation->getGeolocationService());
    m_webPage->client()->requestGeolocationPermission(service->tracker());
}

void ChromeClientOlympia::cancelGeolocationPermissionRequestForFrame(Frame*, Geolocation* geolocation)
{
    GeolocationServiceOlympia* service = static_cast<GeolocationServiceOlympia*>(geolocation->getGeolocationService());
    m_webPage->client()->cancelGeolocationPermission(service->tracker());
}

void ChromeClientOlympia::runOpenPanel(WebCore::Frame*, WTF::PassRefPtr<WebCore::FileChooser> chooser)
{
    PageGroupLoadDeferrer deferrer(m_webPage->d->m_page, true);

    SharedArray<Olympia::WebKit::String> initialFiles;
    unsigned int initialFileSize = chooser->filenames().size();
    if (initialFileSize > 0)
        initialFiles.reset(new Olympia::WebKit::String[initialFileSize]);
    for (int i = 0; i < initialFileSize; ++i)
        initialFiles[i] = chooser->filenames()[i];

    SharedArray<Olympia::WebKit::String> chosenFiles;
    unsigned int chosenFileSize;

    if (m_webPage->client()->chooseFilenames(chooser->allowsMultipleFiles(), chooser->acceptTypes(), initialFiles, initialFileSize, chosenFiles, chosenFileSize)) {
        Vector<String> files(chosenFileSize);
        for (int i = 0; i < chosenFileSize; ++i)
            files[i] = chosenFiles[i];
        chooser->chooseFiles(files);
    }
}

bool ChromeClientOlympia::setCursor(PlatformCursorHandle)
{
    notImplemented();
    return false;
}

void ChromeClientOlympia::formStateDidChange(const Node* node)
{
    m_webPage->d->m_inputHandler->nodeTextChanged(node);
}

PassOwnPtr<HTMLParserQuirks> ChromeClientOlympia::createHTMLParserQuirks()
{
    notImplemented();
    return 0;
}

void ChromeClientOlympia::scrollbarsModeDidChange() const
{
    notImplemented();
}

void ChromeClientOlympia::contentsSizeChanged(WebCore::Frame* frame, const WebCore::IntSize& size) const
{
    if (frame != m_webPage->d->m_mainFrame)
        return;

    m_webPage->d->contentsSizeChanged(size);
}

void ChromeClientOlympia::invalidateContents(const IntRect& updateRect, bool immediate)
{
    m_webPage->backingStore()->d->repaint(updateRect, true /*contentChanged*/, immediate, true /*repaintContentOnly*/);
}

void ChromeClientOlympia::invalidateWindow(const IntRect& updateRect, bool immediate)
{
    m_webPage->backingStore()->d->repaint(updateRect, false /*contentChanged*/, immediate, false /*repaintContentOnly*/);
}

void ChromeClientOlympia::invalidateContentsAndWindow(const IntRect& updateRect, bool immediate)
{
    m_webPage->backingStore()->d->repaint(updateRect, true /*contentChanged*/, immediate, false /*repaintContentOnly*/);
}

void ChromeClientOlympia::invalidateContentsForSlowScroll(const IntSize& delta, const IntRect& updateRect, bool immediate, const ScrollView* scrollView)
{
    if (scrollView != m_webPage->d->m_mainFrame->view())
        m_webPage->backingStore()->d->repaint(updateRect, true /*contentChanged*/, true /*immediate*/, false /*repaintContentOnly*/);
    else {
        m_webPage->d->checkOriginOfCurrentScrollOperation();
        m_webPage->backingStore()->d->slowScroll(delta, updateRect, immediate);
    }
}

void ChromeClientOlympia::scroll(const IntSize& delta, const IntRect& scrollViewRect, const IntRect& clipRect)
{
    // FIXME: There's a chance the function is called indirectly by FrameView's dtor
    // when the Frame's view() is null. We probably want to fix it in another way, but
    // at this moment let's do a quick fix.
    if (!m_webPage->d->m_mainFrame->view())
        return;

    m_webPage->d->checkOriginOfCurrentScrollOperation();
    m_webPage->backingStore()->d->scroll(delta, scrollViewRect, clipRect);
}

void ChromeClientOlympia::scrollRectIntoView(const IntRect&, const ScrollView*) const
{
    m_webPage->d->notifyTransformedScrollChanged();
}

bool ChromeClientOlympia::shouldInterruptJavaScript()
{
    return m_webPage->client()->shouldInterruptJavaScript();
}

PlatformPageClient ChromeClientOlympia::platformPageClient() const
{
    return m_webPage->d;
}

void ChromeClientOlympia::reachedMaxAppCacheSize(int64_t spaceNeeded)
{
    notImplemented();
}

void ChromeClientOlympia::layoutFinished(WebCore::Frame* frame) const
{
    if (frame != m_webPage->d->m_mainFrame)
        return;

    m_webPage->d->layoutFinished();
}

void ChromeClientOlympia::overflowExceedsContentsSize(WebCore::Frame* frame) const
{
    if (frame != m_webPage->d->m_mainFrame)
        return;

#if DEBUG_OVERFLOW_DETECTION
    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "ChromeClientOlympia::overflowExceedsContentsSize contents=%dx%d overflow=%dx%d",
                           frame->contentRenderer()->rightLayoutOverflow(),
                           frame->contentRenderer()->bottomLayoutOverflow(),
                           frame->contentRenderer()->rightAbsoluteVisibleOverflow(),
                           frame->contentRenderer()->bottomAbsoluteVisibleOverflow());
#endif
    m_webPage->d->overflowExceedsContentsSize();
}

void ChromeClientOlympia::didDiscoverFrameSet(WebCore::Frame* frame) const
{
    if (frame != m_webPage->d->m_mainFrame)
        return;

    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "ChromeClientOlympia::didDiscoverFrameSet");
    if (m_webPage->d->loadState() == Olympia::WebKit::WebPagePrivate::Committed) {
        m_webPage->d->setShouldUseFixedDesktopMode(true);
        m_webPage->d->zoomToInitialScaleOnLoad();
    }
}

int ChromeClientOlympia::reflowWidth() const
{
    return m_webPage->d->reflowWidth();
}

void ChromeClientOlympia::chooseIconForFiles(const Vector<String>&, FileChooser*)
{
    notImplemented();
}

#if ENABLE(SVG)
void ChromeClientOlympia::didSetSVGZoomAndPan(Frame* frame, unsigned short zoomAndPan)
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
