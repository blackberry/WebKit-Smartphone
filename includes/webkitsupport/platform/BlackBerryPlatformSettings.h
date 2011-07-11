/*
 * Copyright (C) Research In Motion, Limited 2010. All rights reserved.
 */

#ifndef BlackBerryPlatformSettings_h
#define BlackBerryPlatformSettings_h

#include <string>

namespace BlackBerry {
    namespace Platform {
        class Settings {
        private:
            Settings();
            ~Settings() {};

        public:

            static Settings* get();

            bool isRSSFilteringEnabled() const { return m_rssFiltering; }
            void setRSSFilteringEnabled(bool enable) { m_rssFiltering = enable; }

            unsigned getSuggestedCacheCapacity(unsigned currentUsage) const;

            unsigned long secondaryThreadStackSize() const { return m_secondaryThreadStackSize; }
            void setSecondaryThreadStackSize(unsigned long size) { m_secondaryThreadStackSize = size; }

            unsigned maxPixelsPerDecodedImage() const { return m_maxPixelsPerDecodedImage; }

            bool shouldReportLowMemoryToUser() const { return m_shouldReportLowMemoryToUser; }
            void setShouldReportLowMemoryToUser(bool shouldReport) { m_shouldReportLowMemoryToUser = shouldReport; }

            unsigned numberOfBackingStoreTiles() const { return m_numberOfBackingStoreTiles; }

            unsigned maxNumberOfWorkers() const { return m_maxNumberOfWorkers; }
            void setMaxNumberOfWorkers(unsigned maxNumberOfWorkers) { m_maxNumberOfWorkers = maxNumberOfWorkers; }

            // Resource Path
            std::string resourceDirectory() const { return m_resourceDirectory; }
            void setResourceDirectory(const char* dir) { m_resourceDirectory = std::string(dir); }

        private:
            unsigned long m_secondaryThreadStackSize;
            unsigned m_maxPixelsPerDecodedImage;

            bool m_rssFiltering;

            bool m_shouldReportLowMemoryToUser;

            unsigned m_numberOfBackingStoreTiles;

            unsigned m_maxNumberOfWorkers;

            std::string m_resourceDirectory;
        };
    } // namespace Platform
} // namespace Olympia

#endif // BlackBerryPlatformSettings_h
