/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 * Copyright (C) 2006, 2007 Apple, Inc.  All rights reserved.
 * Copyright (C) 2009 Google, Inc.  All rights reserved.

 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "EditorClientBlackBerry.h"

#include "NotImplemented.h"

#include "DumpRenderTreeBlackBerry.h"
#include "EditCommand.h"
#include "FocusController.h"
#include "Frame.h"
#include "HTMLElement.h"
#include "HTMLInputElement.h"
#include "HTMLTextAreaElement.h"
#include "HTMLNames.h"
#include "InputHandler.h"
#include "KeyboardEvent.h"
#include "Node.h"
#include "OlympiaPlatformMisc.h"
#include "Page.h"
#include "PlatformKeyboardEvent.h"
#include "WebPage.h"
#include "WebPage_p.h"
#include "WindowsKeyboardCodes.h"

namespace WebCore {

    // Arbitrary depth limit for the undo stack, to keep it from using
    // unbounded memory.  This is the maximum number of distinct undoable
    // actions -- unbroken stretches of typed characters are coalesced
    // into a single action only when not interrupted by string replacements
    // triggered by replaceText calls.
    static const size_t maximumUndoStackDepth = 1000;

EditorClientBlackBerry::EditorClientBlackBerry(Olympia::WebKit::WebPage* page)
    : m_page(page)
    , m_waitingForCursorFocus(false)
    , m_inRedo(false)
{
}

void EditorClientBlackBerry::pageDestroyed()
{
    delete this;
}

bool EditorClientBlackBerry::shouldDeleteRange(Range* range)
{
    if (m_page->d->m_dumpRenderTree)
        return m_page->d->m_dumpRenderTree->shouldDeleteDOMRange(range);
    return true;
}

bool EditorClientBlackBerry::shouldShowDeleteInterface(HTMLElement*)
{
    notImplemented();
    return false;
}

bool EditorClientBlackBerry::smartInsertDeleteEnabled()
{
    notImplemented();
    return false;
}

bool EditorClientBlackBerry::isSelectTrailingWhitespaceEnabled()
{
    notImplemented();
    return false;
}

bool EditorClientBlackBerry::isContinuousSpellCheckingEnabled()
{
    notImplemented();
    return false;
}

void EditorClientBlackBerry::toggleContinuousSpellChecking()
{
    notImplemented();
}

bool EditorClientBlackBerry::isGrammarCheckingEnabled()
{
    notImplemented();
    return false;
}

void EditorClientBlackBerry::toggleGrammarChecking()
{
    notImplemented();
}

int EditorClientBlackBerry::spellCheckerDocumentTag()
{
    notImplemented();
    return 0;
}

bool EditorClientBlackBerry::isEditable()
{
    notImplemented();
    return false;
}

bool EditorClientBlackBerry::shouldBeginEditing(Range* range)
{
    if (m_page->d->m_dumpRenderTree)
        return m_page->d->m_dumpRenderTree->shouldBeginEditingInDOMRange(range);
    return true;
}

bool EditorClientBlackBerry::shouldEndEditing(Range* range)
{
    if (m_page->d->m_dumpRenderTree)
        return m_page->d->m_dumpRenderTree->shouldEndEditingInDOMRange(range);
    return true;
}

bool EditorClientBlackBerry::shouldInsertNode(Node*, Range*, EditorInsertAction)
{
    notImplemented();
    return true;
}

bool EditorClientBlackBerry::shouldInsertText(const WTF::String&, Range*, EditorInsertAction)
{
    notImplemented();
    return true;
}

bool EditorClientBlackBerry::shouldChangeSelectedRange(Range* fromRange, Range* toRange, EAffinity affinity, bool stillSelecting)
{
    if (m_page->d->m_dumpRenderTree)
        return m_page->d->m_dumpRenderTree->shouldChangeSelectedDOMRangeToDOMRangeAffinityStillSelecting(fromRange, toRange, affinity, stillSelecting);

    Frame* frame = m_page->d->m_page->focusController()->focusedOrMainFrame();
    if (frame && frame->document()) {
        if (frame->document()->focusedNode() && frame->document()->focusedNode()->hasTagName(HTMLNames::selectTag))
            return false;
        m_page->d->m_inputHandler->nodeFocused(frame->document()->focusedNode());
    }

    return true;
}

bool EditorClientBlackBerry::shouldApplyStyle(CSSStyleDeclaration*, Range*)
{
    notImplemented();
    return true;
}

bool EditorClientBlackBerry::shouldMoveRangeAfterDelete(Range*, Range*)
{
    notImplemented();
    return true;
}

void EditorClientBlackBerry::didBeginEditing()
{
    if (m_page->d->m_dumpRenderTree)
        m_page->d->m_dumpRenderTree->didBeginEditing();
}

void EditorClientBlackBerry::respondToChangedContents()
{
    if (m_page->d->m_dumpRenderTree)
        m_page->d->m_dumpRenderTree->didChange();
}

void EditorClientBlackBerry::respondToChangedSelection()
{
    if (m_waitingForCursorFocus)
        m_waitingForCursorFocus = false;
    else
        m_page->selectionChanged();

    if (m_page->d->m_dumpRenderTree)
        m_page->d->m_dumpRenderTree->didChangeSelection();
}

void EditorClientBlackBerry::didEndEditing()
{
    if (m_page->d->m_dumpRenderTree)
        m_page->d->m_dumpRenderTree->didEndEditing();
}

void EditorClientBlackBerry::didWriteSelectionToPasteboard()
{
    notImplemented();
}

void EditorClientBlackBerry::didSetSelectionTypesForPasteboard()
{
    notImplemented();
}

void EditorClientBlackBerry::registerCommandForUndo(PassRefPtr<EditCommand> command)
{
    if (m_undoStack.size() == maximumUndoStackDepth)
        m_undoStack.removeFirst(); // drop oldest item off the far end
    if (!m_inRedo)
        m_redoStack.clear();
    m_undoStack.append(command);
}

void EditorClientBlackBerry::registerCommandForRedo(PassRefPtr<EditCommand> command)
{
    m_redoStack.append(command);
}

void EditorClientBlackBerry::clearUndoRedoOperations()
{
    m_undoStack.clear();
    m_redoStack.clear();
}

bool EditorClientBlackBerry::canUndo() const
{
    return !m_undoStack.isEmpty();
}

bool EditorClientBlackBerry::canRedo() const
{
    return !m_redoStack.isEmpty();
}

void EditorClientBlackBerry::undo()
{
    if (canUndo()) {
        EditCommandStack::iterator back = --m_undoStack.end();
        RefPtr<EditCommand> command(*back);
        m_undoStack.remove(back);

        command->unapply();
        // unapply will call us back to push this command onto the redo stack.
    }
}

void EditorClientBlackBerry::redo()
{
    if (canRedo()) {
        EditCommandStack::iterator back = --m_redoStack.end();
        RefPtr<EditCommand> command(*back);
        m_redoStack.remove(back);

        ASSERT(!m_inRedo);
        m_inRedo = true;
        command->reapply();
        // reapply will call us back to push this command onto the undo stack.
        m_inRedo = false;
    }
}

void EditorClientBlackBerry::handleKeyboardEvent(KeyboardEvent* event)
{
    Node* node = event->target()->toNode();
    ASSERT(node);
    Frame* frame = node->document()->frame();
    ASSERT(frame);

    ASSERT(m_page->d->m_inputHandler);

    const PlatformKeyboardEvent* platformEvent = event->keyEvent();
    if (!platformEvent)
        return;

    Node* start = frame->selection()->start().node();
    if (!start)
        return;

    if (start->isContentEditable()) {

        // Text insertion commands should only be triggered from keypressEvent.
        // There is an assert guaranteeing this in
        // EventHandler::handleTextInputEvent.  Note that windowsVirtualKeyCode
        // is not set for keypressEvent: special keys should have been already
        // handled in keydownEvent, which is called first.
        if (event->type() == eventNames().keypressEvent) {
            if (event->charCode() == '\r') { // \n has been converted to \r in PlatformKeyboardEventOlympia
                if (frame->editor()->command("InsertNewline").execute(event))
                    event->setDefaultHandled();
            } else if (event->charCode() >= ' ' // Don't insert null or control characters as they can result in unexpected behaviour
                    && !platformEvent->text().isEmpty()) {
                if (frame->editor()->insertText(platformEvent->text(), event))
                    event->setDefaultHandled();
            }
            return;
        }

        if (event->type() != eventNames().keydownEvent)
            return;

        bool handledEvent = false;
        switch (platformEvent->windowsVirtualKeyCode()) {
        case VK_BACK:
            handledEvent = frame->editor()->deleteWithDirection(SelectionController::DirectionBackward, CharacterGranularity, false, true);
            break;
        case VK_DELETE:
            handledEvent = frame->editor()->deleteWithDirection(SelectionController::DirectionForward, CharacterGranularity, false, true);
            break;
        case VK_ESCAPE:
            handledEvent = frame->page()->focusController()->setFocusedNode(0, frame);
            break;
        }
        if (handledEvent)
            event->setDefaultHandled();
    }
}

void EditorClientBlackBerry::handleInputMethodKeydown(KeyboardEvent*)
{
    notImplemented();
}

void EditorClientBlackBerry::textFieldDidBeginEditing(Element*)
{
    notImplemented();
}

void EditorClientBlackBerry::textFieldDidEndEditing(Element*)
{
    // Notify the page that editing is complete.
    m_page->d->m_inputHandler->nodeFocused(0);
}

void EditorClientBlackBerry::textDidChangeInTextField(Element*)
{
    notImplemented();
}

bool EditorClientBlackBerry::doTextFieldCommandFromEvent(Element*, KeyboardEvent*)
{
    notImplemented();
    return false;
}

void EditorClientBlackBerry::textWillBeDeletedInTextField(Element*)
{
    notImplemented();
}

void EditorClientBlackBerry::textDidChangeInTextArea(Element*)
{
    notImplemented();
}

void EditorClientBlackBerry::ignoreWordInSpellDocument(const WTF::String&)
{
    notImplemented();
}

void EditorClientBlackBerry::learnWord(const WTF::String&)
{
    notImplemented();
}

void EditorClientBlackBerry::checkSpellingOfString(const UChar*, int, int*, int*)
{
    notImplemented();
}

WTF::String EditorClientBlackBerry::getAutoCorrectSuggestionForMisspelledWord(const WTF::String& misspelledWord)
{
    notImplemented();
    return WTF::String();
};

void EditorClientBlackBerry::checkGrammarOfString(const UChar*, int, WTF::Vector<GrammarDetail, 0u>&, int*, int*)
{
    notImplemented();
}

void EditorClientBlackBerry::updateSpellingUIWithGrammarString(const WTF::String&, const GrammarDetail&)
{
    notImplemented();
}

void EditorClientBlackBerry::updateSpellingUIWithMisspelledWord(const WTF::String&)
{
    notImplemented();
}

void EditorClientBlackBerry::showSpellingUI(bool)
{
    notImplemented();
}

bool EditorClientBlackBerry::spellingUIIsShowing()
{
    notImplemented();
    return false;
}

void EditorClientBlackBerry::getGuessesForWord(const WTF::String&, WTF::Vector<WTF::String, 0u>&)
{
    notImplemented();
}

void EditorClientBlackBerry::willSetInputMethodState()
{
    notImplemented();
}

void EditorClientBlackBerry::setInputMethodState(bool active)
{
    // Focus changed.  We need to find actual node so active can be ignored.
    Frame* frame = m_page->d->m_page->focusController()->focusedOrMainFrame();
    if (active && frame && frame->document())
        m_page->d->m_inputHandler->nodeFocused(frame->document()->focusedNode());
    else {
        // No frame or document, can't be a focused node.
        m_page->d->m_inputHandler->nodeFocused(0);
    }
}

} // namespace WebCore
