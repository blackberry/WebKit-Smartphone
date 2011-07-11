/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "config.h"
#include "FontCustomPlatformData.h"

#include "AtomicString.h"
#include "FontDescription.h"
#include "FontPlatformData.h"
#include "SharedBuffer.h"

#include <string.h> // for memcpy()

namespace WebCore {

static int sharedBufferStreamGetLength(Olympia::Platform::Text::Stream& stream)
{
    return static_cast<SharedBuffer*>(stream.m_data)->size();
}

static int sharedBufferStreamRead(Olympia::Platform::Text::Stream& stream, int offset, unsigned char* dstBuffer, int count)
{
    const unsigned char* srcBuffer = reinterpret_cast<const unsigned char*>(
        static_cast<SharedBuffer*>(stream.m_data)->data());

    if (offset + count > sharedBufferStreamGetLength(stream))
        count = sharedBufferStreamGetLength(stream) - offset;

    memcpy(dstBuffer, srcBuffer + offset, count);

    return count; // FIXME: is that the correct return value?
}

static void sharedBufferStreamClose(Olympia::Platform::Text::Stream& stream)
{
}


FontCustomPlatformData::~FontCustomPlatformData()
{
    Olympia::Platform::Text::ReturnCode error = FontPlatformData::engine()->unloadFontData(m_fontDataId);
    ASSERT(!error);
}

FontPlatformData FontCustomPlatformData::fontPlatformData(int size, bool bold, bool italic, FontRenderingMode mode)
{
    String familyName("@CSS");
    familyName.append(String::number(m_id));

    FontDescription desc;
    desc.setSpecifiedSize(size);
    desc.setComputedSize(size);
    desc.setWeight(bold ? FontWeightBold : FontWeightNormal);
    desc.setItalic(italic ? true : false);

    return FontPlatformData(desc, familyName);
}

FontCustomPlatformData* createFontCustomPlatformData(SharedBuffer* buffer)
{
    static int customFontId = 1;

    Olympia::Platform::Text::Stream stream;
    stream.m_data = buffer;
    stream.m_read = sharedBufferStreamRead;
    stream.m_close = sharedBufferStreamClose;
    stream.m_getLength = sharedBufferStreamGetLength;

    Olympia::Platform::Text::FontDataId fontDataId;
    Olympia::Platform::Text::ReturnCode error;
    String familyName("@CSS");
    familyName.append(String::number(customFontId));
    familyName.append('\0');

    error = FontPlatformData::engine()->loadFontData(fontDataId, stream, familyName.characters());
    if (error)
        return 0;

    FontCustomPlatformData* fcpd = new FontCustomPlatformData();
    fcpd->m_fontDataId = fontDataId;
    fcpd->m_id = customFontId;
    ++customFontId;

    return fcpd;
}

bool FontCustomPlatformData::supportsFormat(const String& /* format */)
{
    return false;
}


}
