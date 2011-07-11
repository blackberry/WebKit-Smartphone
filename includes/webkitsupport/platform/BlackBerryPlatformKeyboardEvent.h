/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef BlackBerryPlatformKeyboardEvent_h
#define BlackBerryPlatformKeyboardEvent_h

namespace BlackBerry {
namespace Platform {
class KeyboardEvent {
    public:
        enum Type { KeyDown, KeyUp, KeyChar };
        KeyboardEvent(const unsigned short character, bool keyDown = true, bool shiftActive = false, bool altActive = false)
            : m_character(character),
              m_keyDown(keyDown),
              m_shiftActive(shiftActive),
              m_altActive(altActive)
            {}

        unsigned short character() { return m_character; }
        bool shiftActive() { return m_shiftActive; }
        bool altActive() { return m_altActive; }
        bool keyDown() { return m_keyDown; }
        void setKeyDown(bool down) { m_keyDown = down; }

    private:
        const unsigned short m_character;
        bool m_keyDown;
        bool m_shiftActive;
        bool m_altActive;
};
}
}

#endif // BlackBerryPlatformKeyboardEvent_h
