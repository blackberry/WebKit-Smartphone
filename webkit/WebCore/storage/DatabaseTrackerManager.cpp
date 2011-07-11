/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"
#include "DatabaseTrackerManager.h"

#if ENABLE(DATABASE)

#include "DatabaseTracker.h"
#include "SecurityOrigin.h"
#include <wtf/text/CString.h>

namespace WebCore {

DatabaseTrackerManager::DatabaseTrackerManager()
    : m_defaultTracker(0)
{
}

DatabaseTrackerManager::~DatabaseTrackerManager()
{
    delete m_defaultTracker;

    for (DatabaseTrackerMap::iterator iter = m_trackers.begin(); iter != m_trackers.end(); ++iter)
        delete iter->second;

    m_trackers.clear();
}

void DatabaseTrackerManager::initializeTracker(const String& groupName, const String& databasePath)
{
    if (groupName.isEmpty()) {
        ASSERT(!m_defaultTracker);
        m_defaultTracker = new DatabaseTracker(databasePath);
    } else {
        ASSERT(!m_trackers.get(groupName));
        m_trackers.add(groupName, new DatabaseTracker(databasePath));
    }
}

DatabaseTracker& DatabaseTrackerManager::tracker(const String& groupName)
{
    if (groupName.isEmpty()) {
        if (!m_defaultTracker)
            m_defaultTracker = new DatabaseTracker("");

        return *m_defaultTracker;
    }

    DatabaseTracker* tracker = m_trackers.get(groupName);
    if (!tracker) {
        tracker = new DatabaseTracker("");
        m_trackers.add(groupName, tracker);
    }

    return *tracker;
}

DatabaseTrackerManager& databaseTrackerManager()
{
    DEFINE_STATIC_LOCAL(DatabaseTrackerManager, manager, ());

    return manager;
}

#if OS(OLYMPIA)
void DatabaseTrackerManager::reopenAllTrackerDatabases()
{
    DatabaseTrackerMap& trackers = databaseTrackerManager().m_trackers;
    DatabaseTrackerMap::iterator it = trackers.begin();
    for (DatabaseTrackerMap::iterator it = trackers.begin(); it != trackers.end(); ++it) {
        it->second->reopenTrackerDatabase();
    }
    if (databaseTrackerManager().m_defaultTracker)
        databaseTrackerManager().m_defaultTracker->reopenTrackerDatabase();
}
void DatabaseTrackerManager::closeAllTrackerDatabases()
{
    DatabaseTrackerMap& trackers = databaseTrackerManager().m_trackers;
    DatabaseTrackerMap::iterator it = trackers.begin();
    for (DatabaseTrackerMap::iterator it = trackers.begin(); it != trackers.end(); ++it) {
        it->second->closeTrackerDatabase();
    }
    if (databaseTrackerManager().m_defaultTracker)
        databaseTrackerManager().m_defaultTracker->closeTrackerDatabase();
}

#endif // OS(OLYMPIA)

} // namespace WebCore

#endif // ENABLE(DATABASE)
