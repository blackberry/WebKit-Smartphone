/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "Logging.h"

#include "NotImplemented.h"

namespace WebCore {

static inline void initializeWithUserDefault(WTFLogChannel& channel, bool enabled)
{
    if(enabled)
        channel.state = WTFLogChannelOn;
    else
        channel.state = WTFLogChannelOff;
}

void InitializeLoggingChannelsIfNecessary()
{
    static bool haveInitializedLoggingChannels = false;
    if (haveInitializedLoggingChannels)
        return;
    haveInitializedLoggingChannels = true;

    initializeWithUserDefault(LogNotYetImplemented, true);
    initializeWithUserDefault(LogFrames, true);
    initializeWithUserDefault(LogLoading, true);
    initializeWithUserDefault(LogPopupBlocking, true);
    initializeWithUserDefault(LogEvents, true);
    initializeWithUserDefault(LogEditing, true);
    initializeWithUserDefault(LogLiveConnect, true);
    initializeWithUserDefault(LogIconDatabase, true);
    initializeWithUserDefault(LogSQLDatabase, true);
    initializeWithUserDefault(LogSpellingAndGrammar, true);
    initializeWithUserDefault(LogBackForward, true);
    initializeWithUserDefault(LogHistory, true);
    initializeWithUserDefault(LogPageCache, true);
    initializeWithUserDefault(LogPlatformLeaks, true);
    initializeWithUserDefault(LogNetwork, true);
    initializeWithUserDefault(LogFTP, true);
    initializeWithUserDefault(LogThreading, true);
    initializeWithUserDefault(LogStorageAPI, true);
    initializeWithUserDefault(LogMedia, true);
    initializeWithUserDefault(LogPlugins, true);
    initializeWithUserDefault(LogArchives, true);
    initializeWithUserDefault(LogProgress, true);
    initializeWithUserDefault(LogFileAPI, true);
}

} // namespace WebCore
