TEMPLATE = subdirs
CONFIG += ordered

include(WebKit.pri)

SUBDIRS += \
        JavaScriptCore \
        WebCore

symbian {
    # Forward the install target to WebCore. A workaround since INSTALLS is not implemented for symbian
    install.commands = $(MAKE) -C WebCore install
    QMAKE_EXTRA_TARGETS += install
}

