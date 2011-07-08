/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Apple Computer, Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "EventSender.h"

#include "DumpRenderTreeOlympia.h"
#include "IntPoint.h"
#include <OlympiaPlatformInputEvents.h>
#include <OlympiaPlatformKeyboardCodes.h>
#include "NotImplemented.h"
#include "PlatformTouchPoint.h"
#include <Vector>
#include "WebPage.h"

#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSRetainPtr.h>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSValueRef.h>

static WebCore::IntPoint lastMousePosition;
static Vector<Olympia::Platform::TouchPoint> touches;
static bool touchActive = false;

void sendTouchEvent(Olympia::Platform::TouchEvent::Type type);

// Callbacks

static JSValueRef getDragModeCallback(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef* exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static bool setDragModeCallback(JSContextRef context, JSObjectRef object, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef mouseWheelToCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef*    exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef contextClickCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef*    exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef mouseDownCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    Olympia::WebKit::WebPage* page = Olympia::WebKit::DumpRenderTree::currentInstance()->page();
    page->mouseEvent(Olympia::WebKit::MouseEventPressed, lastMousePosition, WebCore::IntPoint(0, 0));
    return JSValueMakeUndefined(context);
}

static JSValueRef mouseUpCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    Olympia::WebKit::WebPage* page = Olympia::WebKit::DumpRenderTree::currentInstance()->page();
    page->mouseEvent(Olympia::WebKit::MouseEventReleased, lastMousePosition, WebCore::IntPoint(0, 0));
    return JSValueMakeUndefined(context);
}

static JSValueRef mouseMoveToCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    if (argumentCount < 2)
        return JSValueMakeUndefined(context);

    int x = static_cast<int>(JSValueToNumber(context, arguments[0], exception));
    ASSERT(!exception || !*exception);
    int y = static_cast<int>(JSValueToNumber(context, arguments[1], exception));
    ASSERT(!exception || !*exception);

    lastMousePosition = WebCore::IntPoint(x, y);
    Olympia::WebKit::WebPage* page = Olympia::WebKit::DumpRenderTree::currentInstance()->page();
    page->mouseEvent(Olympia::WebKit::MouseEventMoved, lastMousePosition, WebCore::IntPoint(0, 0));

    return JSValueMakeUndefined(context);
}

static JSValueRef beginDragWithFilesCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef leapForwardCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef keyDownCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    if (argumentCount < 1)
        return JSValueMakeUndefined(context);

    JSStringRef character = JSValueToStringCopy(context, arguments[0], exception);
    ASSERT(!*exception);
    short charCode = 0;
    bool needsShiftKeyModifier = false;
    if (JSStringIsEqualToUTF8CString(character, "leftArrow"))
        charCode = Olympia::Platform::KEY_CONTROL_LEFT;
    else if (JSStringIsEqualToUTF8CString(character, "rightArrow"))
        charCode = Olympia::Platform::KEY_CONTROL_RIGHT;
    else if (JSStringIsEqualToUTF8CString(character, "upArrow"))
        charCode = Olympia::Platform::KEY_CONTROL_UP;
    else if (JSStringIsEqualToUTF8CString(character, "downArrow"))
        charCode = Olympia::Platform::KEY_CONTROL_DOWN;
    else if (JSStringIsEqualToUTF8CString(character, "pageUp")
             || JSStringIsEqualToUTF8CString(character, "pageDown")
             || JSStringIsEqualToUTF8CString(character, "home")
             || JSStringIsEqualToUTF8CString(character, "end"))
         return JSValueMakeUndefined(context);
    else if (JSStringIsEqualToUTF8CString(character, "delete"))
        charCode = Olympia::Platform::KEY_BACKSPACE;
    else {
        charCode = JSStringGetCharactersPtr(character)[0];
        if (WTF::isASCIIUpper(charCode))
            needsShiftKeyModifier = true;
    }
    JSStringRelease(character);

    Olympia::WebKit::WebPage* page = Olympia::WebKit::DumpRenderTree::currentInstance()->page();
    page->keyEvent(Olympia::Platform::KeyboardEvent::KeyChar, charCode, needsShiftKeyModifier);

    return JSValueMakeUndefined(context);
}

static JSValueRef textZoomInCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef textZoomOutCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef zoomPageInCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    Olympia::WebKit::DumpRenderTree::currentInstance()->page()->zoomIn();
    return JSValueMakeUndefined(context);
}

static JSValueRef zoomPageOutCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    Olympia::WebKit::DumpRenderTree::currentInstance()->page()->zoomOut();
    return JSValueMakeUndefined(context);
}

static JSValueRef addTouchPointCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    if (argumentCount < 2)
        return JSValueMakeUndefined(context);

    int x = static_cast<int>(JSValueToNumber(context, arguments[0], exception));
    ASSERT(!exception || !*exception);
    int y = static_cast<int>(JSValueToNumber(context, arguments[1], exception));
    ASSERT(!exception || !*exception);

    Olympia::Platform::TouchPoint touch;
    touch.m_id = touches.isEmpty() ? 0 : touches.last().m_id + 1;
    WebCore::IntPoint pos(x, y);
    touch.m_pos = pos;
    touch.m_screenPos = pos;
    touch.m_state = Olympia::Platform::TouchPoint::TouchPressed;

    touches.append(touch);

    return JSValueMakeUndefined(context);
}

static JSValueRef updateTouchPointCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    if (argumentCount < 3)
        return JSValueMakeUndefined(context);

    int index = static_cast<int>(JSValueToNumber(context, arguments[0], exception));
    ASSERT(!exception || !*exception);
    int x = static_cast<int>(JSValueToNumber(context, arguments[1], exception));
    ASSERT(!exception || !*exception);
    int y = static_cast<int>(JSValueToNumber(context, arguments[2], exception));
    ASSERT(!exception || !*exception);

    if (index < 0 || index >= touches.size())
        return JSValueMakeUndefined(context);

    Olympia::Platform::TouchPoint& touch = touches[index];
    WebCore::IntPoint pos(x, y);
    touch.m_pos = pos;
    touch.m_screenPos = pos;
    touch.m_state = Olympia::Platform::TouchPoint::TouchMoved;

    return JSValueMakeUndefined(context);
}

static JSValueRef setTouchModifierCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef touchStartCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    if (!touchActive) {
        sendTouchEvent(Olympia::Platform::TouchEvent::TouchStart);
        touchActive = true;
    } else
        sendTouchEvent(Olympia::Platform::TouchEvent::TouchMove);
    return JSValueMakeUndefined(context);
}

static JSValueRef touchCancelCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef touchMoveCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    sendTouchEvent(Olympia::Platform::TouchEvent::TouchMove);
    return JSValueMakeUndefined(context);
}

static JSValueRef touchEndCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    for (int i = 0; i < touches.size(); ++i)
        if (touches[i].m_state != Olympia::Platform::TouchPoint::TouchReleased) {
            sendTouchEvent(Olympia::Platform::TouchEvent::TouchMove);
            return JSValueMakeUndefined(context);
        }
    sendTouchEvent(Olympia::Platform::TouchEvent::TouchEnd);
    touchActive = false;
    return JSValueMakeUndefined(context);
}

static JSValueRef clearTouchPointsCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    touches.clear();
    touchActive = false;
    return JSValueMakeUndefined(context);
}

static JSValueRef cancelTouchPointCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef releaseTouchPointCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    if (argumentCount < 1)
        return JSValueMakeUndefined(context);

    int index = static_cast<int>(JSValueToNumber(context, arguments[0], exception));
    ASSERT(!exception || !*exception);
    if (index < 0 || index >= touches.size())
        return JSValueMakeUndefined(context);

    touches[index].m_state = Olympia::Platform::TouchPoint::TouchReleased;
    return JSValueMakeUndefined(context);
}

void sendTouchEvent(Olympia::Platform::TouchEvent::Type type)
{
    Olympia::Platform::TouchEvent event;
    event.m_type = type;
    event.m_singleType = Olympia::Platform::TouchEvent::SingleNone;
    event.m_points.assign(touches.begin(), touches.end());
    Olympia::WebKit::DumpRenderTree::currentInstance()->page()->touchEvent(event);

    Vector<Olympia::Platform::TouchPoint> t;

    for (Vector<Olympia::Platform::TouchPoint>::iterator it = touches.begin();
         it != touches.end();
         ++it) {
        if (it->m_state != Olympia::Platform::TouchPoint::TouchReleased) {
            it->m_state = Olympia::Platform::TouchPoint::TouchStationary;
            t.append(*it);
        }
    }
    touches = t;
}

static JSStaticFunction staticFunctions[] = {
    { "mouseWheelTo", mouseWheelToCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "contextClick", contextClickCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "mouseDown", mouseDownCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "mouseUp", mouseUpCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "mouseMoveTo", mouseMoveToCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "beginDragWithFiles", beginDragWithFilesCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "leapForward", leapForwardCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "keyDown", keyDownCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "textZoomIn", textZoomInCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "textZoomOut", textZoomOutCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "addTouchPoint", addTouchPointCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "cancelTouchPoint", addTouchPointCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "clearTouchPoints", clearTouchPointsCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "releaseTouchPoint", releaseTouchPointCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "setTouchModifier", setTouchModifierCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "touchCancel", touchCancelCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "touchEnd", touchEndCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "touchMove", touchMoveCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "touchStart", touchStartCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "updateTouchPoint", updateTouchPointCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "zoomPageIn", zoomPageInCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { "zoomPageOut", zoomPageOutCallback, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete },
    { 0, 0, 0 }
};

static JSStaticValue staticValues[] = {
    { "dragMode", getDragModeCallback, setDragModeCallback, kJSPropertyAttributeNone },
    { 0, 0, 0, 0 }
};

static JSClassRef getClass(JSContextRef context)
{
    static JSClassRef eventSenderClass = 0;

    if (!eventSenderClass) {
        JSClassDefinition classDefinition = {0};
        classDefinition.staticFunctions = staticFunctions;
        classDefinition.staticValues = staticValues;

        eventSenderClass = JSClassCreate(&classDefinition);
    }

    return eventSenderClass;
}

void replaySavedEvents()
{
    notImplemented();
}

JSObjectRef makeEventSender(JSContextRef context)
{
    return JSObjectMake(context, getClass(context), 0);
}
