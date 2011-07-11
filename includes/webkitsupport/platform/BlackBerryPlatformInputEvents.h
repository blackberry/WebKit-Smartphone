/*
 * Copyright (C) Research In Motion Limited 2009. All rights reserved.
 */

#ifndef BlackBerryPlatformInputEvents_h
#define BlackBerryPlatformInputEvents_h

// FIXME This is for backwards compatibility with OlympiaPlatformInputEvents
// before refactoring
#include "BlackBerryPlatformTouchEvents.h"

namespace BlackBerry {
namespace Platform {

// NOTE:  Conversion between values in HTMLInputElement and these values is in WebPage.cpp and any changes made
// here will require a corresponding update.

// These enum values must match the corresponding constants in OlympiaConstants.java.
enum CaretHighlightStyle { HighlightCharacterCaret = 0, OutlineCharacterCaret, ThinVerticalCaret, NoCaret };

enum OlympiaInputType {
    InputTypeText = 0,
    InputTypePassword,
    InputTypeIsIndex,
    InputTypeSearch,
    InputTypeEmail,
    InputTypeNumber,
    InputTypeTelephone,
    InputTypeURL,
    InputTypeTextArea,
    InputTypeColor,
    InputTypeDate,
    InputTypeDateTime,
    InputTypeDateTimeLocal,
    InputTypeMonth,
    InputTypeTime,
    InputTypeWeek
};

// Define the Popup entry types.
enum PopupItem { TypeSeparator, TypeGroup, TypeOption };

// Scroll directions, this maps to WebCore::ScrollDirection defined in ScrollTypes.h.
// Currently used in link-to-link navigation.

enum ScrollDirection { ScrollUp, ScrollDown, ScrollLeft, ScrollRight };

// Event modes set by meta tags
enum CursorEventMode {
    ProcessedCursorEvents = 0,
    NativeCursorEvents,
};

enum TouchEventMode {
    ProcessedTouchEvents = 0,
    NativeTouchEvents,
    PureTouchEventsWithMouseConversion,
};

} // namespace Platform
} // namespace Olympia

#endif // BlackBerryPlatformInputEvents_h
