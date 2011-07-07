SOURCES = \
    $$PWD/AccessibilityControllerOlympia.cpp \
    $$PWD/AccessibilityUIElementOlympia.cpp \
    $$PWD/DumpRenderTree.cpp \
    $$PWD/EventSender.cpp \
    $$PWD/GCControllerOlympia.cpp \
    $$PWD/LayoutTestControllerOlympia.cpp \
    $$PWD/WorkQueueItemOlympia.cpp

INCLUDEPATH += $$PWD/
INCLUDEPATH += $$PWD/../
INCLUDEPATH += $$PWD/../../../JavaScriptCore/ForwardingHeaders
DEPENDPATH += $$PWD/../

HEADERS += \
    $$PWD/../DumpRenderTree.h \
    $$PWD/../LayoutTestController.h \
    $$PWD/../WorkQueue.h \
    $$PWD/../WorkQueueItem.h \
    $$PWD/DumpRenderTreeOlympia.h \
    $$PWD/EventSender.h \
    $$PWD/GCControllerOlympia.h

SOURCES += \
    $$PWD/../AccessibilityController.cpp \
    $$PWD/../AccessibilityUIElement.cpp \
    $$PWD/../LayoutTestController.cpp \
    $$PWD/../WorkQueue.cpp

