/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef OlympiaContext_h
#define OlympiaContext_h

#include <vector>

namespace Olympia {
namespace WebKit {

class String;

class Context {
public:
    enum ContextType { ContextTypeText, ContextTypeUrl, ContextTypeImageSrc, ContextTypeImageAltText, ContextTypeNotSelectable, ContextTypePattern, ContextTypeInput, ContextTypeFocusable };

    Context() {}

    Context(const std::vector<ContextType>& types, const std::vector<String>& info)
        : m_types(types)
        , m_info(info) {}

    int size() const { return m_types.size(); }
    const std::vector<ContextType>& types() const { return m_types; }
    const std::vector<String>& info() const { return m_info; }

private:
    std::vector<ContextType> m_types;
    std::vector<String> m_info;

};
}
}
#endif // OlympiaContext_h
