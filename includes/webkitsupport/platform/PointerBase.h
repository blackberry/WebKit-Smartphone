/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef PointerBase_h
#define PointerBase_h

#include "BlackBerryPlatformMemory.h"

// The following static functions are defined in the same module where the 
// smart pointer is attached to an object/array created by operator new/new[],
// so that it uses the proper operator delete/delete[].

template<typename T>
struct PointerTraits {
    typedef T& ReferenceType;
};

template<>
struct PointerTraits<void> {
    typedef void ReferenceType;
};

template<typename T> static void deleteObject(void* object)
{
    char checkSafeType[sizeof(T) ? 1 : -1];
    checkSafeType; // suppress the warning
    delete static_cast<T*>(object);
}

template<typename T> static void deleteArray(void* object)
{
    char checkSafeType[sizeof(T) ? 1 : -1];
    checkSafeType; // suppress the warning
    delete[] static_cast<T*>(object);
}

typedef void (*FuncDelete)(void*);

struct ImplicitCaster
{
    template<typename T, typename Y> static void cast(T* source, Y** dest)
    {
        *dest = source;
    }
};

struct StaticCaster
{
    template<typename T, typename Y> static void cast(T* source, Y** dest)
    {
        *dest = static_cast<Y*>(source);
    }
};

struct ReinterpretCaster
{
    template<typename T, typename Y> static void cast(T* source, Y** dest)
    {
        *dest = reinterpret_cast<Y*>(source);
    }
};

#endif // PointerBase_h
