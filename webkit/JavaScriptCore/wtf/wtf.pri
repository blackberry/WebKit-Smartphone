# wtf - qmake build info

SOURCES += \
    wtf/Assertions.cpp \
    wtf/ByteArray.cpp \
    wtf/CurrentTime.cpp \
    wtf/DateMath.cpp \
    wtf/dtoa.cpp \
    wtf/FastMalloc.cpp \
    wtf/HashTable.cpp \
    wtf/MD5.cpp \
    wtf/MainThread.cpp \
    wtf/PageAllocation.cpp \
    wtf/RandomNumber.cpp \
    wtf/RefCountedLeakCounter.cpp \
    wtf/Threading.cpp \
    wtf/TypeTraits.cpp \
    wtf/WTFThreadData.cpp \
    wtf/text/AtomicString.cpp \
    wtf/text/CString.cpp \
    wtf/text/StringImpl.cpp \
    wtf/text/StringStatics.cpp \
    wtf/text/WTFString.cpp \
    wtf/unicode/CollatorDefault.cpp \
    wtf/unicode/icu/CollatorICU.cpp \
    wtf/unicode/blackberry/UnicodeBlackBerry.cpp \
    wtf/unicode/blackberry/icu.cpp \
    wtf/unicode/UTF8.cpp \


isEmpty(OLYMPIA_PTHREAD) {
    SOURCES += wtf/ThreadingPthreads.cpp \
               wtf/ThreadIdentifierDataPthreads.cpp
    HEADERS += wtf/ThreadIdentifierDataPthreads.h
} else {
    SOURCES += wtf/ThreadingNone.cpp
}

!contains(DEFINES, USE_SYSTEM_MALLOC) {
    SOURCES += wtf/TCSystemAlloc.cpp
}

