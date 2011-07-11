TEMPLATE = subdirs
CONFIG += ordered

include(WebKit.pri)

!v8: SUBDIRS += JavaScriptCore
webkit2 {
    SUBDIRS += WebKit2
}
SUBDIRS += WebCore

webkit2 {
    exists($$PWD/WebKit2/WebProcess.pro): SUBDIRS += WebKit2/WebProcess.pro
}

symbian {
    # Forward the install target to WebCore. A workaround since INSTALLS is not implemented for symbian
    install.commands = $(MAKE) -C WebCore install
    QMAKE_EXTRA_TARGETS += install
}

