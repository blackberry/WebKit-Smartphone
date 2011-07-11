SOURCES = \
    $$PWD/AccessibilityControllerBlackBerry.cpp \
    $$PWD/AccessibilityUIElementBlackBerry.cpp \
    $$PWD/DumpRenderTree.cpp \
    $$PWD/EventSender.cpp \
    $$PWD/GCControllerBlackBerry.cpp \
    $$PWD/LayoutTestControllerBlackBerry.cpp \
    $$PWD/WorkQueueItemBlackBerry.cpp

INCLUDEPATH += $$PWD/
INCLUDEPATH += $$PWD/../
INCLUDEPATH += $$PWD/../../../JavaScriptCore/ForwardingHeaders
DEPENDPATH += $$PWD/../

HEADERS += \
    $$PWD/../DumpRenderTree.h \
    $$PWD/../LayoutTestController.h \
    $$PWD/../WorkQueue.h \
    $$PWD/../WorkQueueItem.h \
    $$PWD/DumpRenderTreeBlackBerry.h \
    $$PWD/EventSender.h \
    $$PWD/GCControllerBlackBerry.h

SOURCES += \
    $$PWD/../AccessibilityController.cpp \
    $$PWD/../AccessibilityTextMarker.cpp \
    $$PWD/../AccessibilityUIElement.cpp \
    $$PWD/../LayoutTestController.cpp \
    $$PWD/../WorkQueue.cpp

