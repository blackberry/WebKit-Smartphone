/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef SelectionHandler_h
#define SelectionHandler_h

#include "OlympiaPlatformPrimitives.h"
#include "TextGranularity.h"

namespace WTF {
class String;
}

namespace WebCore {
class IntPoint;
class IntRect;
}

namespace Olympia {
namespace WebKit {

class WebPagePrivate;
class WebString;

class SelectionHandler {
public:
    SelectionHandler(WebPagePrivate*);
    ~SelectionHandler();

    bool isSelectionActive() { return m_selectionActive; }
    void setSelectionActive(bool active) { m_selectionActive = active; }

    void cancelSelection();
    WebString selectedText() const;

    bool findNextString(const WTF::String&);

    void setSelection(WebCore::IntPoint start, WebCore::IntPoint end);
    void selectAtPoint(WebCore::IntPoint&);
    void selectObject(WebCore::IntPoint&, WebCore::TextGranularity);

    WebCore::IntRect selectionStartCaretRect();
    WebCore::IntRect selectionEndCaretRect();

    void selectionPositionChanged();

private:

    WebPagePrivate* m_webPage;

    bool m_selectionActive;
};

}
}

#endif // SelectionHandler_h
