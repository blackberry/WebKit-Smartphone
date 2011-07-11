/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "InputHandler.h"

#include "RenderLayer.h"
#include "CSSStyleDeclaration.h"
#include "CString.h"
#include "Chrome.h"
#include "Document.h"
#include "FocusController.h"
#include "Frame.h"
#include "FrameView.h"
#include "HTMLInputElement.h"
#include "HTMLOptGroupElement.h"
#include "HTMLOptionElement.h"
#include "HTMLNames.h"
#include "HTMLSelectElement.h"
#include "HTMLTextAreaElement.h"
#include "OlympiaPlatformKeyboardCodes.h"
#include "OlympiaPlatformKeyboardEvent.h"
#include "OlympiaPlatformMisc.h"
#include "Page.h"
#include "PlatformKeyboardEvent.h"
#include "Range.h"
#include "RenderMenuList.h"
#include "RenderText.h"
#include "RenderTextControl.h"
#include "ScopePointer.h"
#include "SelectionHandler.h"
#include "TextIterator.h"
#include "WebPageClient.h"
#include "WebPage_p.h"
#include "WebSettings.h"

#define SHOWDEBUG_INPUTHANDLER 0
#define ENABLE_DEBUG_UNDOREDO 0
#define SHOWDEBUG_SYNCHANDLING 0

using namespace Olympia::Platform;
using namespace WebCore;

namespace Olympia {
namespace WebKit {

class ProcessingChangeGuard
{
public:
    ProcessingChangeGuard(InputHandler* inputHandler)
        : m_inputHandler(inputHandler)
        , m_savedProcessingChange(false)
    {
        ASSERT(m_inputHandler);

        m_savedProcessingChange = m_inputHandler->processingChange();
        m_inputHandler->setProcessingChange(true);
    }

    ~ProcessingChangeGuard()
    {
        m_inputHandler->setProcessingChange(m_savedProcessingChange);
    }

private:
    InputHandler* m_inputHandler;
    bool m_savedProcessingChange;
};

InputHandler::InputHandler(WebPagePrivate* page)
    : m_webPage(page)
    , m_currentFocusElement(0)
    , m_processingChange(false)
    , m_navigationMode(false)
    , m_numberOfOutstandingSyncMessages(0)
    , m_currentFocusElementType(TextEdit)
    , m_syncWatchdogTimer(new Timer<InputHandler>(this, &InputHandler::syncWatchdogFired))
{
}

InputHandler::~InputHandler()
{
    delete m_syncWatchdogTimer;
    m_syncWatchdogTimer = 0;
}

void InputHandler::syncWatchdogFired(WebCore::Timer<InputHandler>*)
{
    // Reset sync tracking.
    if (m_numberOfOutstandingSyncMessages && isActiveTextEdit()) {
#if SHOWDEBUG_SYNCHANDLING
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::syncWatchdogFired Resetting");
#endif
        m_numberOfOutstandingSyncMessages = 0;
        // notifyTextChange is a sync message and will re-increment the value continuing
        // to prevent input until client responds.
        notifyTextChanged(m_currentFocusElement.get());
    }
}

void InputHandler::syncAckReceived()
{
    if (!m_numberOfOutstandingSyncMessages) {
#if SHOWDEBUG_SYNCHANDLING
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::syncAckReceived unexpected");

        // An unexpected ack was received.  Sender should be investigated.
        ASSERT_NOT_REACHED();
#endif
        return;
    }
    m_numberOfOutstandingSyncMessages--;
#if SHOWDEBUG_SYNCHANDLING
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::syncAckReceived current m_numberOfOutstandingSyncMessages = %d", m_numberOfOutstandingSyncMessages);
#endif
    if (!m_numberOfOutstandingSyncMessages)
        m_syncWatchdogTimer->stop();
}

void InputHandler::syncMessageSent()
{
    m_numberOfOutstandingSyncMessages++;
    m_syncWatchdogTimer->startOneShot(1.0); // Timeout after 1 second if no response is received.
}

bool InputHandler::allowSyncInput()
{
    return !m_numberOfOutstandingSyncMessages;
}

static OlympiaInputType convertInputType(HTMLInputElement::InputType type)
{
    switch (type) {
    case HTMLInputElement::PASSWORD:
        return InputTypePassword;
    case HTMLInputElement::ISINDEX:
        return InputTypeIsIndex;
    case HTMLInputElement::SEARCH:
        return InputTypeSearch;
    case HTMLInputElement::EMAIL:
        return InputTypeEmail;
    case HTMLInputElement::NUMBER:
        return InputTypeNumber;
    case HTMLInputElement::TELEPHONE:
        return InputTypeTelephone;
    case HTMLInputElement::URL:
        return InputTypeURL;
    case HTMLInputElement::COLOR:
        return InputTypeColor;
    case HTMLInputElement::DATE:
        return InputTypeDate;
    case HTMLInputElement::DATETIME:
        return InputTypeDateTime;
    case HTMLInputElement::DATETIMELOCAL:
        return InputTypeDateTimeLocal;
    case HTMLInputElement::MONTH:
        return InputTypeMonth;
    case HTMLInputElement::TIME:
        return InputTypeTime;
    case HTMLInputElement::WEEK:
        // FIXME: missing WEEK popup selector
    case HTMLInputElement::TEXT:
    default:
        return InputTypeText;
    }
}

WTF::String InputHandler::elementText(Element* element)
{
    // Note:  requesting the value from some input fields can result in an update
    // when it updates it's internal value from the renderer.
    ProcessingChangeGuard guard(this);
    WTF::String elementText;
    if (element->hasTagName(HTMLNames::inputTag)) {
        const HTMLInputElement* inputElement = static_cast<const HTMLInputElement*>(element);
        elementText = inputElement->value();
    } else if (element->hasTagName(HTMLNames::textareaTag)) {
        const HTMLTextAreaElement* inputElement = static_cast<const HTMLTextAreaElement*>(element);
        elementText = inputElement->value();
    } else {
        RefPtr<Range> rangeForNode = rangeOfContents(element);
        elementText = rangeForNode.get()->text();
    }
    return elementText;
}

OlympiaInputType InputHandler::elementType(const Element* element)
{
    if (element->hasTagName(HTMLNames::inputTag)) {
        const HTMLInputElement* inputElement = static_cast<const HTMLInputElement*>(element);
        return convertInputType(inputElement->inputType());
    }
    if (element->hasTagName(HTMLNames::textareaTag))
        return InputTypeTextArea;

    // Default to InputTypeTextArea for content editable fields.
    return InputTypeTextArea;
}

static inline HTMLTextFormControlElement* toTextControlElement(WebCore::Node* node)
{
    if (!(node && node->isElementNode()))
        return 0;

    Element* element = static_cast<Element*>(node);
    if (!element->isFormControlElement())
        return 0;

    HTMLFormControlElement* formElement = static_cast<HTMLFormControlElement*>(element);
    if (!formElement->isTextFormControl())
        return 0;

    return static_cast<HTMLTextFormControlElement*>(formElement);
}

// Check if the first non-container node in a content editable node is a text node.
static bool isTextBasedContentEditableElement(const Element* element)
{
    if (!element)
        return false;

    bool isText = false;
    if (element->isContentEditable()) {
        const Element* subElement = element;
        isText = subElement->isTextNode();
        while (!isText && subElement->isContainerNode() && subElement->hasChildNodes()) {
            subElement = static_cast<const Element*>(subElement->firstChild());
            isText = subElement->isTextNode();
        }
    }
    return isText;
}

void InputHandler::nodeFocused(Node* node)
{
    if (isActiveTextEdit() && m_currentFocusElement == node) {
        setNavigationMode(true);
        return;
    }

    if (node && node->isElementNode()) {
        Element* element = static_cast<Element*>(node);
        if ((element->hasTagName(HTMLNames::inputTag) && element->isTextFormControl())
            || element->hasTagName(HTMLNames::textareaTag)
            || element->isContentEditable()) {
            // Focus node is a text based input field, textarea or content editable field.
            setElementFocused(element);
            return;
        }
    }

    if (isActiveTextEdit() && m_currentFocusElement->isContentEditable()) {
        // This is a special handler for content editable fields.  The focus node is the top most
        // field that is content editable, however, by enabling / disabling designmode and
        // content editable, it is possible through javascript or selection to trigger the focus node to
        // change even while maintaining editing on the same selection point.  If the focus element
        // isn't contentEditable, but the current selection is, don't send a change notification.

        // When processing changes selection changes occur that may reset the focus element
        // and could potentially cause a crash as m_currentFocusElement should not be
        // changed during processing of an EditorCommand.
        if (processingChange())
            return;

        Frame* frame = m_currentFocusElement->document()->frame();
        ASSERT(frame);

        // Test the current selection to make sure that the content in focus is still content
        // editable.  This may mean Javascript triggered a focus change by modifying the
        // top level parent of this object's content editable state without actually modifying
        // this particular object.
        // Example site: html5demos.com/contentEditable - blur event triggers focus change.
        if (frame->selection()->start().anchorNode()
            && frame->selection()->start().anchorNode()->isContentEditable())
                return;
    }

    // Not a text input field.
    setElementFocused(0);
}

void InputHandler::setElementFocused(Element* element)
{
    if (isActiveTextEdit()) {
        unsigned int oldFrameId = m_currentFocusElement->document() ? idFromFrame(m_currentFocusElement->document()->frame()) : 0;
        unsigned int oldFocusId = idFromElement(m_currentFocusElement.get());

        if (elementType(m_currentFocusElement.get()) == InputTypePassword)
            applyPasswordFieldUnmaskSecure(m_currentFocusElement.get(), 0, 0);

#if SHOWDEBUG_INPUTHANDLER
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::setElementFocused unfocusing frameId=%u, FocusId=%u", oldFrameId, oldFocusId);
#endif
        m_webPage->m_client->inputFocusLost(oldFrameId, oldFocusId);
        setNavigationMode(false, false);
        if (oldFrameId)
            removeAttributedTextMarker();
        m_numberOfOutstandingSyncMessages = 0;
        m_syncWatchdogTimer->stop();
        m_currentFocusElement = 0;
    } else if (!element) {
        // Focus is being cleared and we are a popup field.  We can't hide the popup
        // but we need to invalidate the state.
        m_currentFocusElement = 0;
        m_currentFocusElementType = TextEdit;
    }

    if (!element || element->isReadOnlyFormControl())
        return;

    ASSERT(element->document() && element->document()->frame());

    OlympiaInputType type = elementType(element);
    if (type == InputTypeDate || type == InputTypeDateTime  || type == InputTypeDateTimeLocal
        || type == InputTypeMonth  || type == InputTypeTime || type == InputTypeColor) {

        EventHandler* eventHandler = m_webPage->mainFrame()->eventHandler();
        if (!eventHandler->mousePressed()) {
            // Only trigger popup if mouse is pressed.  Auto focus / link to link
            // should not trigger the popup.
            return;
        }

        if (isActiveTextPopup()) {
            // If we already have a text popup active, disregard this event.  This can be triggered
            // by the selectionChanged message if there is already text in the field when it is
            // clicked.
            return;
        }

        HTMLInputElement* inputElement = static_cast<HTMLInputElement*>(element);
        m_currentFocusElementType = TextPopup;
        m_currentFocusElement = element;
        if (type == InputTypeDate || type == InputTypeDateTime  || type == InputTypeDateTimeLocal
            || type == InputTypeMonth  || type == InputTypeTime)
            dateElementFocused(inputElement, type);
        else
            colorElementFocused(inputElement);
        return;
    }
    m_currentFocusElementType = TextEdit;

    if (!isTextBasedContentEditableElement(element) && !element->isTextFormControl())
        return;

    addElementToFrameSet(element);
    unsigned int currentFrameId = idFromFrame(element->document()->frame());
    unsigned int currentFocusId = idFromElement(element);
    unsigned int length = elementText(element).length();

    // Always send selection as zero since cursor placement doesn't occur until after
    // focus is given.
    unsigned int selectionStart = 0;
    unsigned int selectionEnd = 0;

#if SHOWDEBUG_INPUTHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::setElementFocused frameId=%u, FocusId=%u, Type=%d, length=%u, selectionStart=%u, selectionEnd=%u",
                currentFrameId, currentFocusId, type, length, selectionStart, selectionEnd);
#endif
    m_webPage->m_client->inputFocusGained(currentFrameId, currentFocusId, type, length, selectionStart, selectionEnd);
    m_currentFocusElement = element;
    setNavigationMode(true, m_webPage->m_mainFrame->eventHandler()->mousePressed() /*sendMessage*/);
    handleInputLocaleChanged(m_webPage->m_webSettings->isWritingDirectionRTL());
}

Element* InputHandler::elementFromId(unsigned int id)
{
    return reinterpret_cast<Element*>(id);
}

unsigned int InputHandler::idFromElement(const Element* element)
{
    return reinterpret_cast<unsigned int>(element);
}

Frame* InputHandler::frameFromId(unsigned int id)
{
    return reinterpret_cast<Frame*>(id);
}

unsigned int InputHandler::idFromFrame(Frame* frame)
{
    return reinterpret_cast<unsigned int>(frame);
}

void InputHandler::addElementToFrameSet(Element* element)
{
    ASSERT(element);
    ASSERT(element->document() && element->document()->frame());

    Frame* frame = element->document()->frame();
    // FIXME: is this slow?
    if (!m_frameElements.contains(frame)) {
        ElementHash hashSet;
        hashSet.add(element);
        m_frameElements.set(frame, hashSet);
    } else if (!m_frameElements.get(frame).contains(element)) {
        ElementHash hashSet = m_frameElements.get(frame);
        hashSet.add(element);
        m_frameElements.set(frame, hashSet);
    }
}

bool InputHandler::validElement(Frame* frame, const Element* element)
{
    ASSERT(frame);
    ASSERT(element);
    return (m_frameElements.contains(frame) && m_frameElements.get(frame).contains(const_cast<Element*>(element)));
}

void InputHandler::dateElementFocused(HTMLInputElement* element, OlympiaInputType type)
{
    WTF::String value = element->value();
    WTF::String min = element->getAttribute(HTMLNames::minAttr).string();
    WTF::String max = element->getAttribute(HTMLNames::maxAttr).string();
    double step = element->getAttribute(HTMLNames::stepAttr).toDouble();
    m_webPage->m_client->openDateTimePopup(type, value, min, max, step);
}

void InputHandler::colorElementFocused(HTMLInputElement* element)
{
    m_webPage->m_client->openColorPopup(element->value());
}

void InputHandler::setInputValue(const WTF::String& value)
{
    if (!isActiveTextPopup())
        return;

    HTMLInputElement* inputElement = static_cast<HTMLInputElement*>(m_currentFocusElement.get());
    inputElement->setValue(value);
    clearCurrentFocusElement();
}

void InputHandler::nodeTextChanged(const Node* node)
{
    if (processingChange() || !node)
        return;

    if (node->isElementNode())
        notifyTextChanged(static_cast<const Element*>(node));
}

void InputHandler::ensureFocusElementVisible()
{
    if (!m_currentFocusElement || !m_navigationMode)
        return;

    Document* document = m_currentFocusElement->document();
    if (!document)
        return;

    Frame* frame = document->frame();
    if (!frame)
        return;

    FrameView* frameView = frame->view();
    if (!frameView)
        return;

    WebCore::IntRect selectionFocusRect;
    switch (frame->selection()->selectionType()) {
    case VisibleSelection::CaretSelection:
        selectionFocusRect = frame->selection()->absoluteCaretBounds();
        break;
    case VisibleSelection::RangeSelection:
        selectionFocusRect = VisiblePosition(frame->selection()->start()).absoluteCaretBounds();
        break;
    case VisibleSelection::NoSelection:
    default:
        return;
    }

    WebCore::IntRect actualScreenRect = WebCore::IntRect(frameView->scrollPosition(), m_webPage->actualVisibleSize());
    if (actualScreenRect.contains(selectionFocusRect))
        return;

    Position start = frame->selection()->start();
    if (start.node() && start.node()->renderer()) {
        if (RenderLayer* layer = start.node()->renderer()->enclosingLayer()) {

            // Align the selection rect if possible so that we show the field's
            // outline if the caret is at the edge of the field
            ScrollAlignment horizontalScrollAlignment = ScrollAlignment::alignCenterIfNeeded;
            if (RenderObject* focusedRenderer = m_currentFocusElement->renderer()) {
                WebCore::IntRect nodeOutlineBounds = focusedRenderer->absoluteOutlineBounds();
                WebCore::IntRect caretAtEdgeRect = rectForCaret(0);
                int paddingX = abs(caretAtEdgeRect.x() - nodeOutlineBounds.x());
                int paddingY = abs(caretAtEdgeRect.y() - nodeOutlineBounds.y());

                if (selectionFocusRect.x() - paddingX == nodeOutlineBounds.x())
                    selectionFocusRect.setX(nodeOutlineBounds.x());
                else if (selectionFocusRect.right() + paddingX == nodeOutlineBounds.right())
                    selectionFocusRect.setRight(nodeOutlineBounds.right());
                if (selectionFocusRect.y() - paddingY == nodeOutlineBounds.y())
                    selectionFocusRect.setY(nodeOutlineBounds.y());
                else if (selectionFocusRect.bottom() + paddingY == nodeOutlineBounds.bottom())
                    selectionFocusRect.setBottom(nodeOutlineBounds.bottom());

                // If the editing point is on the left hand side of the screen when the node's
                // rect is edge aligned, edge align the node rect.
                if (selectionFocusRect.x() - caretAtEdgeRect.x() < actualScreenRect.width() / 2) {
                    selectionFocusRect.setX(nodeOutlineBounds.x());
                    horizontalScrollAlignment = ScrollAlignment::alignToEdgeIfNeeded;
                }
            }

            WebCore::IntRect revealRect = layer->getRectToExpose(actualScreenRect, selectionFocusRect,
                                                                 horizontalScrollAlignment,
                                                                 ScrollAlignment::alignCenterIfNeeded);

            frameView->setScrollPosition(revealRect.location());
        }
    }
}

void InputHandler::notifyTextChanged(const Element* element)
{
    if (!element)
        return;

    ASSERT(element->document());
    Frame* frame = element->document()->frame();

    // If the function is called by timer, the document may have been detached from the frame.
    if (!frame)
        return;

    if (validElement(frame, element)) {
        unsigned int focusId = idFromElement(element);
        unsigned int currentFrameId = idFromFrame(frame);
        Element* activeElement = const_cast<Element*>(element);
        WTF::String currentText = elementText(activeElement);

        int selectionStartOffset = -1;
        int selectionEndOffset = -1;
        // Fixme.  Selection might work out of focus?
        if (activeElement == m_currentFocusElement) {
            selectionStartOffset = selectionStart();
            selectionEndOffset = selectionStart();
        }

#if SHOWDEBUG_INPUTHANDLER
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::notifyTextChanged frameId=%u, FocusId=%u, currentText=%s, length=%u, selectionStart=%u, selectionEnd=%u",
                    currentFrameId, focusId, currentText.latin1().data(), currentText.length(), selectionStartOffset, selectionEndOffset);
#endif
        m_webPage->m_client->inputTextChanged(currentFrameId, focusId, currentText.characters(), currentText.length(), selectionStartOffset, selectionEndOffset);
#if SHOWDEBUG_SYNCHANDLING
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::notifyTextChanged syncMessage sent current m_numberOfOutstandingSyncMessages = %d", m_numberOfOutstandingSyncMessages);
#endif
        syncMessageSent();
    }
}

void InputHandler::requestElementText(int requestedFrameId, int requestedElementId, int offset, int length)
{
    // Offset -1 means this is an ack notification of a change.
    if (offset == -1) {
        if (isActiveTextEdit() && m_currentFocusElement == elementFromId(requestedElementId))
            syncAckReceived();
        return;
    }

    if (validElement(frameFromId(requestedFrameId), elementFromId(requestedElementId))) {
        Element* element = elementFromId(requestedElementId);

        ASSERT(element && element->document());
        if (offset < 0)
            offset = 0;

        WTF::String currentText = elementText(element);
        if (length == -1 || length > (currentText.length() - offset))
            length = currentText.length() - offset;
        currentText = currentText.substring(offset, length);

        int selectionStartOffset = -1;
        int selectionEndOffset = -1;
        if (element == m_currentFocusElement) {
            selectionStartOffset = selectionStart();
            selectionEndOffset = selectionStart();
        }

#if SHOWDEBUG_INPUTHANDLER
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::requestElementText frameId=%u, FocusId=%u, currentText=%s, length=%u, selectionStart=%u, selectionEnd=%u",
                    requestedFrameId, requestedElementId, currentText.latin1().data(), currentText.length(), selectionStartOffset, selectionEndOffset);
#endif
        m_webPage->m_client->inputTextForElement(requestedFrameId, requestedElementId, offset, length, selectionStartOffset, selectionEndOffset, currentText.characters(), length);
#if SHOWDEBUG_SYNCHANDLING
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::requestElementText syncMessage sent current m_numberOfOutstandingSyncMessages = %d", m_numberOfOutstandingSyncMessages);
#endif
        syncMessageSent();
    } else {
        // Invalid Request.
        length = -1;
#if SHOWDEBUG_INPUTHANDLER
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::requestElementText frameId=%u, FocusId=%u, invalid element", requestedFrameId, requestedElementId);
#endif
        m_webPage->m_client->inputTextForElement(requestedFrameId, requestedElementId, offset, length, -1, -1, WTF::String().characters(), 0);
#if SHOWDEBUG_SYNCHANDLING
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::requestElementText syncMessage sent current m_numberOfOutstandingSyncMessages = %d", m_numberOfOutstandingSyncMessages);
#endif
        syncMessageSent();
    }
}

void InputHandler::cleanFrameSet(Frame* frame)
{
    m_frameElements.remove(frame);
}

void InputHandler::frameUnloaded(Frame *frame)
{
    unsigned int currentFrameId = idFromFrame(frame);
    cleanFrameSet(frame);
    setNavigationMode(false, false);
#if SHOWDEBUG_INPUTHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::frameUnloaded frameId=%u", currentFrameId);
#endif
    m_webPage->m_client->inputFrameCleared(currentFrameId);
}

void InputHandler::setNavigationMode(bool active, bool sendMessage)
{
    if (active && !isActiveTextEdit()) {
        if (!m_navigationMode)
            return;

        // We can't be active if there is no element.  Send out notification that we
        // aren't in navigation mode.
        active = false;
    }

    // Don't send the change if it's setting the event setting navigationMode true
    // if we are already in navigation mode and this is a navigation move event.
    // We need to send the event when it's triggered by a touch event or mouse
    // event to allow display of the VKB, but do not want to send it when it's
    // triggered by a navigation event as it has no effect.
    // Touch events are simulated as mouse events so mousePressed will be active
    // when it is a re-entry event.
    // See RIM Bugs #369 & #878.
    if (active && active == m_navigationMode && !m_webPage->m_mainFrame->eventHandler()->mousePressed())
        return;

    m_navigationMode = active;

    if (sendMessage)
        m_webPage->m_client->inputSetNavigationMode(active);

#if SHOWDEBUG_INPUTHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::setNavigationMode %s, %s", active ? "true" : "false", sendMessage ? "message sent" : "message not sent");
#endif
}

bool InputHandler::selectionAtStartOfElement()
{
    if (!isActiveTextEdit())
        return false;

    ASSERT(m_currentFocusElement->document() && m_currentFocusElement->document()->frame());

    if (selectionStart() == 0)
        return true;

    return false;
}

bool InputHandler::selectionAtEndOfElement()
{
    if (!isActiveTextEdit())
        return false;

    ASSERT(m_currentFocusElement->document() && m_currentFocusElement->document()->frame());

    if (selectionStart() == elementText(m_currentFocusElement.get()).length())
        return true;

    return false;
}

unsigned int InputHandler::selectionStart()
{
    if (!m_currentFocusElement->document() || !m_currentFocusElement->document()->frame())
        return 0;

    HTMLTextFormControlElement* controlElement = toTextControlElement(m_currentFocusElement.get());
    if (controlElement)
        return controlElement->selectionStart();

    SelectionController caretSelection;
    caretSelection.setSelection(m_currentFocusElement->document()->frame()->selection()->selection());
    RefPtr<Range> rangeSelection = caretSelection.selection().toNormalizedRange();
    if (rangeSelection) {
        unsigned int selectionStartInNode = rangeSelection->startOffset();
        Node* startContainerNode = rangeSelection->startContainer();

        ExceptionCode ec;
        RefPtr<Range> rangeForNode = rangeOfContents(m_currentFocusElement.get());
        rangeForNode->setEnd(startContainerNode, selectionStartInNode, ec);
        ASSERT(!ec);

        return TextIterator::rangeLength(rangeForNode.get());
    }
    return 0;
}

unsigned int InputHandler::selectionEnd()
{
    if (!m_currentFocusElement->document() || !m_currentFocusElement->document()->frame())
        return 0;

    HTMLTextFormControlElement* controlElement = toTextControlElement(m_currentFocusElement.get());
    if (controlElement)
        return controlElement->selectionEnd();

    SelectionController caretSelection;
    caretSelection.setSelection(m_currentFocusElement->document()->frame()->selection()->selection());
    RefPtr<Range> rangeSelection = caretSelection.selection().toNormalizedRange();
    if (rangeSelection) {
        unsigned int selectionEndInNode = rangeSelection->endOffset();
        Node* endContainerNode = rangeSelection->endContainer();

        ExceptionCode ec;
        RefPtr<Range> rangeForNode = rangeOfContents(m_currentFocusElement.get());
        rangeForNode->setEnd(endContainerNode, selectionEndInNode, ec);
        ASSERT(!ec);

        return TextIterator::rangeLength(rangeForNode.get());
    }
    return 0;
}

void InputHandler::selectionChanged()
{
    if (processingChange() || !isActiveTextEdit())
        return;

    if (!m_currentFocusElement->document() || !m_currentFocusElement->document()->frame())
        return;

    unsigned int currentFrameId = idFromFrame(m_currentFocusElement->document()->frame());
    unsigned int currentFocusId = idFromElement(m_currentFocusElement.get());

    unsigned int selectionStartOffset = selectionStart();
    unsigned int selectionEndOffset = selectionEnd();

#if SHOWDEBUG_INPUTHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::selectionChanged frameId=%u, FocusId=%u, selectionStart=%u, selectionEnd=%u",
                currentFrameId, currentFocusId, selectionStartOffset, selectionEndOffset);
#endif
    m_webPage->m_client->inputSelectionChanged(currentFrameId, currentFocusId, selectionStartOffset, selectionEndOffset);
#if SHOWDEBUG_SYNCHANDLING
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::selectionChanged syncMessage sent current m_numberOfOutstandingSyncMessages = %d", m_numberOfOutstandingSyncMessages);
#endif
    syncMessageSent();
}

RefPtr<Range> InputHandler::subrangeForFocusElement(unsigned int offsetStart, unsigned int offsetLength)
{
    // NOTE:  Function should only be used for non-HTMLTextFormControlElement as rangeOfContents
    // is empty for those elements.
    ASSERT(m_currentFocusElement);

    RefPtr<Range> rangeForNode = rangeOfContents(m_currentFocusElement.get());
    return TextIterator::subrange(rangeForNode.get(), offsetStart, offsetLength);
}

RefPtr<Range> InputHandler::focusElementRange(unsigned int offsetStart, unsigned int offsetLength)
{
    if (!isActiveTextEdit())
        return 0;

    ASSERT(m_currentFocusElement->document() && m_currentFocusElement->document()->frame());

    unsigned int currentTextLength = elementText(m_currentFocusElement.get()).length();
    if (offsetLength < 1 || offsetStart + offsetLength > currentTextLength)
        return 0; // Invalid request.

    HTMLTextFormControlElement* controlElement = toTextControlElement(m_currentFocusElement.get());
    if (controlElement) {
        RenderTextControl* textRender = toRenderTextControl(controlElement->renderer());
        return VisibleSelection(textRender->visiblePositionForIndex(offsetStart), textRender->visiblePositionForIndex(offsetStart + offsetLength)).toNormalizedRange();
    }

    return subrangeForFocusElement(offsetStart, offsetLength);
}

void InputHandler::setSelection(unsigned int selectionStart, unsigned int selectionEnd)
{
    if (!isActiveTextEdit())
        return;

    ASSERT(m_currentFocusElement->document() && m_currentFocusElement->document()->frame());

    HTMLTextFormControlElement* controlElement = toTextControlElement(m_currentFocusElement.get());
    if (controlElement) {
        controlElement->setSelectionStart(selectionStart);
        controlElement->setSelectionEnd(selectionEnd);
    } else {
        RefPtr<Range> selectionRange = subrangeForFocusElement(selectionStart, selectionEnd - selectionStart);
        if (selectionStart == selectionEnd)
            m_currentFocusElement->document()->frame()->selection()->setSelection(VisibleSelection(selectionRange.get()->startPosition(), DOWNSTREAM));
        else
            m_currentFocusElement->document()->frame()->selection()->setSelectedRange(selectionRange.get(), DOWNSTREAM, false);
    }
#if SHOWDEBUG_INPUTHANDLER
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::setSelection selectionStart=%u, selectionEnd=%u", selectionStart, selectionEnd);
#endif
}

void InputHandler::setCaretPosition(int requestedFrameId, int requestedElementId, int caretPosition)
{
    if (!isActiveTextEdit() || caretPosition < 0 || !allowSyncInput())
        return;

    ASSERT(m_currentFocusElement->document() && m_currentFocusElement->document()->frame());

    if ((requestedFrameId == idFromFrame(m_currentFocusElement->document()->frame())) && (requestedElementId == idFromElement(m_currentFocusElement.get()))) {

#if SHOWDEBUG_INPUTHANDLER
        Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::setCaretPosition frameId=%u, FocusId=%u, caretPosition=%u", requestedFrameId, requestedElementId, caretPosition);
#endif
        ProcessingChangeGuard guard(this);
        setSelection(caretPosition, caretPosition);
    }
}
Olympia::Platform::IntRect InputHandler::rectForCaret(int index)
{
    if (!isActiveTextEdit())
        return Olympia::Platform::IntRect();

    ASSERT(m_currentFocusElement->document() && m_currentFocusElement->document()->frame());

    unsigned int currentTextLength = elementText(m_currentFocusElement.get()).length();
    if (index < 0 || index > currentTextLength)
        // Invalid request
        return Olympia::Platform::IntRect();

    VisiblePosition visiblePosition;
    SelectionController caretSelection;
    HTMLTextFormControlElement* controlElement = toTextControlElement(m_currentFocusElement.get());
    if (controlElement) {
        RenderTextControl* textRender = toRenderTextControl(controlElement->renderer());
        visiblePosition = textRender->visiblePositionForIndex(index);
        if (!controlElement->hasTagName(HTMLNames::textareaTag) && !textRender->softBreaksBeforeIndex(index))
            return visiblePosition.absoluteCaretBounds();

        caretSelection.setSelection(visiblePosition);
    } else {
        RefPtr<Range> selectionRange = subrangeForFocusElement(0, index);
        caretSelection.setSelection(VisibleSelection(selectionRange->endPosition(), DOWNSTREAM));
    }

    caretSelection.modify(SelectionController::AlterationExtend, SelectionController::DirectionForward, CharacterGranularity);
    visiblePosition = caretSelection.selection().visibleStart();
    return visiblePosition.absoluteCaretBounds();
}

void InputHandler::cancelSelection()
{
    if (!isActiveTextEdit())
        return;

    ASSERT(m_currentFocusElement->document() && m_currentFocusElement->document()->frame());

    unsigned int selectionStartPosition = selectionStart();
    ProcessingChangeGuard guard(this);
    setSelection(selectionStartPosition, selectionStartPosition);
}

void InputHandler::applyPasswordFieldUnmaskSecure(const Element* element, int startUnmask, int lengthUnmask)
{
    ASSERT(element);
    ASSERT(elementType(element) == InputTypePassword);

    if (!element->renderer())
        return;

    RenderTextControl* renderTextControl = toRenderTextControl(m_currentFocusElement->renderer());
    renderTextControl->setInnerTextUnmaskSecure(startUnmask, lengthUnmask);

#if SHOWDEBUG_INPUTHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::applyPasswordFieldUnmaskSecure start = %d, length = %d", startUnmask, lengthUnmask);
#endif
}

bool InputHandler::handleKeyboardInput(PlatformKeyboardEvent::Type type, unsigned short character, bool shiftDown, bool altDown)
{
#if SHOWDEBUG_INPUTHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::handleKeyboardInput received character=%lc, type=%d", character, type);
#endif

    ASSERT(m_webPage->m_page->focusController());
    bool keyboardEventHandled = false;
    if (Frame* focusedFrame = m_webPage->m_page->focusController()->focusedFrame()) {
        Olympia::Platform::KeyboardEvent keyboardEvent(character, type == PlatformKeyboardEvent::KeyDown || type == PlatformKeyboardEvent::Char, shiftDown || WTF::isASCIIUpper(character), altDown);
        keyboardEventHandled = focusedFrame->eventHandler()->keyEvent(PlatformKeyboardEvent(keyboardEvent));

        // Handle Char event as keyDown followed by KeyUp.
        if (type == PlatformKeyboardEvent::Char) {
            keyboardEvent.setKeyDown(false);
            keyboardEventHandled = focusedFrame->eventHandler()->keyEvent(PlatformKeyboardEvent(keyboardEvent)) || keyboardEventHandled;
        }
    }
    return keyboardEventHandled;
}

void InputHandler::handleNavigationMove(unsigned short character, bool shiftDown, bool altDown)
{
#if SHOWDEBUG_INPUTHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::handleNavigationMove received character=%lc", character);
#endif

    ASSERT(m_webPage->m_page->focusController());
    if (Frame* focusedFrame = m_webPage->m_page->focusController()->focusedFrame()) {
        switch (character) {
        case Olympia::Platform::KEY_CONTROL_LEFT:
            if (altDown) {
                // If alt is down, do not break out of the field.
                if (shiftDown)
                    focusedFrame->editor()->command("MoveToBeginningOfLineAndModifySelection").execute();
                else
                    focusedFrame->editor()->command("MoveToBeginningOfLine").execute();
            } else if (shiftDown) {
                focusedFrame->editor()->command("MoveBackwardAndModifySelection").execute();
            } else {
                // If we are at the extent of the edit box restore the cursor.
                if (selectionAtStartOfElement())
                    setNavigationMode(false);
                else
                    focusedFrame->editor()->command("MoveBackward").execute();
            }
            break;
        case Olympia::Platform::KEY_CONTROL_RIGHT:
            if (altDown) {
                // If alt is down, do not break out of the field.
                if (shiftDown)
                    focusedFrame->editor()->command("MoveToEndOfLineAndModifySelection").execute();
                else
                    focusedFrame->editor()->command("MoveToEndOfLine").execute();
            } else if (shiftDown) {
                focusedFrame->editor()->command("MoveForwardAndModifySelection").execute();
            } else {
                // If we are at the extent of the edit box restore the cursor.
                if (selectionAtEndOfElement())
                    setNavigationMode(false);
                else
                    focusedFrame->editor()->command("MoveForward").execute();
            }
            break;
        case Olympia::Platform::KEY_CONTROL_UP:
            if (altDown) {
                // If alt is down, do not break out of the field.
                if (shiftDown)
                    focusedFrame->editor()->command("MoveToBeginningOfDocumentAndModifySelection").execute();
                else
                    focusedFrame->editor()->command("MoveToBeginningOfDocument").execute();
            } else if (shiftDown) {
                focusedFrame->editor()->command("MoveUpAndModifySelection").execute();
            } else {
                // If we are at the extent of the edit box restore the cursor.
                if (selectionAtStartOfElement())
                    setNavigationMode(false);
                else
                    focusedFrame->editor()->command("MoveUp").execute();
            }
            break;
        case Olympia::Platform::KEY_CONTROL_DOWN:
            if (altDown) {
                // If alt is down, do not break out of the field.
                if (shiftDown)
                    focusedFrame->editor()->command("MoveToEndOfDocumentAndModifySelection").execute();
                else
                    focusedFrame->editor()->command("MoveToEndOfDocument").execute();
            } else if (shiftDown) {
                focusedFrame->editor()->command("MoveDownAndModifySelection").execute();
            } else {
                // If we are at the extent of the edit box restore the cursor.
                if (selectionAtEndOfElement())
                    setNavigationMode(false);
                else
                    focusedFrame->editor()->command("MoveDown").execute();
            }
            break;
        }
    }
}

static WebCore::FontWeight convertFontWeight(const FontWeights& weight)
{
    switch (weight) {
    case ELightWeight:
        return WebCore::FontWeight300;
    case EBoldWeight:
        return WebCore::FontWeightBold;
    case EExtraBoldWeight:
        return WebCore::FontWeight800;
    case ESuperBoldWeight:
        return WebCore::FontWeight900;
    case EDefaultWeight:
    case EMediumWeight:
    default:
        return WebCore::FontWeightNormal;
    }
}

static WebCore::AttributeTextStyle::AttributeStyleState convertAttributeValue(const AttributeValue& value)
{
    switch (value) {
    case AttributeNotApplied:
        return WebCore::AttributeTextStyle::AttributeNotApplied;
    case AttributeApplied:
        return WebCore::AttributeTextStyle::AttributeApplied;
    case AttributeNotProvided:
    default:
        return WebCore::AttributeTextStyle::AttributeNotProvided;
    }
}

static WebCore::AttributeTextStyle::UnderlineStyle convertUnderlineStyle(const UnderlineStyles& style)
{
    switch (style) {
    case NoUnderline:
        return WebCore::AttributeTextStyle::NoUnderline;
    case StandardUnderline:
        return WebCore::AttributeTextStyle::StandardUnderline;
    case BrokenUnderline:
        return WebCore::AttributeTextStyle::BrokenUnderline;
    case DottedUnderline:
        return WebCore::AttributeTextStyle::DottedUnderline;
    case WavyUnderline:
    case UnderlineNotProvided:
    default:
        return WebCore::AttributeTextStyle::UnderlineNotProvided;
    }
}

bool InputHandler::setCursorPositionIfInputTextValidated(const WTF::String& expectedString, unsigned int desiredCursorPosition)
{
    // The element may have lost focus while changing the input.  Don't validate if the focus element
    // has been lost.
    if (!isActiveTextEdit())
        return false;

    if (expectedString != elementText(m_currentFocusElement.get()))
        return false;

    ASSERT(m_currentFocusElement->document() && m_currentFocusElement->document()->frame());
    setSelection(desiredCursorPosition, desiredCursorPosition);
    m_currentFocusElement->document()->frame()->selection()->revealSelection();

    return true;
}

ReplaceTextErrorCode InputHandler::replaceText(const ReplaceArguments& replaceArguments, const AttributedText& attributedText)
{
    ReplaceTextErrorCode errorCode = processReplaceText(replaceArguments, attributedText);

    // If an error occurred during processing, send a textChanged event so the client
    // can update it's cache of the input field details.
    if (errorCode != ReplaceTextSuccess && errorCode != ReplaceTextChangeDeniedSyncMismatch)
        notifyTextChanged(m_currentFocusElement.get());

    return errorCode;
}

ReplaceTextErrorCode InputHandler::processReplaceText(const ReplaceArguments& replaceArguments, const AttributedText& attributedText)
{
    if (!allowSyncInput())
        return ReplaceTextChangeDeniedSyncMismatch;

    if (!isActiveTextEdit())
        return ReplaceTextNonSpecifiedFault;

    ASSERT(m_currentFocusElement->document() && m_currentFocusElement->document()->frame());
    Frame* frame = m_currentFocusElement->document()->frame();

    unsigned int start = replaceArguments.start;
    unsigned int end = replaceArguments.end;
    unsigned int attributeStart = replaceArguments.attributeStart;
    unsigned int attributeEnd = replaceArguments.attributeEnd;
    unsigned int desiredCursorPosition = replaceArguments.cursorPosition;

    unsigned int stringLength = elementText(m_currentFocusElement.get()).length();
    if (start > stringLength)
        return ReplaceTextWrongStartArgument;
    if (end < start || end > stringLength)
        return ReplaceTextWrongEndArgument;

    WTF::String replacementText(attributedText.text, attributedText.length);
    WTF::String expectedString = elementText(m_currentFocusElement.get());
    if (expectedString.isEmpty())
        expectedString = replacementText;
    else
        expectedString.replace(start, end - start, replacementText);

    if (attributeStart + start > expectedString.length())
        return ReplaceTextWrongAttributeStart;

    if (attributeEnd < attributeStart || attributeEnd + start > expectedString.length())
        return ReplaceTextWrongAttributeEnd;

    if (desiredCursorPosition > expectedString.length())
        return ReplaceTextWrongCursorLocation;

    // Validation of input parameters is complete.  A change will be made!
    ProcessingChangeGuard guard(this);

#if SHOWDEBUG_INPUTHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::replaceText Request being processed start=%d, end=%d, attributeStart=%d, attributeEnd=%d, desiredCursor=%d, stringLength=%d, attributeTextLength=%d, runsCount=%d", start, end, attributeStart, attributeEnd, desiredCursorPosition, stringLength, attributedText.length, attributedText.runsCount);

    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::replaceText Text before %s", elementText(m_currentFocusElement.get()).latin1().data());
#endif

    // Disable selectionHandler's active selection as we will be resetting and these
    // changes should not be handled as notification event.
    m_webPage->m_selectionHandler->setSelectionActive(false);

    // Remove any attributes currently set on the text in the field before making any changes.
    removeAttributedTextMarker();

    bool isPasswordField = elementType(m_currentFocusElement.get()) == InputTypePassword;
    if (isPasswordField)
        applyPasswordFieldUnmaskSecure(m_currentFocusElement.get(), 0, 0);

    // Place the cursor at the desired location or select the appropriate text.
    if (selectionStart() != start || selectionEnd() != end)
        setSelection(start, end);

    if (attributedText.length == 1 && attributeStart == attributeEnd && start == end) {
        // Handle single key non-attributed entry as key press rather than replaceText to allow
        // triggering of javascript events.

#if ENABLE_DEBUG_UNDOREDO
        if (replacementText[0] == 'u')
            editor->command("Undo").execute();
        else if (replacementText[0] == 'r')
            editor->command("Redo").execute();
        else
#endif
            handleKeyboardInput(PlatformKeyboardEvent::Char, replacementText[0]);

        return setCursorPositionIfInputTextValidated(expectedString, desiredCursorPosition) ? ReplaceTextSuccess : ReplaceTextChangeNotValidated;
    }

    Editor* editor = frame->editor();
    ASSERT(editor);

    // Perform the text change as a single command if there is one.
    if (end > start)
        editor->command("DeleteBackward").execute();
    if (attributedText.length)
        editor->command("InsertText").execute(replacementText);

#if SHOWDEBUG_INPUTHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::replaceText Text after %s", elementText(m_currentFocusElement.get()).latin1().data());
#endif

    if (!setCursorPositionIfInputTextValidated(expectedString, desiredCursorPosition))
        return ReplaceTextChangeNotValidated;

    // Apply the attributes to the field.
    AttribRun* attributeRun = reinterpret_cast<AttribRun*>(attributedText.runs);
    for (int i = 0; i < attributedText.runsCount; i++) {
        unsigned int runStartPosition = start + attributeRun->offset;
        unsigned int runEndPosition = runStartPosition + attributeRun->length;
        if (runEndPosition > expectedString.length() || attributeRun->offset + attributeRun->length > attributeEnd)
            return ReplaceTextInvalidRunlength;

        addAttributedTextMarker(runStartPosition, attributeRun->length, attributeRun->attributes);

        // This is a hack.  It would be nice to test against the password attribute, but it is not present
        // on the reduced keyboard, but other attributes are and we need to show the text.  Additionally,
        // we can't just check if there is a run and show the text because we get alot of useless requests
        // with empty attributes (no change) being applied over top of the text.

        // There should never be more than one run, just in case there is, we'll only actually be using the
        // last one.
        if (isPasswordField && attributeRun->attributes)
            applyPasswordFieldUnmaskSecure(m_currentFocusElement.get(), runStartPosition, attributeRun->length);

        attributeRun++;
    }
    return ReplaceTextSuccess;
}

void InputHandler::addAttributedTextMarker(unsigned int startPosition, unsigned int length, uint64_t attributes)
{
    if (!attributes) // No attributes set, don't create an empty AttributeText marker.
        return;

    AttributeTextStyle textStyle;
    textStyle.setItalic(convertAttributeValue(static_cast<AttributeValue>(getItalicAttrib(attributes))));
    textStyle.setStrikethrough(convertAttributeValue(static_cast<AttributeValue>(getStrikethroughAttrib(attributes))));
    textStyle.setUnderline(convertUnderlineStyle(static_cast<UnderlineStyles>(getUnderlineAttrib(attributes))));

    AttributeValue highlightAttribute = static_cast<AttributeValue>(getHighlightAttrib(attributes));
    if (highlightAttribute == AttributeApplied) {
        textStyle.setBackgroundColor(Color("blue"));
        textStyle.setTextColor(Color("white"));
    } else if (highlightAttribute == AttributeNotApplied) {
        textStyle.setBackgroundColor(Color("white"));
        textStyle.setTextColor(Color("black"));
    }

    FontWeights fontWeight = static_cast<FontWeights>(getFontWeightAttrib(attributes));
    if (fontWeight != EUnsetWeight)
        textStyle.setFontWeight(convertFontWeight(static_cast<FontWeights>(fontWeight)));

    RefPtr<Range> markerRange = focusElementRange(startPosition, length);
    m_currentFocusElement->document()->markers()->addMarker(markerRange.get(), DocumentMarker::AttributeText, textStyle);

#if SHOWDEBUG_INPUTHANDLER
    Olympia::Platform::log(Olympia::Platform::LogLevelCritical, "InputHandler::addAttributedTextMarker Run Details start=%u, length=%u, italic=%d, strikethrough=%d, underline=%d, highlight=%d, fontWeight=%d"
                           , startPosition, length, static_cast<AttributeValue>(getItalicAttrib(attributes)), static_cast<AttributeValue>(getStrikethroughAttrib(attributes))
                           , static_cast<UnderlineStyles>(getUnderlineAttrib(attributes)), highlightAttribute, fontWeight);
#endif
}

void InputHandler::removeAttributedTextMarker()
{
    // Remove all attribute text markers.
    if (m_currentFocusElement && m_currentFocusElement->document())
        m_currentFocusElement->document()->markers()->removeMarkers(DocumentMarker::AttributeText);
}

void InputHandler::handleInputLocaleChanged(bool isRTL)
{
    if (!isActiveTextEdit())
        return;

    ASSERT(m_currentFocusElement->document() && m_currentFocusElement->document()->frame());
    Editor* editor = m_currentFocusElement->document()->frame()->editor();
    RenderObject* renderer = m_currentFocusElement->renderer();

    if (!renderer)
        return;
    ASSERT(editor);

    if ((renderer->style()->direction() == RTL) != isRTL)
        editor->setBaseWritingDirection(isRTL ? RightToLeftWritingDirection : LeftToRightWritingDirection);
}

void InputHandler::clearCurrentFocusElement()
{
    if (m_currentFocusElement)
        m_currentFocusElement->blur();
}

bool InputHandler::didNodeOpenPopup(Node* node)
{
    if (!node)
        return false;

    if (node->hasTagName(HTMLNames::selectTag))
        return openSelectPopup(static_cast<HTMLSelectElement*>(node));

    if (node->hasTagName(HTMLNames::optionTag)) {
        HTMLOptionElement* optionElement = static_cast<HTMLOptionElement*>(node);
        return openSelectPopup(optionElement->ownerSelectElement());
    }
    return false;
}

bool InputHandler::openSelectPopup(HTMLSelectElement* select)
{
    if (!select || select->disabled())
        return false;

    m_currentFocusElement = select;
    m_currentFocusElementType = SelectPopup;
    const WTF::Vector<Element*>& listItems = select->listItems();
    int size = listItems.size();
    if (!size)
        return false;

    bool multiple = select->multiple();

    ScopeArray<WebString> labels;
    labels.reset(new WebString[size]);
    bool* enableds = new bool[size];
    int* itemTypes = new int[size];
    bool* selecteds = new bool[size];

    for (int i = 0; i < size; i++) {
        if (listItems[i]->hasTagName(HTMLNames::optionTag)) {
            HTMLOptionElement* option = static_cast<HTMLOptionElement*>(listItems[i]);
            labels[i] = option->textIndentedToRespectGroupLabel();
            enableds[i] = option->disabled() ? 0 : 1;
            selecteds[i] = option->selected();
            itemTypes[i] = TypeOption;
        } else if (listItems[i]->hasTagName(HTMLNames::optgroupTag)) {
            HTMLOptGroupElement* optGroup = static_cast<HTMLOptGroupElement*>(listItems[i]);
            labels[i] = optGroup->groupLabelText();
            enableds[i] = false;
            selecteds[i] = false;
            itemTypes[i] = TypeGroup;
        } else if (listItems[i]->hasTagName(HTMLNames::hrTag)) {
            enableds[i] = false;
            selecteds[i] = false;
            itemTypes[i] = TypeSeparator;
        }
    }

    m_webPage->m_client->openPopupList(multiple, size, labels, enableds, itemTypes, selecteds);
    return true;
}

void InputHandler::setPopupListIndex(int index)
{
    if (index == -2)  // Abandon
        return clearCurrentFocusElement();

    if (!isActiveSelectPopup())
        return clearCurrentFocusElement();

    RenderObject* renderer = m_currentFocusElement->renderer();
    if (renderer && renderer->isMenuList()) {
        RenderMenuList* renderMenu = toRenderMenuList(renderer);
        renderMenu->hidePopup();
    }

    HTMLSelectElement* selectElement = static_cast<HTMLSelectElement*>(m_currentFocusElement.get());
    int optionIndex = (static_cast<WebCore::SelectElement*>(selectElement))->listToOptionIndex(index);
    selectElement->setSelectedIndexByUser(optionIndex, /* deselect = true */ true, /* fireOnChangeNow = false */ true);
    clearCurrentFocusElement();
}

void InputHandler::setPopupListIndexes(int size, bool* selecteds)
{
    if (!isActiveSelectPopup())
        return clearCurrentFocusElement();

    HTMLSelectElement* selectElement = static_cast<HTMLSelectElement*>(m_currentFocusElement.get());

    const WTF::Vector<Element*>& items = selectElement->listItems();
    if (items.size() != size)
        return;

    HTMLOptionElement* option;
    for (int i = 0; i < size; i++) {
        if (items[i]->hasTagName(HTMLNames::optionTag)) {
            option = static_cast<HTMLOptionElement*>(items[i]);
            option->setSelectedState(selecteds[i]);
        }
    }
    // Force repaint because we do not send mouse events to the select element
    // and the element doesn't automatically repaint itself.
    selectElement->dispatchFormControlChangeEvent();
    selectElement->renderer()->repaint(true);
    clearCurrentFocusElement();
}

}
}
