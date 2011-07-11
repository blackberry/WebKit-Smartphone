/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef BlackBerryPlatformTextBreakIterators_h
#define BlackBerryPlatformTextBreakIterators_h

namespace BlackBerry {
    namespace Platform {

        class TextBreakIterator;
        const int TextBreakDone = -1;

        TextBreakIterator* characterBreakIterator(const unsigned short* text, int textLength);
        TextBreakIterator* cursorMovementIterator(const unsigned short* text, int textLength);
        TextBreakIterator* wordBreakIterator(const unsigned short* text, int textLength);
        TextBreakIterator* lineBreakIterator(const unsigned short* text, int textLength);
        TextBreakIterator* sentenceBreakIterator(const unsigned short* text, int textLength);

        int textBreakFirst(TextBreakIterator* iterator);
        int textBreakLast(TextBreakIterator* iterator);
        int textBreakNext(TextBreakIterator* iterator);
        int textBreakPrevious(TextBreakIterator* iterator);
        int textBreakCurrent(TextBreakIterator* iterator);
        int textBreakPreceding(TextBreakIterator* iterator, int posistion);
        int textBreakFollowing(TextBreakIterator* iterator, int posistion);
        bool isTextBreak(TextBreakIterator* iterator, int posistion);

    } // Platform
} // Olympia

#endif // BlackBerryPlatformTextBreakIterators_h
