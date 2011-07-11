/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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
#include "WebDOMElement.h"

#include "Attr.h"
#include "Element.h"
#include "ExceptionCode.h"
#include "WebDOMAttr.h"
#include "WebExceptionHandlers.h"

void WebDOMElement::setAttribute(const WebDOMString& name, const WebDOMString& value)
{
    if (!impl())
        return;

    WebCore::ExceptionCode ec = 0;
    impl()->setAttribute(name, value, ec);
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
}

WebDOMAttr WebDOMElement::setAttributeNode(const WebDOMAttr& newAttr)
{
    if (!impl())
        return WebDOMAttr();

    WebCore::Attr* attr = webCore(newAttr);
    if (!attr) {
        webDOMRaiseError(static_cast<WebDOMExceptionCode>(WebCore::TYPE_MISMATCH_ERR));
        return WebDOMAttr();
    }

    WebCore::ExceptionCode ec = 0;
    WebDOMAttr result = webKit(WTF::getPtr(impl()->setAttributeNode(attr, ec)));
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
    return result;
}

void WebDOMElement::setAttributeNS(const WebDOMString& namespaceURI, const WebDOMString& qualifiedName, const WebDOMString& value)
{
    if (!impl())
        return;

    WebCore::ExceptionCode ec = 0;
    impl()->setAttributeNS(namespaceURI, qualifiedName, value, ec);
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
}

WebDOMAttr WebDOMElement::setAttributeNodeNS(const WebDOMAttr& newAttr)
{
    if (!impl())
        return WebDOMAttr();

    WebCore::Attr* attr = webCore(newAttr);
    if (!attr) {
        webDOMRaiseError(static_cast<WebDOMExceptionCode>(WebCore::TYPE_MISMATCH_ERR));
        return WebDOMAttr();
    }

    WebCore::ExceptionCode ec = 0;
    WebDOMAttr result = webKit(WTF::getPtr(impl()->setAttributeNodeNS(attr, ec)));
    webDOMRaiseError(static_cast<WebDOMExceptionCode>(ec));
    return result;
}
