/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "SelectionHandler.h"

#include "Document.h"
#include "Editor.h"
#include "EditorClient.h"
#include "Frame.h"
#include "InputHandler.h"
#include "IntRect.h"
#include "Page.h"
#include "RenderPart.h"
#include "WebPageClient.h"
#include "WebPage.h"
#include "WebPage_p.h"
#include "WebPageClient.h"

#define SHOWDEBUG_SELECTIONHANDLER 0

using namespace WebCore;

namespace Olympia {
namespace WebKit {

SelectionHandler::SelectionHandler(WebPagePrivate* page)
    : m_webPage(page)
    , m_selectionActive(false)
{
}

SelectionHandler::~SelectionHandler()
{
}

void SelectionHandler::cancelSelection()
{
    m_selectionActive = false;

#if SHOWDEBUG_SELECTIONHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "SelectionHandler::cancelSelection");
#endif

    if (m_webPage->m_inputHandler->isInputMode())
        m_webPage->m_inputHandler->cancelSelection();
    else
        m_webPage->focusedOrMainFrame()->selection()->clear();
}

Olympia::WebKit::String SelectionHandler::selectedText() const
{
    return m_webPage->focusedOrMainFrame()->selectedText();
}

bool SelectionHandler::findNextString(const WebCore::String& searchString)
{
    if (searchString.isEmpty()) {
        cancelSelection();
        return false;
    }

    ASSERT(m_webPage->m_page);

    m_webPage->m_page->unmarkAllTextMatches();

    bool result = m_webPage->m_page->findString(searchString, WebCore::TextCaseInsensitive, WebCore::FindDirectionForward, true /*should wrap*/);
    if (result && m_webPage->focusedOrMainFrame()->selection()->selectionType() == VisibleSelection::NoSelection) {
        // Word was found but could not be selected on this page.
        result = m_webPage->m_page->markAllMatchesForText(searchString, WebCore::TextCaseInsensitive, true /*should highlight*/, 0 /*limit to match 0 = unlimited*/);
    }

    // Defocus the input field if one is active.
    if (m_webPage->m_inputHandler->isInputMode())
        m_webPage->m_inputHandler->nodeFocused(0);

    return result;
}

void SelectionHandler::setSelection(WebCore::IntPoint start, WebCore::IntPoint end)
{
    m_selectionActive = true;

    ASSERT(m_webPage && m_webPage->mainFrame() && m_webPage->mainFrame()->selection());
    Frame* frame = m_webPage->mainFrame();

#if SHOWDEBUG_SELECTIONHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "SelectionHandler::setSelection adjusted points %d, %d, %d, %d", start.x(), start.y(), end.x(), end.y());
#endif

    VisibleSelection newSelection(frame->visiblePositionForPoint(start), frame->visiblePositionForPoint(end));
    frame->selection()->setSelection(newSelection);

    // Need to manually trigger notification as response to change.
    selectionPositionChanged();
}

void SelectionHandler::selectAtPoint(WebCore::IntPoint& location)
{
    // selectAtPoint API currently only supports WordGranularity but may be extended in the future.
    selectObject(location, WordGranularity);
}

static void expandSelectionToGranularity(Frame* frame, int x, int y, TextGranularity granularity, bool isInputMode)
{
    VisiblePosition pointLocation(frame->visiblePositionForPoint(WebCore::IntPoint(x, y)));
    VisibleSelection selection(pointLocation, pointLocation);
    if (!(selection.start().anchorNode() && selection.start().anchorNode()->isTextNode()))
        return;
    selection.expandUsingGranularity(granularity);
    RefPtr<Range> newRange = selection.toNormalizedRange();
    if (!newRange)
        return;
    ExceptionCode ec = 0;
    if (newRange->collapsed(ec))
        return;
    RefPtr<Range> oldRange = frame->selection()->selection().toNormalizedRange();
    EAffinity affinity = frame->selection()->affinity();
    if (isInputMode && !frame->editor()->client()->shouldChangeSelectedRange(oldRange.get(), newRange.get(), affinity, false))
        return;
    frame->selection()->setSelectedRange(newRange.get(), affinity, true);
}

void SelectionHandler::selectObject(WebCore::IntPoint& location, TextGranularity granularity)
{
    m_selectionActive = true;

    ASSERT(m_webPage && m_webPage->mainFrame() && m_webPage->mainFrame()->selection());
    Frame* frame = m_webPage->mainFrame();

#if SHOWDEBUG_SELECTIONHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "SelectionHandler::selectWord adjusted points %d, %d", location.x(), location.y());
#endif

    expandSelectionToGranularity(frame, location.x(), location.y(), WordGranularity, m_webPage->m_inputHandler->isInputMode());

    // Need to manually trigger notification as response to change.
    selectionPositionChanged();
}

WebCore::IntRect SelectionHandler::selectionStartCaretRect()
{
    if (!m_selectionActive)
        return IntRect();

    ASSERT(m_webPage && m_webPage->focusedOrMainFrame() && m_webPage->focusedOrMainFrame()->selection());

    WebCore::IntRect caretLocation(m_webPage->focusedOrMainFrame()->selection()->selection().visibleStart().absoluteCaretBounds());

    if(m_webPage->focusedOrMainFrame()->ownerRenderer()) {
        WebCore::IntPoint frameOffset = m_webPage->focusedOrMainFrame()->ownerRenderer()->absoluteContentBox().location();
        caretLocation.move(frameOffset.x(), frameOffset.y());
    }
    return caretLocation;
}

WebCore::IntRect SelectionHandler::selectionEndCaretRect()
{
    if (!m_selectionActive)
        return IntRect();

    ASSERT(m_webPage && m_webPage->focusedOrMainFrame() && m_webPage->focusedOrMainFrame()->selection());

    WebCore::IntRect caretLocation(m_webPage->focusedOrMainFrame()->selection()->selection().visibleEnd().absoluteCaretBounds());

    if(m_webPage->focusedOrMainFrame()->ownerRenderer()) {
        WebCore::IntPoint frameOffset = m_webPage->focusedOrMainFrame()->ownerRenderer()->absoluteContentBox().location();
        caretLocation.move(frameOffset.x(), frameOffset.y());
    }
    return caretLocation;
}

// Note:  This is the only function in SelectionHandler in which the coordinate
// system is not entirely WebKit.
void SelectionHandler::selectionPositionChanged()
{
    if (!m_selectionActive)
        return;

    // Get the transformed coordinates from the WebPage.
    WebCore::IntRect startCaret = m_webPage->m_webPage->selectionStartCaretRect();
    WebCore::IntRect endCaret = m_webPage->m_webPage->selectionEndCaretRect();

#if SHOWDEBUG_SELECTIONHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "SelectionHandler::selectionPositionChanged Start Rect %d, %d, %dx%d End Rect%d, %d, %dx%d",
                                   startCaret.x(), startCaret.y(), startCaret.width(), startCaret.height(),
                                   endCaret.x(), endCaret.y(), endCaret.width(), endCaret.height());
#endif

    m_webPage->m_client->selectionBounds(startCaret, endCaret);
}

}
}
