/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef BlackBerryPlatformKeyboardCodes_h
#define BlackBerryPlatformKeyboardCodes_h

namespace BlackBerry {
    namespace Platform {

        // NOTE:  All values are derived from keypad.h and values must remain linked.

        const int KEY_BACKSPACE = 0x08;    ///< Backspace key.
        const int KEY_DELETE = 0x7F;    ///< Delete key.
        const int KEY_ESCAPE = 0x1B;    ///< Escape key.
        const int KEY_ENTER = 0x0D;    ///< Enter key.
        const int KEY_ENTERALT = 0x0A;    ///< Enter key - Alternate.
        const int KEY_SPACE = 0x20;    ///< Space key.
        const int KEY_HASH = 0x23;    ///< Hash (#) key.
        const int KEY_COMMA = 0x2C;    ///< Comma key.
        const int KEY_POINT = 0x2E;    ///< Point (.) key.
        const int KEY_QUESTION = 0x3F;    ///< Question mark key.
        const int KEY_MAGIC = 0x15;    ///< Magic key.
        const int KEY_CHAR_CONTEXT = 0x16;    ///< Key for displaying context sensitive alternate characters.
        const int KEY_SEND = 0x11;    ///< Send key.
        const int KEY_END = 0x12;    ///< End key.
        const int KEY_UNNAMED_MIDDLE = 0x13;    ///< Key between Send and End on Charm.
        const int KEY_NEXT = 0x14;    ///< Charm Next Key.
        const int KEY_TACTILE_1 = 209;     ///< Upper tactile key.
        const int KEY_TACTILE_2 = 210;     ///< Lower tactile key.
        const int KEY_CAMERA_FOCUS = 211;     ///< Camera focus key.
        const int KEY_SHIFT_R = 256;     ///< Right SHIFT key.
        const int KEY_ALT = 257;     ///< ALT key.
        const int KEY_SHIFT_L = 258;     ///< Left SHIFT key.
        const int KEY_BKLITE = 259;     ///< Backlight key.

        const int KEY_CONTROL_UP = 0x0081;     ///< Up key in a four-way keypad.
        const int KEY_CONTROL_RIGHT = 0x0084;     ///< Right key in a four-way keypad.
        const int KEY_CONTROL_DOWN = 0x0082;     ///< Down key in a four-way keypad.
        const int KEY_CONTROL_LEFT = 0x0083;     ///< Left key in a four-way keypad.

        // Modifiers

        const int ALT_STATUS = 0x0001;    ///< ALT key status.
        const int SHIFT_STATUS = 0x0002;    ///< SHIFT key status.
        const int CAPS_LOCK = 0x0004;    ///< CAPS LOCk key status
        const int KEY_HELD_WHILE_ROLLING = 0x0008; ///< Rolling state of the trackwheel.
        const int ALT_LOCK = 0x0010;    ///< ALT LOCK key status.
        const int SHIFT_STATUS_L = 0x0020; ///< Left Shift key status
        const int SHIFT_STATUS_R = 0x0040;  ///< Right SHIFT key status
        const int KEY_TURNED_BACKLIGHT_ON = 0x0080; ///< Last action activated backlight

        //Events
        const int KEY_DOWN = 0x0201;    ///< Key down event.
        const int KEY_REPEAT = 0x0202;   ///< Key repeat event.
        const int KEY_UP = 0x0203;    ///< Key up event.
        const int THUMB_CLICK = 0x0204;    ///< Thumbwheel press event.
        const int THUMB_UNCLICK = 0x0205;    ///< Thumbwheel release event.
        const int THUMB_ROLL_UP = 0x0206;    ///< Thumbwheel roll up event.
        const int THUMB_ROLL_DOWN = 0x0207;    ///< Thumbwheel roll down event.

    } // namespace Olympia
} // namespace Platform

#endif // BlackBerryPlatformKeyboardCodes_h
