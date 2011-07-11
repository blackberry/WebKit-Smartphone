/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "InspectorClientBlackBerry.h"

#include "NotImplemented.h"

namespace WebCore {

void InspectorClientBlackBerry::inspectorDestroyed()
{
    delete this;
}

Page* InspectorClientBlackBerry::createPage()
{
    notImplemented();
    return 0;
}

String InspectorClientBlackBerry::localizedStringsURL()
{
    notImplemented();
    return String();
}

String InspectorClientBlackBerry::hiddenPanels()
{
    notImplemented();
    return String();
}

void InspectorClientBlackBerry::showWindow()
{
    notImplemented();
}

void InspectorClientBlackBerry::closeWindow()
{
    notImplemented();
}

void InspectorClientBlackBerry::attachWindow()
{
    notImplemented();
}

void InspectorClientBlackBerry::detachWindow()
{
    notImplemented();
}

void InspectorClientBlackBerry::setAttachedWindowHeight(unsigned)
{
    notImplemented();
}

void InspectorClientBlackBerry::highlight(Node*)
{
    notImplemented();
}

void InspectorClientBlackBerry::hideHighlight()
{
    notImplemented();
}

void InspectorClientBlackBerry::inspectedURLChanged(const String&)
{
    notImplemented();
}

void InspectorClientBlackBerry::populateSetting(const String& key, String* value)
{
    notImplemented();
}

void InspectorClientBlackBerry::storeSetting(const String& key, const String& value)
{
    notImplemented();
}

void InspectorClientBlackBerry::inspectorWindowObjectCleared()
{
    notImplemented();
}

void InspectorClientBlackBerry::openInspectorFrontend(InspectorController*)
{
    notImplemented();
}

} // namespace WebCore
