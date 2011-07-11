/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "TextBreakIterator.h"

#include "OlympiaPlatformTextBreakIterators.h"

namespace WebCore {

class TextBreakIterator {
public:
    Olympia::Platform::TextBreakIterator* m_iterator;
};

bool isTextBreak(TextBreakIterator* iterator, int position)
{
    return Olympia::Platform::isTextBreak(iterator->m_iterator, position);
}

int textBreakCurrent(TextBreakIterator* iterator)
{
    return Olympia::Platform::textBreakCurrent(iterator->m_iterator);
}

int textBreakFirst(TextBreakIterator* iterator)
{
    return Olympia::Platform::textBreakFirst(iterator->m_iterator);
}

int textBreakFollowing(TextBreakIterator* iterator, int position)
{
    return Olympia::Platform::textBreakFollowing(iterator->m_iterator, position);
}

int textBreakLast(TextBreakIterator* iterator)
{
    return Olympia::Platform::textBreakLast(iterator->m_iterator);
}

int textBreakNext(TextBreakIterator* iterator)
{
    return Olympia::Platform::textBreakNext(iterator->m_iterator);
}

int textBreakPrevious(TextBreakIterator* iterator)
{
    return Olympia::Platform::textBreakPrevious(iterator->m_iterator);
}

int textBreakPreceding(TextBreakIterator* iterator, int position)
{
    return Olympia::Platform::textBreakPreceding(iterator->m_iterator, position);
}

TextBreakIterator* characterBreakIterator(unsigned short const* text, int textLength)
{
    static TextBreakIterator iterator;
    iterator.m_iterator = Olympia::Platform::characterBreakIterator(text, textLength);

    return &iterator;
}

TextBreakIterator* cursorMovementIterator(unsigned short const* text, int textLength)
{
    static TextBreakIterator iterator;
    iterator.m_iterator = Olympia::Platform::cursorMovementIterator(text, textLength);

    return &iterator;
}

TextBreakIterator* lineBreakIterator(unsigned short const* text, int textLength)
{
    static TextBreakIterator iterator;
    iterator.m_iterator = Olympia::Platform::lineBreakIterator(text, textLength);

    return &iterator;
}

TextBreakIterator* sentenceBreakIterator(unsigned short const* text, int textLength)
{
    static TextBreakIterator iterator;
    iterator.m_iterator = Olympia::Platform::sentenceBreakIterator(text, textLength);

    return &iterator;
}

TextBreakIterator* wordBreakIterator(unsigned short const* text, int textLength)
{
    static TextBreakIterator iterator;
    iterator.m_iterator = Olympia::Platform::wordBreakIterator(text, textLength);

    return &iterator;
}

} // namespace WebCore
