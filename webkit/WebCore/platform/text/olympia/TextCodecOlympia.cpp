/*
 * Copyright (C) 2007-2009 Torch Mobile, Inc. All rights reserved.
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#include "config.h"
#include "TextCodecOlympia.h"

#include "CString.h"
#include "OlympiaPlatformTextCodec.h"
#include "PlatformString.h"
#include <stdio.h>

namespace WebCore {

static PassOwnPtr<TextCodec> newTextCodecOlympia(const TextEncoding& encoding, const void* additionalData)
{
    return new TextCodecOlympia(reinterpret_cast<unsigned>(additionalData));
}

static Vector<Olympia::Platform::TextCodec::Encoding> gSupportedEncodings;

static inline void getSupportedEncodings()
{
    static bool hasBeenCalled = false;
    if (hasBeenCalled)
        return;
    hasBeenCalled = true;

    unsigned numEncodings = Olympia::Platform::TextCodec::getSupportedEncodings(0, 0);
    gSupportedEncodings.resize(numEncodings);
    numEncodings = Olympia::Platform::TextCodec::getSupportedEncodings(gSupportedEncodings.data(), gSupportedEncodings.size());
    ASSERT(numEncodings == gSupportedEncodings.size());
}

TextCodecOlympia::TextCodecOlympia(unsigned encodingID)
    : m_encodingID(encodingID)
{
}

TextCodecOlympia::~TextCodecOlympia()
{
}

void TextCodecOlympia::registerBaseEncodingNames(EncodingNameRegistrar registrar)
{
    getSupportedEncodings();
    for (unsigned i = 0; i < gSupportedEncodings.size(); ++i) {
        registrar(gSupportedEncodings[i].m_name, gSupportedEncodings[i].m_name);

        unsigned numAliases = Olympia::Platform::TextCodec::getAliases(gSupportedEncodings[i].m_id, 0, 0);
        Vector<const char*> aliases(numAliases);
        numAliases = Olympia::Platform::TextCodec::getAliases(gSupportedEncodings[i].m_id, aliases.data(), aliases.size());
        ASSERT(numAliases == aliases.size());

        for (unsigned j = 0; j < aliases.size(); ++j)
            registrar(aliases[j], gSupportedEncodings[i].m_name);
    }
}

void TextCodecOlympia::registerBaseCodecs(TextCodecRegistrar registrar)
{
    getSupportedEncodings();
    for (unsigned i = 0; i < gSupportedEncodings.size(); ++i)
        registrar(gSupportedEncodings[i].m_name, newTextCodecOlympia, reinterpret_cast<void*>(gSupportedEncodings[i].m_id));
}

void TextCodecOlympia::registerExtendedEncodingNames(EncodingNameRegistrar registrar)
{
}

void TextCodecOlympia::registerExtendedCodecs(TextCodecRegistrar registrar)
{
}

static void decode(Vector<UChar, 8192>& outputBuffer, unsigned encodingID, const char* bytes, size_t length, size_t* left, bool canBeFirstTime, bool& sawInvalidChar)
{
    if (!length) {
        *left = 0;
        return;
    }

    size_t oldSize = outputBuffer.size();
    outputBuffer.resize(oldSize + length);
    UChar* destinationStart = outputBuffer.data() + oldSize;
    const char* sourceStart = bytes;
    const char* const sourceEnd = bytes + length;

    for (;;) {
        Olympia::Platform::TextCodec::Result result = Olympia::Platform::TextCodec::decode(encodingID, sourceStart, sourceEnd, destinationStart, outputBuffer.data() + outputBuffer.size());

        // This should never happen
        ASSERT(result != Olympia::Platform::TextCodec::encodingNotSupported);

        if (result == Olympia::Platform::TextCodec::destinationPending) {
            oldSize = outputBuffer.size();
            outputBuffer.resize(oldSize + 256);
            destinationStart = outputBuffer.data() + oldSize;
            continue;
        }

        if (result != Olympia::Platform::TextCodec::successful)
            sawInvalidChar = true;

        break;
    }

    *left = sourceEnd - sourceStart;
    outputBuffer.resize(destinationStart - outputBuffer.data());
}

String TextCodecOlympia::decode(const char* bytes, size_t length, bool flush, bool stopOnError, bool& sawError)
{
    if (!m_decodeBuffer.isEmpty()) {
        m_decodeBuffer.append(bytes, length);
        bytes = m_decodeBuffer.data();
        length = m_decodeBuffer.size();
    }

    size_t left;
    Vector<UChar, 8192> outputBuffer;
    for (;;) {
        bool sawInvalidChar = false;
        WebCore::decode(outputBuffer, m_encodingID, bytes, length, &left, m_decodeBuffer.isEmpty(), sawInvalidChar);
        if (!left)
            break;

        if (!sawInvalidChar && !flush && left < 16)
            break;

        outputBuffer.append(L'?');
        sawError = true;
        if (stopOnError)
            return String(outputBuffer.data(), outputBuffer.size());


        if (left == 1)
            break;

        bytes += length - left + 1;
        length = left - 1;
    }
    if (left && !flush) {
        if (m_decodeBuffer.isEmpty())
            m_decodeBuffer.append(bytes + length - left, left);
        else {
            memmove(m_decodeBuffer.data(), bytes + length - left, left);
            m_decodeBuffer.resize(left);
        }
    } else
        m_decodeBuffer.clear();
    return String(outputBuffer.data(), outputBuffer.size());
}

CString TextCodecOlympia::encode(const UChar* characters, size_t length, UnencodableHandling errorHandling)
{
    if (!characters || !length)
        return CString();

    Vector<char> outputBuffer(length * 3);
    char* destinationStart = outputBuffer.data();
    const UChar* sourceStart = characters;
    const UChar* const sourceEnd = characters + length;
    while (sourceStart < sourceEnd) {
        Olympia::Platform::TextCodec::Result result = Olympia::Platform::TextCodec::encode(m_encodingID, sourceStart, sourceEnd, destinationStart, outputBuffer.data() + outputBuffer.size());

        // This should never happen
        ASSERT(result != Olympia::Platform::TextCodec::encodingNotSupported);

        if (result == Olympia::Platform::TextCodec::successful)
            break;

        if (result == Olympia::Platform::TextCodec::destinationPending) {
            unsigned oldSize = outputBuffer.size();
            outputBuffer.resize(oldSize + 256);
            destinationStart = outputBuffer.data() + oldSize;
            continue;
        }

        // Saw invalid character
        size_t charactersEncoded = destinationStart - outputBuffer.data();
        // See definition of URLEncodedEntitiesForUnencodables
        // The max encoded length can be 3 * 3 + 6 = 15 (16bit integer can be 6 digit in octal)
        static const unsigned maxEntityLength = 15;
        // We need an extra byte for null terminator
        if (outputBuffer.size() < charactersEncoded + maxEntityLength + 1) {
            outputBuffer.resize(charactersEncoded + 256);
            destinationStart = outputBuffer.data() + charactersEncoded;
        }
        switch (errorHandling) {
        case EntitiesForUnencodables:
            snprintf(destinationStart, outputBuffer.end() - destinationStart, "&#%o;", *sourceStart);
            while (*destinationStart && destinationStart < outputBuffer.end())
                ++destinationStart;
            break;
        case URLEncodedEntitiesForUnencodables:
            snprintf(destinationStart, outputBuffer.end() - destinationStart, "%%26%%23%o%%3B", *sourceStart);
            while (*destinationStart && destinationStart < outputBuffer.end())
                ++destinationStart;
            break;
        default:
            ASSERT(QuestionMarksForUnencodables == errorHandling);
            *destinationStart++ = '?';
            break;
        }
        ++sourceStart;
    }

    return CString(outputBuffer.data(), destinationStart - outputBuffer.data());
}

} // namespace WebCore
