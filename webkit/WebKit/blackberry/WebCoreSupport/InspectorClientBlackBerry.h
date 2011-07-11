/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#ifndef InspectorClientOlympia_h
#define InspectorClientOlympia_h

#include "InspectorClient.h"

namespace WebCore {

class InspectorClientBlackBerry : public InspectorClient {
public:
    virtual void inspectorDestroyed();
    virtual Page* createPage();
    virtual String localizedStringsURL();
    virtual String hiddenPanels();
    virtual void showWindow();
    virtual void closeWindow();
    virtual void attachWindow();
    virtual void detachWindow();
    virtual void setAttachedWindowHeight(unsigned);
    virtual void highlight(Node*);
    virtual void hideHighlight();
    virtual void inspectedURLChanged(const String&);
    virtual void populateSetting(const String& key, String* value);
    virtual void storeSetting(const String& key, const String& value);
    virtual void inspectorWindowObjectCleared();
    virtual void openInspectorFrontend(InspectorController*);
};

} // WebCore

#endif // InspectorClientOlympia_h
