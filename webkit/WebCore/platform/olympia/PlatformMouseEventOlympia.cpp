/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "PlatformMouseEvent.h"

#include <wtf/CurrentTime.h>

namespace WebCore {

PlatformMouseEvent::PlatformMouseEvent(const IntPoint& eventPos, const IntPoint& globalPos, const MouseEventType type, int clickCount, MouseInputMethod method)
    : m_position(eventPos)
    , m_globalPosition(globalPos)
    , m_eventType(type)
    , m_clickCount(clickCount)
    , m_shiftKey(false)
    , m_ctrlKey(false)
    , m_altKey(false)
    , m_metaKey(false)
    , m_inputMethod(method)
{
    ASSERT(type != MouseEventScroll);

    m_timestamp = WTF::currentTime();
    m_button = (m_clickCount > 0) ? LeftButton : NoButton;
}

} // namespace WebCore
