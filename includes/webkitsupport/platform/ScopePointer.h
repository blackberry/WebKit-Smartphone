/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef ScopePointer_h
#define ScopePointer_h

#include "PointerBase.h"

// Warning: 
// 1. ScopePointer is not safe for casting. For example,
// ScopePointer<A> pointer(new B());
// This will crash in the case A is not the first base class of B,
// or/and A has no virtual function but B does.
// 2. The ctor and dtor of a ScopePointer object must be called
// with same new/delete provider.
// For safe unshared pointer, use UnsharedPointer which saves the
// original address of the object and uses the proper delete function.

template<typename T> class ScopePointerBase
{
public:
    T* get() const { return m_object; }
    operator bool() const { return m_object; }

    void swap(ScopePointerBase<T>& o)
    {
        T* tmpObject = m_object;
        m_object = o.m_object;
        o.m_object = tmpObject;
    }

    T* release()
    {
        T* object = m_object;
        m_object = 0;
        return object;
    }

protected:
    explicit ScopePointerBase(T* object)
        : m_object(object)
    {
    }

    // Uncopyable
    ScopePointerBase(const ScopePointerBase<T>& o);
    ScopePointerBase<T>& operator=(const ScopePointerBase<T>& o);

    T* m_object;
};

template<typename T> class ScopePointer: public ScopePointerBase<T>
{
public:
    typedef ScopePointerBase<T> BaseType;
    explicit ScopePointer(T* object = 0)
        : BaseType(object)
    {
    }

    ~ScopePointer()
    {
        delete this->m_object;
    }

    void reset(T* object = 0)
    {
        delete this->m_object;
        this->m_object = object;
    }

    typename PointerTraits<T>::ReferenceType operator*() const { return *this->m_object; }
    T* operator->() const { return this->m_object; }
};

template<typename T> class ScopeArray: public ScopePointerBase<T>
{
public:
    typedef ScopePointerBase<T> BaseType;

    explicit ScopeArray(T* object = 0)
        : BaseType(object)
    {
    }

    ~ScopeArray()
    {
        delete[] this->m_object;
    }

    void reset(T* object = 0)
    {
        delete[] this->m_object;
        this->m_object = object;
    }

    typename PointerTraits<T>::ReferenceType operator[](unsigned index) const { return this->m_object[index]; }
};

#endif // ScopePointer_h
