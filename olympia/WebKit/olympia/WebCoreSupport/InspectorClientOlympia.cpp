/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "InspectorClientOlympia.h"

#include "NotImplemented.h"

namespace WebCore {

void InspectorClientOlympia::inspectorDestroyed()
{
    delete this;
}

Page* InspectorClientOlympia::createPage()
{
    notImplemented();
    return 0;
}

String InspectorClientOlympia::localizedStringsURL()
{
    notImplemented();
    return String();
}

String InspectorClientOlympia::hiddenPanels()
{
    notImplemented();
    return String();
}

void InspectorClientOlympia::showWindow()
{
    notImplemented();
}

void InspectorClientOlympia::closeWindow()
{
    notImplemented();
}

void InspectorClientOlympia::attachWindow()
{
    notImplemented();
}

void InspectorClientOlympia::detachWindow()
{
    notImplemented();
}

void InspectorClientOlympia::setAttachedWindowHeight(unsigned)
{
    notImplemented();
}

void InspectorClientOlympia::highlight(Node*)
{
    notImplemented();
}

void InspectorClientOlympia::hideHighlight()
{
    notImplemented();
}

void InspectorClientOlympia::inspectedURLChanged(const String&)
{
    notImplemented();
}

void InspectorClientOlympia::populateSetting(const String& key, String* value)
{
    notImplemented();
}

void InspectorClientOlympia::storeSetting(const String& key, const String& value)
{
    notImplemented();
}

void InspectorClientOlympia::inspectorWindowObjectCleared()
{
    notImplemented();
}

void InspectorClientOlympia::openInspectorFrontend(InspectorController*)
{
    notImplemented();
}

} // namespace WebCore
