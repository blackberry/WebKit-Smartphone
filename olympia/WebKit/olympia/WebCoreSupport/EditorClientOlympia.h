/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 * Copyright (C) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef EditorClientOlympia_h
#define EditorClientOlympia_h

#include "EditorClient.h"

#include <wtf/Deque.h>

namespace Olympia {
    namespace WebKit {
        class WebPage;
    }
}

namespace WebCore {

class EditorClientOlympia : public EditorClient {
public:
    EditorClientOlympia(Olympia::WebKit::WebPage* page);
    virtual void pageDestroyed();
    virtual bool shouldDeleteRange(Range*);
    virtual bool shouldShowDeleteInterface(HTMLElement*);
    virtual bool smartInsertDeleteEnabled();
    virtual bool isSelectTrailingWhitespaceEnabled();
    virtual bool isContinuousSpellCheckingEnabled();
    virtual void toggleContinuousSpellChecking();
    virtual bool isGrammarCheckingEnabled();
    virtual void toggleGrammarChecking();
    virtual int spellCheckerDocumentTag();
    virtual bool isEditable();
    virtual bool shouldBeginEditing(Range*);
    virtual bool shouldEndEditing(Range*);
    virtual bool shouldInsertNode(Node*, Range*, EditorInsertAction);
    virtual bool shouldInsertText(const String&, Range*, EditorInsertAction);
    virtual bool shouldChangeSelectedRange(Range*, Range*, EAffinity, bool);
    virtual bool shouldApplyStyle(CSSStyleDeclaration*, Range*);
    virtual bool shouldMoveRangeAfterDelete(Range*, Range*);
    virtual void didBeginEditing();
    virtual void respondToChangedContents();
    virtual void respondToChangedSelection();
    virtual void didEndEditing();
    virtual void didWriteSelectionToPasteboard();
    virtual void didSetSelectionTypesForPasteboard();
    virtual void registerCommandForUndo(WTF::PassRefPtr<EditCommand>);
    virtual void registerCommandForRedo(WTF::PassRefPtr<EditCommand>);
    virtual void clearUndoRedoOperations();
    virtual bool canUndo() const;
    virtual bool canRedo() const;
    virtual void undo();
    virtual void redo();
    virtual void handleKeyboardEvent(KeyboardEvent*);
    virtual void handleInputMethodKeydown(KeyboardEvent*);
    virtual void textFieldDidBeginEditing(Element*);
    virtual void textFieldDidEndEditing(Element*);
    virtual void textDidChangeInTextField(Element*);
    virtual bool doTextFieldCommandFromEvent(Element*, KeyboardEvent*);
    virtual void textWillBeDeletedInTextField(Element*);
    virtual void textDidChangeInTextArea(Element*);
    // Note: This code is under review for upstreaming.
    virtual bool focusedElementsAreRichlyEditable();
    virtual void ignoreWordInSpellDocument(const String&);
    virtual void learnWord(const String&);
    virtual void checkSpellingOfString(const UChar*, int, int*, int*);
    virtual String getAutoCorrectSuggestionForMisspelledWord(const String& misspelledWord);
    virtual void checkGrammarOfString(const UChar*, int, WTF::Vector<GrammarDetail, 0u>&, int*, int*);
    virtual void updateSpellingUIWithGrammarString(const String&, const GrammarDetail&);
    virtual void updateSpellingUIWithMisspelledWord(const String&);
    virtual void showSpellingUI(bool);
    virtual bool spellingUIIsShowing();
    virtual void getGuessesForWord(const String&, WTF::Vector<String, 0u>&);
    virtual void setInputMethodState(bool);

private:
    Olympia::WebKit::WebPage* m_page;
    bool m_waitingForCursorFocus;

    bool m_inRedo;

    typedef Deque<RefPtr<WebCore::EditCommand> > EditCommandStack;
    EditCommandStack m_undoStack;
    EditCommandStack m_redoStack;
};

} // WebCore

#endif // EditorClientOlympia_h
