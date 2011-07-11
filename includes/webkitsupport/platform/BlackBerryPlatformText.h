/*
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

#ifndef TextAPI_h
#define TextAPI_h

#include "text_api.h"

namespace BlackBerry {
namespace Platform {
namespace Text {

int clearCaches();

const int MAX_FONT_NAME = ::TextAPI::MAX_FONT_NAME;

typedef ::TextAPI::Color Color;
typedef ::TextAPI::ReturnCode ReturnCode;
typedef ::TextAPI::Utf16Char Utf16Char;
typedef ::TextAPI::Unit Unit;
const Unit UserUnit = ::TextAPI::UserUnit;
const Unit EmUnit = ::TextAPI::EmUnit;

typedef ::TextAPI::Dimension Dimension;
typedef ::TextAPI::ColorSpec ColorSpec;
typedef ::TextAPI::Style Style;
const Style NoStyle = ::TextAPI::NoStyle;
const Style PlainStyle = ::TextAPI::PlainStyle;
const Style ItalicStyle = ::TextAPI::ItalicStyle;
const Style ObliqueStyle = ::TextAPI::ObliqueStyle;

typedef ::TextAPI::Variant Variant;
const Variant NoVariant = ::TextAPI::NoVariant;
const Variant PlainVariant = ::TextAPI::PlainVariant;
const Variant SmallCapsVariant = ::TextAPI::SmallCapsVariant;

typedef ::TextAPI::ShadowType ShadowType;
const ShadowType NoShadow = ::TextAPI::NoShadow;
const ShadowType BlurredShadow = ::TextAPI::BlurredShadow;

typedef ::TextAPI::Shadow Shadow;
typedef ::TextAPI::OutlineEffectType OutlineEffectType;
const OutlineEffectType NoOutlineEffect = ::TextAPI::NoOutlineEffect;
const OutlineEffectType StandardOutlineEffect = ::TextAPI::StandardOutlineEffect;
const OutlineEffectType OpenVGFillAndStrokeOutlineEffect = ::TextAPI::OpenVGFillAndStrokeOutlineEffect;
const OutlineEffectType OpenVGStrokeOutlineEffect = ::TextAPI::OpenVGStrokeOutlineEffect;

typedef ::TextAPI::OutlineEffect OutlineEffect;

typedef ::TextAPI::HintingMode HintingMode;
const HintingMode DefaultHinting = ::TextAPI::DefaultHinting;
const HintingMode NoHinting = ::TextAPI::NoHinting;
const HintingMode LightAutoHinting = ::TextAPI::LightAutoHinting;
const HintingMode StandardAutoHinting = ::TextAPI::StandardAutoHinting;
const HintingMode StandardHinting = ::TextAPI::StandardHinting;

typedef ::TextAPI::HanStyle HanStyle;
const HanStyle UnspecifiedHanStyle = ::TextAPI::UnspecifiedHanStyle;
const HanStyle TraditionalHanStyle = ::TextAPI::TraditionalHanStyle;
const HanStyle SimplifiedHanStyle = ::TextAPI::SimplifiedHanStyle;
const HanStyle JapaneseHanStyle = ::TextAPI::JapaneseHanStyle;
const HanStyle KoreanHanStyle = ::TextAPI::KoreanHanStyle;

typedef ::TextAPI::FontSpec FontSpec;
typedef ::TextAPI::WordSpaceMode WordSpaceMode;
const WordSpaceMode WordSpaceNatural = ::TextAPI::WordSpaceNatural;
const WordSpaceMode WordSpaceOverride = ::TextAPI::WordSpaceOverride;
const WordSpaceMode WordSpaceAdd = ::TextAPI::WordSpaceAdd;
const WordSpaceMode WordSpaceMultiply = ::TextAPI::WordSpaceMultiply;

typedef ::TextAPI::WordSpace WordSpace;
typedef ::TextAPI::CaseTransform CaseTransform;
const CaseTransform NoCaseTransform = ::TextAPI::NoCaseTransform;
const CaseTransform LowerCaseTransform = ::TextAPI::LowerCaseTransform;
const CaseTransform TitleCaseTransform = ::TextAPI::TitleCaseTransform;
const CaseTransform UpperCaseTransform = ::TextAPI::UpperCaseTransform;
const CaseTransform CapitalizeCaseTransform = ::TextAPI::CapitalizeCaseTransform;

typedef ::TextAPI::TextOrder TextOrder;
const TextOrder NoTextOrder = ::TextAPI::NoTextOrder;
const TextOrder ReverseTextOrder = ::TextAPI::ReverseTextOrder;
const TextOrder AlreadyReversedTextOrder = ::TextAPI::AlreadyReversedTextOrder;
const TextOrder BidiTextOrder = ::TextAPI::BidiTextOrder;

typedef ::TextAPI::DrawParam DrawParam;
typedef ::TextAPI::TextMetrics TextMetrics;
typedef ::TextAPI::Handle Handle;
typedef ::TextAPI::GraphicsContextType GraphicsContextType;
const GraphicsContextType BitmapGraphicsContext = ::TextAPI::BitmapGraphicsContext;
const GraphicsContextType OpenVGGraphicsContext = ::TextAPI::OpenVGGraphicsContext;

typedef ::TextAPI::NativeGraphicsDisplay NativeGraphicsDisplay;
typedef ::TextAPI::NativeGraphicsContext NativeGraphicsContext;
typedef ::TextAPI::NativeGraphicsSurface NativeGraphicsSurface;
typedef ::TextAPI::GraphicsContext GraphicsContext;
typedef ::TextAPI::FontDataId FontDataId;
typedef ::TextAPI::FontMetrics FontMetrics;
typedef ::TextAPI::Font Font;
typedef ::TextAPI::Stream Stream;
typedef ::TextAPI::GlyphType GlyphType;
const GlyphType BitmapGlyphType = ::TextAPI::BitmapGlyphType;
const GlyphType OpenVGGlyphType = ::TextAPI::OpenVGGlyphType;

typedef ::TextAPI::TextPosRounding TextPosRounding;
const TextPosRounding TextPosRoundLeft = ::TextAPI::TextPosRoundLeft;
const TextPosRounding TextPosRoundNearest = ::TextAPI::TextPosRoundNearest;
const TextPosRounding TextPosRoundRight = ::TextAPI::TextPosRoundRight;
const TextPosRounding TextPosRoundPreceding = ::TextAPI::TextPosRoundPreceding;
const TextPosRounding TextPosRoundFollowing = ::TextAPI::TextPosRoundFollowing;

typedef ::TextAPI::AdvancedFontLoadingParam AdvancedFontLoadingParam;
typedef ::TextAPI::FontLoader FontLoader;
typedef ::TextAPI::FontLoaderDelegate FontLoaderDelegate;
typedef ::TextAPI::Engine Engine;
Engine* engine();
void setLoadAllFonts();

typedef ::TextAPI::BreakIteratorType BreakIteratorType;
const BreakIteratorType CharacterBreakIteratorType = ::TextAPI::CharacterBreakIteratorType;
const BreakIteratorType WordBreakIteratorType = ::TextAPI::WordBreakIteratorType;
const BreakIteratorType LineBreakIteratorType = ::TextAPI::LineBreakIteratorType;
const BreakIteratorType SentenceBreakIteratorType = ::TextAPI::SentenceBreakIteratorType;

typedef ::TextAPI::BreakIterator BreakIterator;

namespace Char {
typedef ::TextAPI::Char::BidirectionalType BidirectionalType;
typedef ::TextAPI::Char::DecompositionType DecompositionType;
typedef ::TextAPI::Char::Cat Cat;

static inline BidirectionalType bidirectionalType(int c) { return ::TextAPI::Char::bidirectionalType(c); }
static inline DecompositionType decompositionType(int c) { return ::TextAPI::Char::decompositionType(c); }
static inline Cat category(int c) { return ::TextAPI::Char::category(c); }
static inline bool isSpace(int c) { return ::TextAPI::Char::isSpace(c); }
static inline bool isLetter(int c) { return ::TextAPI::Char::isLetter(c); }
static inline bool isPrintable(int c) { return ::TextAPI::Char::isPrintable(c); }
static inline bool isUpper(int c) { return ::TextAPI::Char::isUpper(c); }
static inline bool isLower(int c) { return ::TextAPI::Char::isLower(c); }
static inline bool isPunct(int c) { return ::TextAPI::Char::isPunct(c); }
static inline bool isDigit(int c) { return ::TextAPI::Char::isDigit(c); }
static inline int toLower(int c) { return ::TextAPI::Char::toLower(c); }
static inline int toUpper(int c) { return ::TextAPI::Char::toUpper(c); }
static inline int toTitle(int c) { return ::TextAPI::Char::toTitle(c); }
static inline int foldCase(int c) { return ::TextAPI::Char::foldCase(c); }
static inline int mirroredForm(int c) { return ::TextAPI::Char::mirroredForm(c); }
static inline int combiningClass(int c) { return ::TextAPI::Char::combiningClass(c); }
}

}
}
}

#endif
