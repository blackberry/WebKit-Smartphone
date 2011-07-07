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
#include "EditorClientOlympia.h"

#include "NotImplemented.h"

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

EditorClientOlympia::EditorClientOlympia(Olympia::WebKit::WebPage* page)
    : m_page(page)
    , m_waitingForCursorFocus(false)
    , m_inRedo(false)
{
}

void EditorClientOlympia::pageDestroyed()
{
    delete this;
}

bool EditorClientOlympia::shouldDeleteRange(Range*)
{
    notImplemented();
    return true;
}

bool EditorClientOlympia::shouldShowDeleteInterface(HTMLElement*)
{
    notImplemented();
    return false;
}

bool EditorClientOlympia::smartInsertDeleteEnabled()
{
    notImplemented();
    return false;
}

bool EditorClientOlympia::isSelectTrailingWhitespaceEnabled()
{
    notImplemented();
    return false;
}

bool EditorClientOlympia::isContinuousSpellCheckingEnabled()
{
    notImplemented();
    return false;
}

void EditorClientOlympia::toggleContinuousSpellChecking()
{
    notImplemented();
}

bool EditorClientOlympia::isGrammarCheckingEnabled()
{
    notImplemented();
    return false;
}

void EditorClientOlympia::toggleGrammarChecking()
{
    notImplemented();
}

int EditorClientOlympia::spellCheckerDocumentTag()
{
    notImplemented();
    return 0;
}

bool EditorClientOlympia::isEditable()
{
    notImplemented();
    return false;
}

bool EditorClientOlympia::shouldBeginEditing(Range*)
{
    notImplemented();
    return true;
}

bool EditorClientOlympia::shouldEndEditing(Range*)
{
    notImplemented();
    return true;
}

bool EditorClientOlympia::shouldInsertNode(Node*, Range*, EditorInsertAction)
{
    notImplemented();
    return true;
}

bool EditorClientOlympia::shouldInsertText(const String&, Range*, EditorInsertAction)
{
    notImplemented();
    return true;
}

bool EditorClientOlympia::shouldChangeSelectedRange(Range*, Range*, EAffinity, bool)
{
    Frame* frame = m_page->d->m_page->focusController()->focusedOrMainFrame();
    if (frame && frame->document()) {
        if (frame->document()->focusedNode() && frame->document()->focusedNode()->hasTagName(HTMLNames::selectTag))
            return false;
        m_page->d->m_inputHandler->nodeFocused(frame->document()->focusedNode());
    }

    return true;
}

bool EditorClientOlympia::shouldApplyStyle(CSSStyleDeclaration*, Range*)
{
    notImplemented();
    return true;
}

bool EditorClientOlympia::shouldMoveRangeAfterDelete(Range*, Range*)
{
    notImplemented();
    return true;
}

void EditorClientOlympia::didBeginEditing()
{
    notImplemented();
}

void EditorClientOlympia::respondToChangedContents()
{
    notImplemented();
}

void EditorClientOlympia::respondToChangedSelection()
{
    if (m_waitingForCursorFocus)
        m_waitingForCursorFocus = false;
    else
        m_page->selectionChanged();
}

void EditorClientOlympia::didEndEditing()
{
    notImplemented();
}

void EditorClientOlympia::didWriteSelectionToPasteboard()
{
    notImplemented();
}

void EditorClientOlympia::didSetSelectionTypesForPasteboard()
{
    notImplemented();
}

void EditorClientOlympia::registerCommandForUndo(PassRefPtr<EditCommand> command)
{
    if (!m_page->d->m_inputHandler->canAddCommandToUndoStack())
        return;

    if (m_undoStack.size() == maximumUndoStackDepth)
        m_undoStack.removeFirst(); // drop oldest item off the far end
    if (!m_inRedo)
        m_redoStack.clear();
    m_undoStack.append(command);
}

void EditorClientOlympia::registerCommandForRedo(PassRefPtr<EditCommand> command)
{
    m_redoStack.append(command);
}

void EditorClientOlympia::clearUndoRedoOperations()
{
    m_undoStack.clear();
    m_redoStack.clear();
}

bool EditorClientOlympia::canUndo() const
{
    return !m_undoStack.isEmpty();
}

bool EditorClientOlympia::canRedo() const
{
    return !m_redoStack.isEmpty();
}

void EditorClientOlympia::undo()
{
    if (canUndo()) {
        EditCommandStack::iterator back = --m_undoStack.end();
        RefPtr<EditCommand> command(*back);
        m_undoStack.remove(back);

        command->unapply();
        // unapply will call us back to push this command onto the redo stack.
    }
}

void EditorClientOlympia::redo()
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

void EditorClientOlympia::handleKeyboardEvent(KeyboardEvent* event)
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
            handledEvent = frame->editor()->deleteWithDirection(SelectionController::BACKWARD, CharacterGranularity, false, true);
            break;
        case VK_DELETE:
            handledEvent = frame->editor()->deleteWithDirection(SelectionController::FORWARD, CharacterGranularity, false, true);
            break;
        case VK_ESCAPE:
            handledEvent = frame->page()->focusController()->setFocusedNode(0, frame);
            break;
        }
        if (handledEvent)
            event->setDefaultHandled();
    }
}

void EditorClientOlympia::handleInputMethodKeydown(KeyboardEvent*)
{
    notImplemented();
}

void EditorClientOlympia::textFieldDidBeginEditing(Element*)
{
    notImplemented();
}

void EditorClientOlympia::textFieldDidEndEditing(Element*)
{
    // Notify the page that editing is complete.
    m_page->d->m_inputHandler->nodeFocused(0);
}

void EditorClientOlympia::textDidChangeInTextField(Element*)
{
    notImplemented();
}

bool EditorClientOlympia::doTextFieldCommandFromEvent(Element*, KeyboardEvent*)
{
    notImplemented();
    return false;
}

void EditorClientOlympia::textWillBeDeletedInTextField(Element*)
{
    notImplemented();
}

void EditorClientOlympia::textDidChangeInTextArea(Element*)
{
    notImplemented();
}

// Note: This code is under review for upstreaming.
bool EditorClientOlympia::focusedElementsAreRichlyEditable()
{
    notImplemented();
    return true;
}

void EditorClientOlympia::ignoreWordInSpellDocument(const String&)
{
    notImplemented();
}

void EditorClientOlympia::learnWord(const String&)
{
    notImplemented();
}

void EditorClientOlympia::checkSpellingOfString(const UChar*, int, int*, int*)
{
    notImplemented();
}

String EditorClientOlympia::getAutoCorrectSuggestionForMisspelledWord(const String& misspelledWord)
{
    notImplemented();
    return String();
};

void EditorClientOlympia::checkGrammarOfString(const UChar*, int, WTF::Vector<GrammarDetail, 0u>&, int*, int*)
{
    notImplemented();
}

void EditorClientOlympia::updateSpellingUIWithGrammarString(const String&, const GrammarDetail&)
{
    notImplemented();
}

void EditorClientOlympia::updateSpellingUIWithMisspelledWord(const String&)
{
    notImplemented();
}

void EditorClientOlympia::showSpellingUI(bool)
{
    notImplemented();
}

bool EditorClientOlympia::spellingUIIsShowing()
{
    notImplemented();
    return false;
}

void EditorClientOlympia::getGuessesForWord(const String&, WTF::Vector<String, 0u>&)
{
    notImplemented();
}

void EditorClientOlympia::setInputMethodState(bool active)
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