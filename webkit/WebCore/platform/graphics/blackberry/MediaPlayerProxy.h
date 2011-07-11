/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef MediaPlayerProxy_h
#define MediaPlayerProxy_h

#include "ScriptInstance.h"
#include "WebPlugin.h"

namespace WTF {
    class String;
}

namespace WebCore {
    class HTMLMediaElement;
    class IntSize;
    class MediaPlayerPrivate;
    
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
        WebMediaPlayerProxy(MediaPlayerPrivate* player);
        ~WebMediaPlayerProxy();

        MediaPlayerPrivate* mediaPlayer() {return m_mediaPlayer;}
        void initEngine();
        void load(const String& url);
        HTMLMediaElement* element();
        void invokeMethod(const String& methodName);
        ScriptInstance pluginInstance();
        PassRefPtr<Olympia::WebKit::WebPlugin> getPlugin();
        void play();
        void pause();

        float time();
        float duration();

        void setVolume(float volume);
        float volume();

        void setMuted(bool muted);
        bool muted();

        void setSize(const IntSize& size);

        bool autoplay();
        void setAutoplay(bool autoplay);

        void seek(float time);

        void readyStateChanged(int state);
        void volumeChanged(float volume);
        void durationChanged(float duration);

    private:
        MediaPlayerPrivate* m_mediaPlayer;
        bool m_init;
        ScriptInstance m_instance;
    };

}

#endif

