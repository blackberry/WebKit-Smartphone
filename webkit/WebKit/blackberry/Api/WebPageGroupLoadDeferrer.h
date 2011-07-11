/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef WebPageGroupLoadDeferrer_h
#define WebPageGroupLoadDeferrer_h

namespace WebCore {
class PageGroupLoadDeferrer;
}

namespace Olympia {
namespace WebKit {

class WebPage;

// WebPageGroupLoadDeferrer is supposed to be used in the same way as WebCore::PageGroupLoadDeferrer.
// Declare a WebPageGroupLoadDeferrer object in the scope where the page group should defer loading and DOM timers.
class WebPageGroupLoadDeferrer {
public:
    explicit WebPageGroupLoadDeferrer(WebPage*);
    ~WebPageGroupLoadDeferrer();
private:
    WebCore::PageGroupLoadDeferrer* m_pageGroupLoadDeferrer;
};

} // namespace WebKit
} // namespace Olympia

#endif // WebPageGroupLoadDeferrer_h
