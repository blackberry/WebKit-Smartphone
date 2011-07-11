/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef SharedPointer_h
#define SharedPointer_h

#include "PointerBase.h"

// Usage:
//      SharedPointer<A> a(new A(args));
//      SharedPointer<A> b(a);
//      SharedPointer<A> c;
//      c = b;
//      a.reset(new A(args));
//      someMethodUsingA(*a);
//      a->someMethodOfA();
//      A* rawPointer = a->get();

struct SharedPointerRefCounter {
    unsigned m_referenceCount;
    void* m_object;
    FuncDelete m_funcDeleteCounter;
    FuncDelete m_funcDeleteObject;

    SharedPointerRefCounter(void* object, FuncDelete objectDeleter, FuncDelete counterDeleter = deleteObject<SharedPointerRefCounter>)
        : m_referenceCount(1)
        , m_object(object)
        , m_funcDeleteCounter(counterDeleter)
        , m_funcDeleteObject(objectDeleter)
    {
    }

    void ref()
    {
        ++m_referenceCount;
    }

    void deref()
    {
        if (!--m_referenceCount) {
            m_funcDeleteObject(m_object);
            m_funcDeleteCounter(this);
        }
    }

    void reset(void* originalPointer, FuncDelete objectDeleter)
    {
        m_funcDeleteObject(m_object);
        m_object = originalPointer;
        m_funcDeleteObject = objectDeleter;
    }

    bool hasOneRef() const { return m_referenceCount == 1; }

private:
    // Not copyable
    SharedPointerRefCounter(const SharedPointerRefCounter&);
    SharedPointerRefCounter& operator=(const SharedPointerRefCounter&);
};

template<typename T> class SharedPointerBase {
public:
    unsigned refCount() const { return m_counter ? m_counter->m_referenceCount : 0; }

    T* get() const { return m_object; }

    operator bool() const { return m_object; }

    void reset()
    {
        if (m_counter) {
            m_counter->deref();
            m_counter = 0;
            m_object = 0;
        }
    }

protected:
    // "originalPointer" should be the raw address where the object is allocated.
    // "object" can be implicity cast pointer, and so it can be a different address.
    SharedPointerBase(T* object, void* originalPointer, FuncDelete objectDeleter)
        : m_counter(object ? new SharedPointerRefCounter(originalPointer, objectDeleter) : 0)
        , m_object(object)
    {
    }

    ~SharedPointerBase()
    {
        if (m_counter)
            m_counter->deref();
    }

    template <typename Y>
    SharedPointerBase(const SharedPointerBase<Y>& o)
        : m_counter(o.m_counter)
        , m_object(o.m_object)
    {
        if (m_counter)
            m_counter->ref();
    }

    template <typename Y, typename Caster>
    SharedPointerBase(const SharedPointerBase<Y>& o, Caster)
        : m_counter(o.m_counter)
    {
        Caster::cast(o.m_object, &m_object);
        if (m_counter)
            m_counter->ref();
    }

    void assign(const SharedPointerBase<T>& o)
    {
        if (this == &o || m_counter == o.m_counter)
            return;
        if (m_counter)
            m_counter->deref();
        m_counter = o.m_counter;
        m_object = o.m_object;
        if (m_counter)
            m_counter->ref();
    }

    void resetInternal(T* object, void* originalPointer, FuncDelete objectDeleter)
    {
        if (m_counter) {
            if (object) {
                if (m_counter->hasOneRef()) {
                    m_counter->reset(originalPointer, objectDeleter);
                } else {
                    m_counter->deref();
                    m_counter = new SharedPointerRefCounter(originalPointer, objectDeleter);
                }
            } else {
                m_counter->deref();
                m_counter = 0;
            }
        } else if (object)
            m_counter = new SharedPointerRefCounter(originalPointer, objectDeleter);

        m_object = object;
    }

    SharedPointerRefCounter* m_counter;
    T* m_object;

    template<typename Y> friend class SharedPointerBase;
};

template<typename T> class SharedPointer: public SharedPointerBase<T> {
public:
    typedef SharedPointerBase<T> BaseType;
    SharedPointer()
        : BaseType(0, 0, 0)
    {
    }
    // The first "object" passed to SharedPointerBase will be converted to T* implicitly.
    // The second "object" is stored as "void*" and will be used to delete the pointer.
    template <typename Y>
    explicit SharedPointer(Y* object, FuncDelete objectDeleter = deleteObject<Y>)
        : BaseType(object, object, objectDeleter)
    {
    }

    template <typename Y>
    SharedPointer(const SharedPointer<Y>& o)
        : BaseType(o)
    {
    }

    template <typename Y, typename Caster>
    SharedPointer(const SharedPointer<Y>& o, Caster caster)
        : BaseType(o, caster)
    {
    }


    using BaseType::reset;

    template <typename Y>
    void reset(Y* object, FuncDelete objectDeleter = deleteObject<Y>)
    {
        BaseType::resetInternal(object, object, objectDeleter);
    }

    SharedPointer<T>& operator=(const SharedPointer<T>& o)
    {
        BaseType::assign(o);
        return *this;
    }

    inline typename PointerTraits<T>::ReferenceType operator*() const { return *this->m_object; }
    inline T* operator->() const { return this->m_object; }
};

template<typename T> class SharedArray: public SharedPointerBase<T> {
public:
    typedef SharedPointerBase<T> BaseType;

    SharedArray()
        : BaseType(0, 0, 0)
    {
    }

    explicit SharedArray(T* object, FuncDelete objectDeleter = deleteArray<T>)
        : BaseType(object, object, objectDeleter)
    {
    }

    using BaseType::reset;

    // It's dangerous to cast array types, so we don't do it here.
    void reset(T* object, FuncDelete objectDeleter = deleteArray<T>)
    {
        BaseType::resetInternal(object, object, objectDeleter);
    }

    SharedArray<T>& operator=(const SharedArray<T>& o)
    {
        BaseType::assign(o);
        return *this;
    }

    inline typename PointerTraits<T>::ReferenceType operator[](unsigned index) const { return this->m_object[index]; }
};

#endif // SharedPointer_h
