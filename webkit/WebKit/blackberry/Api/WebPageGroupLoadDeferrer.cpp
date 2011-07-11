/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "WebPageGroupLoadDeferrer.h"

#include "PageGroupLoadDeferrer.h"
#include "WebPage.h"
#include "WebPage_p.h"

namespace Olympia {
namespace WebKit {

WebPageGroupLoadDeferrer::WebPageGroupLoadDeferrer(WebPage* webPage)
{
    WebCore::TimerBase::fireTimersInNestedEventLoop();
    m_pageGroupLoadDeferrer = new WebCore::PageGroupLoadDeferrer(webPage->d->m_page, true);
}

WebPageGroupLoadDeferrer::~WebPageGroupLoadDeferrer()
{
    delete m_pageGroupLoadDeferrer;
}

} // namespace WebKit
} // namespace Olympia
