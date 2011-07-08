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

namespace WebCore {
class Element;
class Frame;
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

    void nodeFocused(WebCore::Node*);
    void nodeTextChanged(const WebCore::Node*);
    void selectionChanged();
    void frameUnloaded(WebCore::Frame*);

    Olympia::Platform::ReplaceTextErrorCode replaceText(const Olympia::Platform::ReplaceArguments& replaceArguments, const Olympia::Platform::AttributedText& attributedText);
    bool handleKeyboardInput(WebCore::PlatformKeyboardEvent::Type, const unsigned short character, bool shiftDown);
    void handleNavigationMove(const unsigned short character, bool shiftDown, bool altDown);
    void requestElementText(int requestedFrameId, int requestedElementId, int offset, int length);

    void setCaretPosition(int requestedFrameId, int requestedElementId, int caretPosition);
    void cancelSelection();

    Olympia::Platform::IntRect rectForCaret(int index);

    bool selectionAtStartOfElement();
    bool selectionAtEndOfElement();
    bool isInputMode() { return m_currentFocusElement; }
    bool isProcessingChange() { return m_processingChange; }

    void setNavigationMode(bool active, bool sendMessage = true);

    void resetAttributedText();

    bool canAddCommandToUndoStack() { return m_addCommandToUndoStack; }

    void ensureFocusElementVisible();

private:
    void setElementFocused(WebCore::Element*);
    WebCore::Element* elementFromId(unsigned int);
    unsigned int idFromElement(const WebCore::Element*);
    WebCore::Frame* frameFromId(unsigned int);
    unsigned int idFromFrame(WebCore::Frame*);

    void addElementToFrameSet(WebCore::Element*);
    void cleanFrameSet(WebCore::Frame*);

    bool validElement(WebCore::Frame*, const WebCore::Element*);

    WebCore::String elementText(WebCore::Element*);
    Olympia::Platform::OlympiaInputType elementType(const WebCore::Element*);

    unsigned int selectionStart();
    unsigned int selectionEnd();
    void setSelection(unsigned int selectionStart, unsigned int selectionEnd);

    Olympia::Platform::ReplaceTextErrorCode processReplaceText(const Olympia::Platform::ReplaceArguments& replaceArguments, const Olympia::Platform::AttributedText& attributedText);

    bool shouldToggleAttribute(bool currentValue, Olympia::Platform::AttributeValue desiredValue);
    bool shouldToggleUnderline(bool currentValue, Olympia::Platform::UnderlineStyles desiredValue);

    void cacheTypingStyle(unsigned int start, unsigned int end);
    void clearCachedTypingStyle();

    void applyPasswordFieldUnmaskSecure(const WebCore::Element* element, int startUnmask, int lengthUnmask);

    void notifyTextChanged(const WebCore::Element*);
    bool confirmReplacementValue(const WebCore::String&);

    void syncWatchdogFired(WebCore::Timer<InputHandler>*);
    void syncAckReceived();
    void syncMessageSent();
    bool allowSyncInput();

    WebPagePrivate* m_webPage;

    RefPtr<WebCore::Element> m_currentFocusElement;
    WTF::HashMap<WebCore::Frame*, ElementHash> m_frameElements;

    bool m_processingChange;
    bool m_addCommandToUndoStack;
    bool m_navigationMode;
    int m_numberOfOutstandingSyncMessages;

    RefPtr<WebCore::CSSMutableStyleDeclaration> m_cachedTypingStyle;
    int m_cachedTypingStyleStart;
    int m_cachedTypingStyleEnd;
    WebCore::Timer<InputHandler>* m_syncWatchdogTimer;
};

}
}

#endif // InputHandler_h
