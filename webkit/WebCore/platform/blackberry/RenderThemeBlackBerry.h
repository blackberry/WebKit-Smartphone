/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef RenderThemeOlympia_H
#define RenderThemeOlympia_H

#include "RenderTheme.h"
#include "OlympiaPlatformMisc.h"

namespace WebCore {
    class Gradient;

    class RenderThemeBlackBerry : public RenderTheme {
    public:
        static PassRefPtr<RenderTheme> create();
        virtual ~RenderThemeBlackBerry();

        virtual String extraDefaultStyleSheet();

        virtual bool supportsHover(const RenderStyle*) const { return true; }

        virtual double caretBlinkInterval() const;
        virtual void paintCaret(GraphicsContext*, const IntRect& caretRect, const Element* rootEditableElement);
        virtual void repaintCaret(RenderView*, const IntRect& caretRect, CaretVisibility);
        virtual void paintCaretMarker(GraphicsContext*, const FloatRect& caretRect, const Font&);
        virtual void adjustTextColorForCaretMarker(Color&) const;
        void setCaretHighlightStyle(Olympia::Platform::CaretHighlightStyle caretHighlightStyle) { m_caretHighlightStyle = caretHighlightStyle; }

        virtual void systemFont(int cssValueId, FontDescription&) const;
        virtual bool paintCheckbox(RenderObject*, const PaintInfo&, const IntRect&);
        virtual void setCheckboxSize(RenderStyle*) const;
        virtual bool paintRadio(RenderObject*, const PaintInfo&, const IntRect&);
        virtual void setRadioSize(RenderStyle*) const;
        virtual bool paintButton(RenderObject*, const PaintInfo&, const IntRect&);
        void calculateButtonSize(RenderStyle*) const;
        virtual void adjustMenuListStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
        virtual bool paintMenuListButton(RenderObject*, const PaintInfo&, const IntRect&);
        virtual void adjustSliderThumbSize(RenderObject* o) const;
        virtual bool paintSliderTrack(RenderObject*, const PaintInfo&, const IntRect&);
        virtual bool paintSliderThumb(RenderObject*, const PaintInfo&, const IntRect&);

        virtual Color platformFocusRingColor() const;
        virtual bool supportsFocusRing(const RenderStyle* style) const { return style->hasAppearance(); }

        virtual void adjustButtonStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
        virtual void adjustTextFieldStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
        virtual bool paintTextField(RenderObject*, const PaintInfo&, const IntRect&);

        virtual void adjustTextAreaStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
        virtual bool paintTextArea(RenderObject*, const PaintInfo&, const IntRect&);

        virtual void adjustSearchFieldStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
        virtual void adjustSearchFieldCancelButtonStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
        virtual bool paintSearchField(RenderObject*, const PaintInfo&, const IntRect&);
        virtual bool paintSearchFieldCancelButton(RenderObject*, const PaintInfo&, const IntRect&);

        virtual void adjustMenuListButtonStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
        virtual void adjustCheckboxStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
        virtual void adjustRadioStyle(CSSStyleSelector*, RenderStyle*, Element*) const;
        virtual bool paintMenuList(RenderObject*, const PaintInfo&, const IntRect&);

        virtual Color platformActiveSelectionBackgroundColor() const;
    private:
        RenderThemeBlackBerry();
        void setButtonStyle(RenderStyle* style) const;

        void paintMenuListButtonGradientAndArrow(GraphicsContext*, RenderObject*, IntRect buttonRect, const Path& clipPath);
        bool paintTextFieldOrTextAreaOrSearchField(RenderObject*, const PaintInfo&, const IntRect&);

        bool m_shouldRepaintVerticalCaret;
        Olympia::Platform::CaretHighlightStyle m_caretHighlightStyle;
        RefPtr<Range> m_oldCaretTextRange; // Used to determine whether we may have already added/painted the caret marker.
    };

}
#endif // RenderThemeOlympia_H
