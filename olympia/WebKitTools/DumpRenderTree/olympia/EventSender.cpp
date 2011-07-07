/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
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

#include "EventSender.h"
#include "NotImplemented.h"

#include <JavaScriptCore/JSObjectRef.h>
#include <JavaScriptCore/JSRetainPtr.h>
#include <JavaScriptCore/JSStringRef.h>
#include <JavaScriptCore/JSValueRef.h>

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

static JSValueRef mouseDownCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef*       exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef mouseUpCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef*         exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef mouseMoveToCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef*     exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef beginDragWithFilesCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[],          JSValueRef* exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef leapForwardCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef*     exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef keyDownCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef*         exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef textZoomInCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef*      exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef textZoomOutCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef*     exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef zoomPageInCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef*      exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
}

static JSValueRef zoomPageOutCallback(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef*     exception)
{
    notImplemented();
    return JSValueMakeUndefined(context);
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
    notImplemented();
    return JSObjectMake(context, getClass(context), 0);
}
