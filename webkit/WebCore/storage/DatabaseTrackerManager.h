/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef DatabaseTrackerManager_h
#define DatabaseTrackerManager_h

#if ENABLE(DATABASE)

#include "PlatformString.h"
#include "StringHash.h"
#include <wtf/HashTraits.h>
#include <wtf/HashMap.h>

namespace WebCore {

class DatabaseTracker;

class DatabaseTrackerManager : public Noncopyable {
public:
    void initializeTracker(const String& groupName, const String& databasePath);
    DatabaseTracker& tracker(const String& groupName);
#if OS(OLYMPIA)
    static void reopenAllTrackerDatabases();
    static void closeAllTrackerDatabases();
#endif // OS(OLYMPIA)

private:
    DatabaseTrackerManager();
    ~DatabaseTrackerManager();

    typedef HashMap<String, DatabaseTracker*> DatabaseTrackerMap;
    DatabaseTrackerMap m_trackers;

    DatabaseTracker* m_defaultTracker;

    friend DatabaseTrackerManager& databaseTrackerManager();
};

DatabaseTrackerManager& databaseTrackerManager();

} // namespace WebCore

#endif // ENABLE(DATABASE)

#endif // DatabaseTrackerManager_h
