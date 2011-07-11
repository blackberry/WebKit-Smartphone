/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef BlackBerryPlatformHibernation_h
#define BlackBerryPlatformHibernation_h

namespace BlackBerry {
namespace Platform {
    class HibernationListener {
    public:
        virtual void hibernate() = 0;
        virtual void awake() = 0;
    };

    class Hibernation {
    public:
        static void backlightStateChanged(bool backlightOn);
        static bool isBacklightOn();
        static bool isHibernating();
        static void addListener(HibernationListener*);
    };
} // namespace Olympia
} // namespace Platform

#endif // BlackBerryPlatformHibernation_h
