/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef BlackBerryPlatformActivation_h
#define BlackBerryPlatformActivation_h

namespace BlackBerry {
namespace Platform {
    class ActivationListener {
    public:
        virtual void deactivate() = 0;
        virtual void activate() = 0;
    };

    class Activation {
    public:
        static void backlightStateChanged(bool backlightOn);
        static void applicationActivationStateChanged(bool applicationInForeground);
        static bool isInForeground();
        static void addListener(ActivationListener*);
    };
} // namespace Olympia
} // namespace Platform

#endif // BlackBerryPlatformActivation_h
