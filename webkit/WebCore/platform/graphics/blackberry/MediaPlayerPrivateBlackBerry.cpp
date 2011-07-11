/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#include "config.h"

#if ENABLE(VIDEO)
#include "MediaPlayerPrivateBlackBerry.h"

#include "TimeRanges.h"
#include "WebSettings.h"
#include <wtf/HashSet.h>

#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
#include "MediaPlayerProxy.h"
#endif
using namespace std;

namespace WebCore {

MediaPlayerPrivateInterface* MediaPlayerPrivate::create(MediaPlayer* player)
{
    return new MediaPlayerPrivate(player);
}

void MediaPlayerPrivate::registerMediaEngine(MediaEngineRegistrar registrar)
{
    if (isAvailable())
        registrar(create, getSupportedTypes, supportsType);
}

MediaPlayerPrivate::MediaPlayerPrivate(MediaPlayer* player)
    : m_player(player)
    , m_networkState(MediaPlayer::Empty)
    , m_readyState(MediaPlayer::HaveNothing)
    , m_startedPlaying(false)
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    m_proxy = new WebMediaPlayerProxy(this);
#endif
}

MediaPlayerPrivate::~MediaPlayerPrivate()
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    delete m_proxy;
#endif
}

void MediaPlayerPrivate::load(const String& url)
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    m_proxy->load(url);
#endif
}

void MediaPlayerPrivate::prepareToPlay()
{
    play();
}

void MediaPlayerPrivate::play()
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    m_proxy->play();
#endif
}

void MediaPlayerPrivate::pause()
{
    m_startedPlaying = false;
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    m_proxy->pause();
#endif
}

float MediaPlayerPrivate::duration() const
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    return m_proxy->duration();
#else
    return 0;
#endif
}

PassRefPtr<TimeRanges> MediaPlayerPrivate::buffered() const
{
    // Not used in proxy mode, implement it later
    return 0;
}

float MediaPlayerPrivate::currentTime() const
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    return m_proxy->time();
#else
    return 0;
#endif
}

void MediaPlayerPrivate::seek(float time)
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    m_proxy->seek(time);
#endif
}

void MediaPlayerPrivate::setEndTime(float time)
{
    // Not used in proxy mode, implement it later
}

bool MediaPlayerPrivate::paused() const
{
    return !m_startedPlaying;
}

bool MediaPlayerPrivate::seeking() const
{
    // Not used in proxy mode, implement it later
    return false;
}

IntSize MediaPlayerPrivate::naturalSize() const
{
    // Not used in proxy mode, implement it later
    return IntSize(0, 0);
}

bool MediaPlayerPrivate::hasVideo() const
{
    // Not used in proxy mode, implement it later
    return false;
}

bool MediaPlayerPrivate::hasAudio() const
{
    // Not used in proxy mode, implement it later
    return false;
}

void MediaPlayerPrivate::setVolume(float volume)
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    m_proxy->setVolume(volume);
#endif    
}

float MediaPlayerPrivate::volume() const
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    return m_proxy->volume();
#else
    return 0;
#endif    
}

void MediaPlayerPrivate::setMuted(bool muted)
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    m_proxy->setMuted(muted);
#endif  
}

bool MediaPlayerPrivate::muted() const
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    return m_proxy->muted();
#else
    return false;
#endif  
}

void MediaPlayerPrivate::setRate(float rate)
{
    // Not used in proxy mode, implement it later
}

int MediaPlayerPrivate::dataRate() const
{
    // Not used in proxy mode, implement it later
    return 0;
}


float MediaPlayerPrivate::maxTimeSeekable() const
{
    // Not used in proxy mode, implement it later
    return 0;
}

unsigned MediaPlayerPrivate::bytesLoaded() const
{
    return 0.;
}

bool MediaPlayerPrivate::totalBytesKnown() const
{
    return totalBytes() > 0;
}

unsigned MediaPlayerPrivate::totalBytes() const
{
    // Not used in proxy mode, implement it later
    return 0;
}

void MediaPlayerPrivate::cancelLoad()
{
    // Not used in proxy mode, implement it later
}

void MediaPlayerPrivate::didEnd()
{
    // Not used in proxy mode, implement it later
}

void MediaPlayerPrivate::setSize(const IntSize& size)
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    m_proxy->setSize(size);
#endif   
}

void MediaPlayerPrivate::setVisible(bool b)
{
    // Not used in proxy mode, implement it later
}

void MediaPlayerPrivate::paint(GraphicsContext*, const IntRect&)
{
    // Not used in proxy mode, plugins take care painting itself
}

bool MediaPlayerPrivate::autoplay() const
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    return m_proxy->autoplay();
#else
    return false;
#endif 
}

void MediaPlayerPrivate::setAutoplay(bool autoplay)
{
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
    m_proxy->setAutoplay(autoplay);
#endif 
}

static HashSet<String> mimeTypeCache()
{
    DEFINE_STATIC_LOCAL(HashSet<String>, typeCache, ());
    static bool typeListInitialized = false;

    if (!typeListInitialized)
        typeListInitialized = true;

    return typeCache;
}

void MediaPlayerPrivate::getSupportedTypes(HashSet<String>& types)
{
    types = mimeTypeCache();
}

bool MediaPlayerPrivate::isAvailable()
{
    // Must return true to enable video/audio through proxy
    return true;
}

void MediaPlayerPrivate::setReadyState(int state)
{
    MediaPlayer::ReadyState newState = static_cast<MediaPlayer::ReadyState>(state);
    if (newState == m_readyState)
        return;

    m_readyState = newState;
    m_player->readyStateChanged();

    switch (m_readyState) {
        case MediaPlayer::HaveMetadata: 
        case MediaPlayer::HaveCurrentData: 
            m_networkState = MediaPlayer::Loading;
            break;
        case MediaPlayer::HaveEnoughData: 
        case MediaPlayer::HaveFutureData: 
            m_networkState = MediaPlayer::Loaded;
            m_startedPlaying = true;
            break;
    }
    m_player->networkStateChanged();
}

void MediaPlayerPrivate::volumeChanged(float volume)
{
    m_player->volumeChanged(volume);
}

void MediaPlayerPrivate::durationChanged(float duration)
{
    m_player->durationChanged();
}

MediaPlayer::SupportsType MediaPlayerPrivate::supportsType(const String& type, const String& codecs)
{
    // Fix me: currently we haven't got supported codecs from client, so just check MIMEType.
    if (Olympia::WebKit::WebSettings::isSupportedObjectMIMEType(type))
        return MediaPlayer::MayBeSupported;
    else
        return MediaPlayer::IsNotSupported;
}
#if ENABLE(PLUGIN_PROXY_FOR_VIDEO)
void MediaPlayerPrivate::setPoster(const String& url)
{
    // Need to implement it later
}
void MediaPlayerPrivate::deliverNotification(MediaPlayerProxyNotificationType)
{
    // Not used in proxy mode at the moment
}
void MediaPlayerPrivate::setMediaPlayerProxy(WebMediaPlayerProxy* proxy)
{
    m_proxy = proxy;
}
#endif
}

#endif

