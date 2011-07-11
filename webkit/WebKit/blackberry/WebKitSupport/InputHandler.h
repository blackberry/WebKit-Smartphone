/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef InputHandler_h
#define InputHandler_h

#include "CSSMutableStyleDeclaration.h"
#include "HTMLInputElement.h"
#include "OlympiaPlatformInputEvents.h"
#include "OlympiaPlatformPrimitives.h"
#include "OlympiaPlatformReplaceText.h"
#include "PlatformKeyboardEvent.h"
#include "Timer.h"

#include <wtf/HashMap.h>
#include <wtf/RefPtr.h>

namespace WTF {
class String;
}

namespace WebCore {
class Element;
class Frame;
class HTMLSelectElement;
class Node;
}

namespace Olympia {
namespace WebKit {

class WebPagePrivate;
typedef WTF::HashSet<RefPtr<WebCore::Element> > ElementHash;

class InputHandler {
public:
    InputHandler(WebPagePrivate*);
    ~InputHandler();

    enum FocusElementType { TextEdit, TextPopup /*Date/Time & Color*/, SelectPopup };

    void nodeFocused(WebCore::Node*);
    void nodeTextChanged(const WebCore::Node*);
    void selectionChanged();
    void frameUnloaded(WebCore::Frame*);

    Olympia::Platform::ReplaceTextErrorCode replaceText(const Olympia::Platform::ReplaceArguments& replaceArguments, const Olympia::Platform::AttributedText& attributedText);
    bool handleKeyboardInput(WebCore::PlatformKeyboardEvent::Type, const unsigned short character, bool shiftDown = false, bool altDown = false);
    void handleNavigationMove(const unsigned short character, bool shiftDown, bool altDown);
    void requestElementText(int requestedFrameId, int requestedElementId, int offset, int length);

    void setCaretPosition(int requestedFrameId, int requestedElementId, int caretPosition);
    void cancelSelection();

    void setInputValue(const WTF::String&);

    Olympia::Platform::IntRect rectForCaret(int index);

    bool selectionAtStartOfElement();
    bool selectionAtEndOfElement();
    bool isInputMode() { return isActiveTextEdit(); }

    void setNavigationMode(bool active, bool sendMessage = true);

    void ensureFocusElementVisible();
    void handleInputLocaleChanged(bool isRTL);

    /* PopupMenu Methods */
    bool didNodeOpenPopup(WebCore::Node*);
    bool openLineInputPopup(WebCore::HTMLInputElement*);
    bool openSelectPopup(WebCore::HTMLSelectElement*);
    void setPopupListIndex(int index);
    void setPopupListIndexes(int size, bool* selecteds);

    bool processingChange() { return m_processingChange; }
    void setProcessingChange(bool processingChange) { m_processingChange = processingChange; }

private:
    void setElementFocused(WebCore::Element*);
    WebCore::Element* elementFromId(unsigned int);
    unsigned int idFromElement(const WebCore::Element*);
    WebCore::Frame* frameFromId(unsigned int);
    unsigned int idFromFrame(WebCore::Frame*);

    void clearCurrentFocusElement();

    bool isActiveTextEdit() { return m_currentFocusElement && m_currentFocusElementType == TextEdit; }
    bool isActiveTextPopup() { return m_currentFocusElement && m_currentFocusElementType == TextPopup; }
    bool isActiveSelectPopup() { return m_currentFocusElement && m_currentFocusElementType == SelectPopup; }

    void dateElementFocused(WebCore::HTMLInputElement*, Olympia::Platform::OlympiaInputType);
    void colorElementFocused(WebCore::HTMLInputElement*);

    void addElementToFrameSet(WebCore::Element*);
    void cleanFrameSet(WebCore::Frame*);

    bool validElement(WebCore::Frame*, const WebCore::Element*);

    WTF::String elementText(WebCore::Element*);
    Olympia::Platform::OlympiaInputType elementType(const WebCore::Element*);

    unsigned int selectionStart();
    unsigned int selectionEnd();
    void setSelection(unsigned int selectionStart, unsigned int selectionEnd);
    RefPtr<WebCore::Range> subrangeForFocusElement(unsigned int offsetStart, unsigned int offsetLength);
    RefPtr<WebCore::Range> focusElementRange(unsigned int offsetStart, unsigned int offsetLength);

    Olympia::Platform::ReplaceTextErrorCode processReplaceText(const Olympia::Platform::ReplaceArguments& replaceArguments, const Olympia::Platform::AttributedText& attributedText);

    void addAttributedTextMarker(unsigned int startPosition, unsigned int length, uint64_t attributes);
    void removeAttributedTextMarker();

    void applyPasswordFieldUnmaskSecure(const WebCore::Element* element, int startUnmask, int lengthUnmask);

    void notifyTextChanged(const WebCore::Element*);
    bool setCursorPositionIfInputTextValidated(const WTF::String&, unsigned int desiredCursorPosition);

    void syncWatchdogFired(WebCore::Timer<InputHandler>*);
    void syncAckReceived();
    void syncMessageSent();
    bool allowSyncInput();

    WebPagePrivate* m_webPage;

    RefPtr<WebCore::Element> m_currentFocusElement;
    WTF::HashMap<WebCore::Frame*, ElementHash> m_frameElements;

    bool m_processingChange;
    bool m_navigationMode;
    int m_numberOfOutstandingSyncMessages;
    FocusElementType m_currentFocusElementType;

    WebCore::Timer<InputHandler>* m_syncWatchdogTimer;
};

}
}

#endif // InputHandler_h
