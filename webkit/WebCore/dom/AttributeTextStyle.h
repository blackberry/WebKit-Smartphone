/*
 * Copyright (C) Research in Motion Limited 2010. All rights reserved.
 *
 */

#ifndef AttributeTextStyle_h
#define AttributeTextStyle_h

#include "Color.h"
#include "FontDescription.h"

namespace WebCore {

class AttributeTextStyle {
public:
    enum AttributeStyleState {
        AttributeNotProvided = -1,
        AttributeNotApplied = 0,
        AttributeApplied = 1
    };

    enum UnderlineStyle {
        UnderlineNotProvided = -1,
        NoUnderline = 0,
        StandardUnderline = 1,
        BrokenUnderline = 2,
        DottedUnderline = 3
    };

    AttributeTextStyle()
        : m_italic(AttributeNotProvided)
        , m_strikethrough(AttributeNotProvided)
        , m_underline(UnderlineNotProvided)
        , m_hasFontWeight(false)
        , m_fontWeight(FontWeightNormal)
        {};

    Color textColor() const { return m_textColor; }
    void setTextColor(Color textColor) { m_textColor = textColor; }

    Color backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(Color backgroundColor) { m_backgroundColor = backgroundColor; }

    AttributeStyleState italic() const { return m_italic; }
    void setItalic(AttributeStyleState italic) { m_italic = italic; }

    AttributeStyleState strikethrough() const { return m_strikethrough; }
    void setStrikethrough(AttributeStyleState strikethrough) { m_strikethrough = strikethrough; }

    UnderlineStyle underline() const { return m_underline; }
    void setUnderline(UnderlineStyle underline) { m_underline = underline; }

    bool hasFontWeight() const { return m_hasFontWeight; }
    void setHasFontWeight(bool hasFontWeight) { m_hasFontWeight = hasFontWeight; }

    FontWeight fontWeight() const { return m_fontWeight; }
    void setFontWeight(FontWeight fontWeight) { setHasFontWeight(true); m_fontWeight = fontWeight; }

    bool operator==(const AttributeTextStyle& o) const
    {
        return m_italic == o.italic() && m_strikethrough == o.strikethrough() && m_underline == o.underline()
                && m_hasFontWeight == o.hasFontWeight() && m_fontWeight == o.fontWeight()
                && m_textColor == o.textColor() && m_backgroundColor == o.backgroundColor();
    }

    bool operator!=(const AttributeTextStyle& o) const
    {
        return !(*this == o);
    }

private:
    Color m_textColor;
    Color m_backgroundColor;
    AttributeStyleState m_italic;
    AttributeStyleState m_strikethrough;
    UnderlineStyle m_underline;
    bool m_hasFontWeight;
    FontWeight m_fontWeight;
};

} // namespace WebCore

#endif // AttributeTextStyle_h
 
