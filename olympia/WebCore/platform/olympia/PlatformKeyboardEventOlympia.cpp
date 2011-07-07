/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "PlatformKeyboardEvent.h"

#include "CString.h"
#include "NotImplemented.h"
#include "OlympiaPlatformKeyboardCodes.h"
#include "OlympiaPlatformKeyboardEvent.h"
#include "OlympiaPlatformMisc.h"
#include "WindowsKeyboardCodes.h"

namespace WebCore {

static String keyIdentifierForOlympiaCharacter(unsigned short character)
{
    switch (character) {
    case Olympia::Platform::KEY_ENTER:
    case Olympia::Platform::KEY_ENTERALT:
        return "Enter";
    case Olympia::Platform::KEY_BACKSPACE:
        return "U+0009";
    case Olympia::Platform::KEY_DELETE:
        return "U+007F";
    case Olympia::Platform::KEY_CONTROL_LEFT:
        return "Left";
    case Olympia::Platform::KEY_CONTROL_RIGHT:
        return "Right";
    case Olympia::Platform::KEY_CONTROL_UP:
        return "Up";
    case Olympia::Platform::KEY_CONTROL_DOWN:
        return "Down";
    case Olympia::Platform::KEY_ESCAPE:
        return "Escape";
    default:
        return String::format("U+%04X", WTF::toASCIIUpper(character));
    }
}

static int windowsKeyCodeForOlympiaCharacter(unsigned short character)
{
    switch (character) {
    case Olympia::Platform::KEY_ENTER:
    case Olympia::Platform::KEY_ENTERALT:
        return VK_RETURN; //(0D) Return key
    case Olympia::Platform::KEY_BACKSPACE:
        return VK_BACK; // (08) BACKSPACE key
    case Olympia::Platform::KEY_DELETE:
        return VK_DELETE; // (2E) DEL key
    case Olympia::Platform::KEY_CONTROL_LEFT:
        return VK_LEFT; // (25) LEFT ARROW key
    case Olympia::Platform::KEY_CONTROL_RIGHT:
        return VK_RIGHT; // (27) RIGHT ARROW key
    case Olympia::Platform::KEY_CONTROL_UP:
        return VK_UP; // (26) UP ARROW key
    case Olympia::Platform::KEY_CONTROL_DOWN:
        return VK_DOWN; // (28) DOWN ARROW key
    case Olympia::Platform::KEY_ESCAPE:
        return VK_ESCAPE;
    case Olympia::Platform::KEY_SPACE:
        return VK_SPACE;
    case '0':
        return VK_0;
    case '1':
        return VK_1;
    case '2':
        return VK_2;
    case '3':
        return VK_3;
    case '4':
        return VK_4;
    case '5':
        return VK_5;
    case '6':
        return VK_6;
    case '7':
        return VK_7;
    case '8':
        return VK_8;
    case '9':
        return VK_9;
    case 'a':
    case 'A':
        return VK_A;
    case 'b':
    case 'B':
        return VK_B;
    case 'c':
    case 'C':
        return VK_C;
    case 'd':
    case 'D':
        return VK_D;
    case 'e':
    case 'E':
        return VK_E;
    case 'f':
    case 'F':
        return VK_F;
    case 'g':
    case 'G':
        return VK_G;
    case 'h':
    case 'H':
        return VK_H;
    case 'i':
    case 'I':
        return VK_I;
    case 'j':
    case 'J':
        return VK_J;
    case 'k':
    case 'K':
        return VK_K;
    case 'l':
    case 'L':
        return VK_L;
    case 'm':
    case 'M':
        return VK_M;
    case 'n':
    case 'N':
        return VK_N;
    case 'o':
    case 'O':
        return VK_O;
    case 'p':
    case 'P':
        return VK_P;
    case 'q':
    case 'Q':
        return VK_Q;
    case 'r':
    case 'R':
        return VK_R;
    case 's':
    case 'S':
        return VK_S;
    case 't':
    case 'T':
        return VK_T;
    case 'u':
    case 'U':
        return VK_U;
    case 'v':
    case 'V':
        return VK_V;
    case 'w':
    case 'W':
        return VK_W;
    case 'x':
    case 'X':
        return VK_X;
    case 'y':
    case 'Y':
        return VK_Y;
    case 'z':
    case 'Z':
        return VK_Z;
    default:
        return 0;
    }
}

PlatformKeyboardEvent::PlatformKeyboardEvent(Olympia::Platform::KeyboardEvent event)
    : m_autoRepeat(false)
    , m_isKeypad(false)
    , m_shiftKey(event.shiftActive())
    , m_ctrlKey(false)
    , m_altKey(false)
    , m_metaKey(false)
{
    m_type = event.keyDown() ? KeyDown : KeyUp;
    m_keyIdentifier = keyIdentifierForOlympiaCharacter(event.character());
    m_windowsVirtualKeyCode = windowsKeyCodeForOlympiaCharacter(event.character());
    unsigned short character = event.character();
    // Convert from line feed to carriage return.
    if (character == 10)
        character = 13;
    m_text = String(&character, 1);
    m_unmodifiedText = m_text;

    Olympia::Platform::log(Olympia::Platform::LogLevelInfo, "Keyboard event received text=%lc, keyIdentifier=%s, windowsVirtualKeyCode=%d", event.character(), m_keyIdentifier.latin1().data(), m_windowsVirtualKeyCode);
}

bool PlatformKeyboardEvent::currentCapsLockState()
{
    notImplemented();
    return false;
}

void PlatformKeyboardEvent::disambiguateKeyDownEvent(PlatformKeyboardEvent::Type type, bool backwardCompatibilityMode)
{
    // Can only change type from KeyDown to RawKeyDown or Char, as we lack information for other conversions.
    ASSERT(m_type == KeyDown);
    m_type = type;

    if (backwardCompatibilityMode)
        return;

    if (type == RawKeyDown) {
        m_text = String();
        m_unmodifiedText = String();
    } else {
        m_keyIdentifier = String();
        m_windowsVirtualKeyCode = 0;
    }
}

void PlatformKeyboardEvent::getCurrentModifierState(bool& shiftKey, bool& ctrlKey, bool& altKey, bool& metaKey)
{
    notImplemented();
}

} // namespace WebCore
