/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "FileChooser.h"

#include "FileSystem.h"
#include "LocalizedStrings.h"
#include "StringTruncator.h"

using WTF::String;
namespace WebCore {

String FileChooser::basenameForWidth(const Font& font, int width) const
{
    if (width <= 0)
        return String();

    String string;
    if (m_filenames.isEmpty())
        string = fileButtonNoFileSelectedLabel();
    else if (m_filenames.size() == 1)
        string = m_filenames[0];
    else
        string = m_filenames[0] + ", ...";

    return StringTruncator::centerTruncate(string, static_cast<float>(width), font, false);
}

} // namespace WebCore
