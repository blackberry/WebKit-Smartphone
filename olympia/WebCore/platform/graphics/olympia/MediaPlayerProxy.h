/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef MediaPlayerProxy_h
#define MediaPlayerProxy_h

#include "ScriptInstance.h"
#include "WebPlugin.h"

namespace WebCore {
    class String;
    class MediaPlayer;
    class HTMLMediaElement;

    // Used by MediaPlayer
    enum MediaPlayerProxyNotificationType {
        MediaPlayerNotificationPlayPauseButtonPressed,
        Idle,
        Loading,
        Loaded,
        FormatError,
        NetworkError,
        DecodeError
    };

    class WebMediaPlayerProxy {
    public:
        WebMediaPlayerProxy(MediaPlayer* player);
        ~WebMediaPlayerProxy();

        MediaPlayer* mediaPlayer() {return m_mediaPlayer;}
        void initEngine();
        void load(const String& url);
        HTMLMediaElement* element();
        void invokeMethod(const String& methodName);
        ScriptInstance pluginInstance();
        void getPlugin();
        void play();
        void pause();

    private:
        MediaPlayer* m_mediaPlayer;
        bool m_init;
        ScriptInstance m_instance;
        RefPtr<Olympia::WebKit::WebPlugin> m_plugin;

    };

}

#endif

