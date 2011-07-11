/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef UnsharedPointer_h
#define UnsharedPointer_h

#include "PointerBase.h"

// Warning: UnsharedPointer/UnsharedArray saves the original raw address and proper delete function.
// But this brings a little bit overhead. In the cases this's not required,
// use ScopePointer/ScopeArray for better performance.

template<typename T> class UnsharedPointerBase
{
public:
    T* get() const { return m_object; }
    operator bool() const { return m_object; }

    T* detach(void** originalPointer = NULL, FuncDelete* deleter = NULL)
    {
        T* object = m_object;
        if (originalPointer)
            *originalPointer = m_originalPointer;
        if (deleter)
            *deleter = m_funcDeleteObject;
        m_object = 0;
        m_originalPointer = 0;
        m_funcDeleteObject = 0;
        return object;
    }

    void reset()
    {
        resetInternal(0, 0, 0);
    }

    template <typename Y, typename Caster> void swap(UnsharedPointerBase<Y>& o, Caster = ImplicitCaster())
    {
        T* tmpObject = m_object;
        void* tmpOriginalPointer =  m_originalPointer;
        FuncDelete tmpFuncDeleteObject = m_funcDeleteObject;
        Caster::cast(o.m_object, &m_object);
        m_originalPointer = o.m_originalPointer;
        m_funcDeleteObject = o.m_funcDeleteObject;
        Caster::cast(tmpObject, &o.m_object);
        o.m_originalPointer = tmpOriginalPointer;
        o.m_funcDeleteObject = tmpFuncDeleteObject;
    }

protected:
    // "originalPointer" should be the raw address where the object is allocated.
    // "object" can be implicity cast pointer, and so it can be a different address.
    UnsharedPointerBase(T* object, void* originalPointer, FuncDelete objectDeleter)
        : m_object(object)
        , m_originalPointer(originalPointer)
        , m_funcDeleteObject(objectDeleter)
    {
    }

    ~UnsharedPointerBase()
    {
        if (m_funcDeleteObject)
            m_funcDeleteObject(m_originalPointer);
    }

    // Uncopyable
    UnsharedPointerBase(const UnsharedPointerBase<T>& o);
    UnsharedPointerBase<T>& operator=(const UnsharedPointerBase<T>& o);

    void resetInternal(T* object, void* originalPointer, FuncDelete objectDeleter)
    {
        if (m_funcDeleteObject && m_originalPointer)
            m_funcDeleteObject(m_originalPointer);
        m_object = object;
        m_originalPointer = originalPointer;
        m_funcDeleteObject = objectDeleter;
    }

    T* m_object;
    void* m_originalPointer;
    FuncDelete m_funcDeleteObject;

    template<typename Y> friend class UnsharedPointerBase;
};

template<typename T> class UnsharedPointer: public UnsharedPointerBase<T>
{
public:
    typedef UnsharedPointerBase<T> BaseType;
    UnsharedPointer()
        : BaseType(0, 0, 0)
    {
    }
    // The first "object" passed to UnsharedPointerBase will be converted to T* implicitly.
    // The second "object" is stored as "void*" and will be used to delete the pointer.
    template <typename Y>
    explicit UnsharedPointer(Y* object, FuncDelete objectDeleter = deleteObject<Y>)
        : BaseType(object, object, objectDeleter)
    {
    }

    using BaseType::reset;

    template <typename Y>
    void reset(Y* object, FuncDelete objectDeleter = deleteObject<Y>)
    {
        BaseType::resetInternal(object, object, objectDeleter);
    }

    typename PointerTraits<T>::ReferenceType operator*() const { return *this->m_object; }
    T* operator->() const { return this->m_object; }
};

template<typename T> class UnsharedArray: public UnsharedPointerBase<T>
{
public:
    typedef UnsharedPointerBase<T> BaseType;

    UnsharedArray()
        : BaseType(0, 0, 0)
    {
    }

    explicit UnsharedArray(T* object, FuncDelete objectDeleter = deleteArray<T>)
        : BaseType(object, object, objectDeleter)
    {
    }

    using BaseType::reset;

    // It's dangerous to cast array types, so we don't do it here.
    // To cast UnsharedArray, Use swap() at the user's risk.
    void reset(T* object, FuncDelete objectDeleter = deleteArray<T>)
    {
        BaseType::resetInternal(object, object, objectDeleter);
    }

    typename PointerTraits<T>::ReferenceType operator[](unsigned index) const { return this->m_object[index]; }
};

#endif // UnsharedPointer_h
