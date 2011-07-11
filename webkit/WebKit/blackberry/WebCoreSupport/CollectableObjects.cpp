/*
 * Copyright (C) 2009 Torch Mobile Inc.
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#include "config.h"
#include "ObjectAllocator.h"

#if ENABLE(OLYMPIA_OBJECT_ALLOCATOR)

#include "Cookie.h"
#include "KURL.h"
#include <runtime/UString.h>
#include <wtf/ListHashSet.h>

#define ALIGN_TO_8(size) ((size + 7) & ~7)

// Implement new/delete operators for a collectable class by using ObjectAllocator.
// "_maxRecycled" specifies how many dead objects can be cached in the pool. If more than 1 collectable
// classes share 1 ObjectAllocator, the capacity of the pool will be the sum of all "_maxRecycled".
// "_objectSize" indicates the maximum size that an object can have. A derived class can be bigger
// than its parent, so be sure "_objectSize" is ok for all derived classes.
#define IMPLEMENT_COLLECTABLE(_namespaceName, _className, _maxRecycled, _objectSize)   \
    typedef Olympia::Platform::ObjectAllocatorWrapper<ALIGN_TO_8(_objectSize), _maxRecycled> _className##Allocator; \
    void* _namespaceName::_className::operator new(size_t s) { return _className##Allocator::getObject(s); } \
    void _namespaceName::_className::operator delete(void* p) { if (p) _className##Allocator::releaseObject(p); }

// Create the ObjectAllocator for the given collectable class
#define INITIALIZE_COLLECTABLE(_className) _className##Allocator::initialize()

#define IMPLEMENT_LISTHASHSET(_valueArg) template struct ListHashSetNodeAllocatorBase<_valueArg>;
#define INITIALIZE_LISTHASHSET(_valueArg) ListHashSetTraits<_valueArg>::Collector::initialize()

namespace Olympia {
namespace Platform {

template <unsigned _ObjectSize, unsigned _MaxRecycled>
class ObjectAllocatorWrapper {
public:
    static void* getObject(size_t s)
    {
        ASSERT(s <= _ObjectSize);
        return m_allocator->getObject();
    }
    static void releaseObject(void* p)
    {
        m_allocator->releaseObject(p);
    }
    static void initialize()
    {
        m_allocator = ObjectAllocator::create(_ObjectSize, _MaxRecycled);
    }
protected:
    static ObjectAllocator* m_allocator;
};

template <unsigned _ObjectSize, unsigned _MaxRecycled>
ObjectAllocator* ObjectAllocatorWrapper<_ObjectSize, _MaxRecycled>::m_allocator = 0;

template <typename ValueArg> struct ListHashSetTraits {
    typedef WTF::ListHashSetNode<ValueArg, 256> Node;
    // FIXME: In order to match WebKit behavior, we should dynamically increase/decrease the capacity when
    // a ListHashSetNodeAllocatorBase object is created/destroyed. But that can also give performance penalty and
    // waste more memory. Let's keep using the old fixed capacity for now.
    typedef ObjectAllocatorWrapper<ALIGN_TO_8(sizeof(Node)), 256> Collector;
};

} // namespace Platform
} // namespace Olympia

//IMPLEMENT_COLLECTABLE(JSC::UString, Rep, 200, sizeof(JSC::UString::BaseString))

namespace WebCore {
    class CachedResource;
    class Document;
    class Element;
    class Node;
    class RenderBox;
    class RenderInline;
    class SimpleFontData;
}

namespace WTF {

template <typename ValueArg> void* ListHashSetNodeAllocatorBase<ValueArg>::allocate()
{ 
    return Olympia::Platform::ListHashSetTraits<ValueArg>::Collector::
        getObject(ALIGN_TO_8(sizeof(typename Olympia::Platform::ListHashSetTraits<ValueArg>::Node)));
}

template <typename ValueArg> void ListHashSetNodeAllocatorBase<ValueArg>::deallocate(void* node)
{ 
    Olympia::Platform::ListHashSetTraits<ValueArg>::Collector::releaseObject(node);
}

IMPLEMENT_LISTHASHSET(WebCore::CachedResource*);
IMPLEMENT_LISTHASHSET(WebCore::Cookie);
IMPLEMENT_LISTHASHSET(WebCore::Element*);
IMPLEMENT_LISTHASHSET(WebCore::Node*);
IMPLEMENT_LISTHASHSET(WebCore::KURL);
IMPLEMENT_LISTHASHSET(WebCore::RenderBox*);
IMPLEMENT_LISTHASHSET(WebCore::RenderInline*);
IMPLEMENT_LISTHASHSET(const WebCore::SimpleFontData*);
IMPLEMENT_LISTHASHSET(WTF::RefPtr<WebCore::Document>);
IMPLEMENT_LISTHASHSET(WTF::RefPtr<WebCore::Node>);
IMPLEMENT_LISTHASHSET(int);

}

#endif // ENABLE(OLYMPIA_OBJECT_ALLOCATOR)

namespace Olympia {
namespace Platform {

void initializeObjectAllocators()
{
#if ENABLE(OLYMPIA_OBJECT_ALLOCATOR)
//    INITIALIZE_COLLECTABLE(Rep);
    INITIALIZE_LISTHASHSET(WebCore::CachedResource*);
    INITIALIZE_LISTHASHSET(WebCore::Cookie);
    INITIALIZE_LISTHASHSET(WebCore::Element*);
    INITIALIZE_LISTHASHSET(WebCore::Node*);
    INITIALIZE_LISTHASHSET(WebCore::KURL);
    INITIALIZE_LISTHASHSET(WebCore::RenderBox*);
    INITIALIZE_LISTHASHSET(WebCore::RenderInline*);
    INITIALIZE_LISTHASHSET(const WebCore::SimpleFontData*);
    INITIALIZE_LISTHASHSET(WTF::RefPtr<WebCore::Document>);
    INITIALIZE_LISTHASHSET(int);
    INITIALIZE_LISTHASHSET(WTF::RefPtr<WebCore::Node>);
#endif // ENABLE(OLYMPIA_OBJECT_ALLOCATOR)
}

} // namespace Platform
} // namespace Olympia

