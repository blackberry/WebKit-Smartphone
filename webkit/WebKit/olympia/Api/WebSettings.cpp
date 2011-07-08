/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "WebSettings.h"
#include "WebSettings_p.h"

#include "ApplicationCacheStorage.h"
#include "Base64.h"
#include "CString.h"
#include "Database.h"
#include "DatabaseSync.h"
#include "DatabaseTrackerManager.h"
#include "HashMap.h"
#include "OlympiaPlatformSettings.h"
#include "PlatformString.h"
#include "Settings.h"

#include "StringHash.h"
#include "StringPattern.h"
#include <wtf/HashSet.h>
#include <wtf/Vector.h>
#include <wtf/UnusedParam.h>

#define COLOR_WHITE 0xFFFFFFFF

using namespace WebCore;

namespace Olympia {
namespace WebKit {

enum WebSetting {
    AllSettings,
    ArePluginsEnabled,
    CanJavaScriptOpenWindowsAutomatically,
    DefaultFixedFontSize,
    DefaultFontSize,
    DefaultTextEncodingName,
    DetectedFormats,
    DoesGetFocusNodeContext,
    FixedFontFamily,
    IsGeolocationEnabled,
    IsJavaEnabled,
    IsJavaScriptEnabled,
    IsScrollbarsEnabled,
    IsZoomToFitOnLoad,
    LoadsImagesAutomatically,
    MinimumFontSize,
    SansSerifFontFamily,
    SerifFontFamily,
    ShouldDrawBorderWhileLoadingImages,
    ShouldUpdateTextReflowMode,
    StandardFontFamily,
    UserAgentString,
    UserStyleSheet,
    AllowCrossSiteRequests,
    AreLinksHandledExternally,
    UserScalable,
    InitialScale,
    ViewportWidth,
    FirstScheduledLayoutDelay,
    ShouldHandlePatternUrls,
    IsCookieCacheEnabled,
    IsDatabasesEnabled,
    IsLocalStorageEnabled,
    IsOfflineWebApplicationCacheEnabled,
    LocalStorageQuota,
    LocalStoragePath,
    IsEmailMode,
    UseWebKitCache,
    ShouldRenderAnimationsOnScroll,
    OverZoomColor,
    IsFrameFlatteningEnabled
};

typedef WTF::HashMap<WebCore::String, WebSettings*> WebSettingsMap;

static WebSettingsMap& settingsMap() {
    static WebSettingsMap& s_map = *(new WebSettingsMap);

    return s_map;
}

struct WebCoreSettingsState {
    WebCoreSettingsState()
        : loadsImagesAutomatically(true)
        , shouldDrawBorderWhileLoadingImages(false)
        , isJavaScriptEnabled(true)
        , isDatabasesEnabled(false)
        , isLocalStorageEnabled(false)
        , isOfflineWebApplicationCacheEnabled(false)
        , canJavaScriptOpenWindowsAutomatically(false)
        , arePluginsEnabled(false)
        , isJavaEnabled(false)
        , isFrameFlatteningEnabled(false)
        , useWebKitCache(true)
        , defaultFixedFontSize(13)
        , defaultFontSize(16)
        , minimumFontSize(8)
        , localStorageQuota(1024*1024*5)
        , serifFontFamily("Times New Roman")
        , fixedFontFamily("Courier New")
        , sansSerifFontFamily("Arial")
        , standardFontFamily("Times New Roman")
        , defaultTextEncodingName("iso-8859-1")
        , firstScheduledLayoutDelay(250) // match Document.cpp cLayoutScheduleThreshold by default
    {
#if ENABLE(BROKEN_DOM)
        detectedFormats.append("telephone");
#endif
    }

    ~WebCoreSettingsState()
    {
    }

    // These are overrides of WebCore::Settings defaults and are only here,
    // because our defaults differ from the WebCore::Settings defaults.
    // If the WebCore::Settings defaults change we need to re-evaluate whether
    // these are needed so we don't store more state than necessary!
    bool loadsImagesAutomatically;
    bool shouldDrawBorderWhileLoadingImages;
    bool isJavaScriptEnabled;
    bool isDatabasesEnabled;
    bool isLocalStorageEnabled;
    bool isOfflineWebApplicationCacheEnabled;
    bool canJavaScriptOpenWindowsAutomatically;
    bool arePluginsEnabled;
    bool isJavaEnabled;
    bool useWebKitCache;
    bool doesGetFocusNodeContext;
    bool isFrameFlatteningEnabled;

    int defaultFixedFontSize;
    int defaultFontSize;
    int minimumFontSize;
    int firstScheduledLayoutDelay;
    unsigned long long localStorageQuota;

    WebCore::String serifFontFamily;
    WebCore::String fixedFontFamily;
    WebCore::String sansSerifFontFamily;
    WebCore::String standardFontFamily;
    WebCore::String defaultTextEncodingName;
    WebCore::String localStoragePath;
    WebCore::String databasePath;
    WebCore::String appCachePath;
    WebCore::String pageGroupName;

    Vector<WebCore::String> detectedFormats;

    KURL userStyleSheet;
};

struct OlympiaSettingsState {
    OlympiaSettingsState()
        : isZoomToFitOnLoad(true)
        , doesGetFocusNodeContext(false)
        , isScrollbarsEnabled(true)
        , isGeolocationEnabled(false)
        , areLinksHandledExternally(false)
        , shouldHandlePatternUrls(false)
        , isCookieCacheEnabled(false)
        , textReflowMode(WebSettings::TextReflowEnabledOnlyForBlockZoom) // FIXME: We should detect whether we are embedded in a browser or an email client and default to TextReflowEnabledOnlyForBlockZoom and TextReflowEnabled, respectively.
        , userScalable(true)
        , initialScale(-1)
        , viewportWidth(0)
        , allowCrossSiteRequests(false)
        , isEmailMode(false)
        , shouldRenderAnimationsOnScroll(true)
        , overZoomColor(COLOR_WHITE)
    {
    }

    ~OlympiaSettingsState()
    {
    }

    // All settings that are Olympia specific and do not have a corresponding setting
    // in WebCore::Settings can store state here!
    WebCore::String userAgentString;
    bool isZoomToFitOnLoad;
    bool doesGetFocusNodeContext;
    bool isScrollbarsEnabled;
    bool isGeolocationEnabled;
    bool areLinksHandledExternally;
    WebSettings::TextReflowMode textReflowMode;
    bool userScalable;
    double initialScale;
    int viewportWidth;
    bool shouldHandlePatternUrls;
    bool isCookieCacheEnabled;
    bool allowCrossSiteRequests;
    bool isEmailMode;
    bool shouldRenderAnimationsOnScroll;
    int overZoomColor;
};

struct MIMETypeAssociationMap {
    const char* mimeType;
    const char* registerType;
};

static const MIMETypeAssociationMap mimeTypeAssociationMap[] = {
    { "image/x-ms-bmp", "image/bmp" },
    { "image/x-windows-bmp", "image/bmp" },
    { "image/x-bmp", "image/bmp" },
    { "image/x-bitmap", "image/bmp" },
    { "image/x-ms-bitmap", "image/bmp" },
    { "image/jpg", "image/jpeg" },
    { "image/pjpeg", "image/jpeg" },
    { "image/x-png", "image/png" },
    { "image/vnd.rim.png", "image/png" },
    { "image/ico", "image/vnd.microsoft.icon" },
    { "image/icon", "image/vnd.microsoft.icon" },
    { "text/ico", "image/vnd.microsoft.icon" },
    { "application/ico", "image/vnd.microsoft.icon" },
    { "image/x-icon", "image/vnd.microsoft.icon" },
    { "audio/vnd.qcelp", "audio/qcelp" },
    { "audio/qcp", "audio/qcelp" },
    { "audio/vnd.qcp", "audio/qcelp" },
    { "audio/wav", "audio/x-wav"},
    { "audio/mid", "audio/midi" },
    { "audio/sp-midi", "audio/midi" },
    { "audio/x-mid", "audio/midi" },
    { "audio/x-midi", "audio/midi" },
    { "audio/x-mpeg", "audio/mpeg" },
    { "audio/mp3", "audio/mpeg" },
    { "audio/x-mp3", "audio/mpeg" },
    { "audio/mpeg3", "audio/mpeg" },
    { "audio/x-mpeg3", "audio/mpeg" },
    { "audio/mpg3", "audio/mpeg" },
    { "audio/mpg", "audio/mpeg" },
    { "audio/x-mpg", "audio/mpeg" },
    { "audio/m4a", "audio/mp4" },
    { "audio/x-m4a", "audio/mp4" },
    { "audio/x-mp4", "audio/mp4" },
    { "audio/x-aac", "audio/aac" },
    { "audio/x-amr", "audio/amr" },
    { "audio/mpegurl", "audio/x-mpegurl" },
    { "video/3gp", "video/3gpp" },
    { "video/avi", "video/x-msvideo" },
    { "video/x-m4v", "video/mp4" },
    { "video/x-quicktime", "video/quicktime" },
    { "application/java", "application/java-archive" },
    { "application/x-java-archive", "application/java-archive" },
    { "application/x-zip-compressed", "application/zip" },
    { 0, 0 }
};

static HashSet<WebCore::String>* s_supportedObjectMIMETypes;

WebSettingsPrivate::WebSettingsPrivate(const String& pageGroupName)
    : m_settings(0)
    , m_webCoreSettingsState(new WebCoreSettingsState)
    , m_olympiaSettingsState(new OlympiaSettingsState)
    , m_screenWidth(0)
    , m_screenHeight(0)
{
    m_webCoreSettingsState->pageGroupName = pageGroupName;
}

WebSettingsPrivate::WebSettingsPrivate(WebCore::Settings* settings, const String& pageGroupName)
    : m_settings(settings)
    , m_webCoreSettingsState(new WebCoreSettingsState)
    , m_olympiaSettingsState(new OlympiaSettingsState)
{
    m_webCoreSettingsState->pageGroupName = pageGroupName;

    // Add ourselves to the list of page settings
    WebSettings::pageGroupSettings(pageGroupName)->d->m_pageSettings.append(this);

    // Initialize our settings to the global defaults
    apply(AllSettings, true /*global*/);
}

WebSettingsPrivate::~WebSettingsPrivate()
{
    if (!isGlobal()) {
        size_t pos = WebSettings::pageGroupSettings(m_webCoreSettingsState->pageGroupName)->d->m_pageSettings.find(this);
        WebSettings::pageGroupSettings(m_webCoreSettingsState->pageGroupName)->d->m_pageSettings.remove(pos);
    }

    delete m_webCoreSettingsState;
    delete m_olympiaSettingsState;
    m_webCoreSettingsState = 0;
    m_olympiaSettingsState = 0;

    m_settings = 0; // FIXME: We don't own this so perhaps it should be a RefPtr?
}

void WebSettingsPrivate::apply(int setting, bool global)
{
    if (isGlobal()) {
        // Cycle through the page settings and apply the defaults
        for (size_t i = 0; i < m_pageSettings.size(); ++i)
            m_pageSettings.at(i)->apply(setting, global);
    } else {
        // Apply the WebCore::Settings to this page's settings
        WebCoreSettingsState* webcoreSettingsState = global ? WebSettings::pageGroupSettings(m_webCoreSettingsState->pageGroupName)->d->m_webCoreSettingsState : m_webCoreSettingsState;

        if (setting == AllSettings || setting == LoadsImagesAutomatically)
            m_settings->setLoadsImagesAutomatically(webcoreSettingsState->loadsImagesAutomatically);

        if (setting == AllSettings || setting == ShouldDrawBorderWhileLoadingImages)
            m_settings->setShouldDrawBorderWhileLoadingImages(webcoreSettingsState->shouldDrawBorderWhileLoadingImages);

        if (setting == AllSettings || setting == IsJavaScriptEnabled)
            m_settings->setJavaScriptEnabled(webcoreSettingsState->isJavaScriptEnabled);

        if (setting == AllSettings || setting == DefaultFixedFontSize)
            m_settings->setDefaultFixedFontSize(webcoreSettingsState->defaultFixedFontSize);

        if (setting == AllSettings || setting == DefaultFontSize)
            m_settings->setDefaultFontSize(webcoreSettingsState->defaultFontSize);

        if (setting == AllSettings || setting == MinimumFontSize)
            m_settings->setMinimumFontSize(webcoreSettingsState->minimumFontSize);

        if (setting == AllSettings || setting == SerifFontFamily)
            m_settings->setSerifFontFamily(webcoreSettingsState->serifFontFamily);

        if (setting == AllSettings || setting == FixedFontFamily)
            m_settings->setFixedFontFamily(webcoreSettingsState->fixedFontFamily);

        if (setting == AllSettings || setting == SansSerifFontFamily)
            m_settings->setSansSerifFontFamily(webcoreSettingsState->sansSerifFontFamily);

        if (setting == AllSettings || setting == StandardFontFamily)
            m_settings->setStandardFontFamily(webcoreSettingsState->standardFontFamily);

        if (setting == AllSettings || setting == DetectedFormats)
            m_settings->setDetectedFormats(webcoreSettingsState->detectedFormats);

        if (setting == AllSettings || setting == CanJavaScriptOpenWindowsAutomatically)
            m_settings->setJavaScriptCanOpenWindowsAutomatically(webcoreSettingsState->canJavaScriptOpenWindowsAutomatically);

        if (setting == AllSettings || setting == ArePluginsEnabled)
            m_settings->setPluginsEnabled(webcoreSettingsState->arePluginsEnabled);

        if (setting == AllSettings || setting == DefaultTextEncodingName)
            m_settings->setDefaultTextEncodingName(webcoreSettingsState->defaultTextEncodingName);

        if (setting == AllSettings || setting == UserStyleSheet)
            m_settings->setUserStyleSheetLocation(webcoreSettingsState->userStyleSheet);

        if (setting == AllSettings || setting == FirstScheduledLayoutDelay)
            m_settings->setFirstScheduledLayoutDelay(webcoreSettingsState->firstScheduledLayoutDelay);

        if (setting == AllSettings || setting == UseWebKitCache)
            m_settings->setUseCache(webcoreSettingsState->useWebKitCache);

#if ENABLE(DATABASE)
        if (setting == AllSettings || setting == LocalStoragePath)
            m_settings->setLocalStorageDatabasePath(webcoreSettingsState->localStoragePath);

        if (setting == AllSettings || setting == IsDatabasesEnabled) {
            WebCore::Database::setIsAvailable(webcoreSettingsState->isDatabasesEnabled);
            WebCore::DatabaseSync::setIsAvailable(webcoreSettingsState->isDatabasesEnabled);
        }

        if (setting == AllSettings || setting == IsLocalStorageEnabled)
            m_settings->setLocalStorageEnabled(webcoreSettingsState->isLocalStorageEnabled);

        if (setting == AllSettings || setting == IsOfflineWebApplicationCacheEnabled)
            m_settings->setOfflineWebApplicationCacheEnabled(webcoreSettingsState->isOfflineWebApplicationCacheEnabled);

        if (setting == AllSettings || setting == LocalStorageQuota)
            m_settings->setLocalStorageQuota(webcoreSettingsState->localStorageQuota);
#endif

        if (setting == AllSettings || setting == IsFrameFlatteningEnabled)
            m_settings->setFrameFlatteningEnabled(webcoreSettingsState->isFrameFlatteningEnabled);

        // These are *NOT* exposed via the API so just always set them if this
        // is global and we're setting all the settings...
        if (setting == AllSettings && global) {
            m_settings->setJavaEnabled(webcoreSettingsState->isJavaEnabled);
        }

        // Apply the Olympia settings to this page's settings
        OlympiaSettingsState* olympiaSettingsState = global ? WebSettings::pageGroupSettings(m_webCoreSettingsState->pageGroupName)->d->m_olympiaSettingsState : m_olympiaSettingsState;

        if (setting == AllSettings || setting == UserAgentString)
            m_olympiaSettingsState->userAgentString = olympiaSettingsState->userAgentString;

        if (setting == AllSettings || setting == IsZoomToFitOnLoad)
            m_olympiaSettingsState->isZoomToFitOnLoad = olympiaSettingsState->isZoomToFitOnLoad;

        if (setting == AllSettings || setting == IsScrollbarsEnabled)
            m_olympiaSettingsState->isScrollbarsEnabled = olympiaSettingsState->isScrollbarsEnabled;

        if (setting == AllSettings || setting == IsGeolocationEnabled)
            m_olympiaSettingsState->isGeolocationEnabled = olympiaSettingsState->isGeolocationEnabled;

        if (setting == AllSettings || setting == DoesGetFocusNodeContext)
            m_olympiaSettingsState->doesGetFocusNodeContext = olympiaSettingsState->doesGetFocusNodeContext;

        if (setting == AllSettings || setting == AreLinksHandledExternally)
            m_olympiaSettingsState->areLinksHandledExternally = olympiaSettingsState->areLinksHandledExternally;

        if (setting == AllSettings || setting == ShouldUpdateTextReflowMode) {
            m_olympiaSettingsState->textReflowMode = olympiaSettingsState->textReflowMode;
            m_settings->setTextReflowEnabled(m_olympiaSettingsState->textReflowMode == WebSettings::TextReflowEnabled);
        }

        if (setting == AllSettings || setting == IsEmailMode) {
            m_olympiaSettingsState->isEmailMode = olympiaSettingsState->isEmailMode;
            // FIXME: See RIM Bug #746.
            // We don't want HTMLTokenizer to yield for anything other than email
            // case because call to currentTime() is too expensive at currentTime() :(
            m_settings->setShouldUseFirstScheduledLayoutDelay(m_olympiaSettingsState->isEmailMode);
            m_settings->setProcessHTTPEquiv(!m_olympiaSettingsState->isEmailMode);
        }

        if (setting == AllSettings || setting == ShouldRenderAnimationsOnScroll)
            m_olympiaSettingsState->shouldRenderAnimationsOnScroll = olympiaSettingsState->shouldRenderAnimationsOnScroll;

        if (setting == AllSettings || setting == OverZoomColor)
            m_olympiaSettingsState->overZoomColor = olympiaSettingsState->overZoomColor;

        // BrowserField2 settings
        if ( setting == AllSettings || setting == AllowCrossSiteRequests )
            m_olympiaSettingsState->allowCrossSiteRequests = olympiaSettingsState->allowCrossSiteRequests;
            
        if ( setting == AllSettings || setting == UserScalable )
            m_olympiaSettingsState->userScalable = olympiaSettingsState->userScalable;

        if ( setting == AllSettings || setting == InitialScale )
            m_olympiaSettingsState->initialScale = olympiaSettingsState->initialScale;

        if ( setting == AllSettings || setting == ViewportWidth )
            m_olympiaSettingsState->viewportWidth = olympiaSettingsState->viewportWidth;

        if ( setting == AllSettings || setting == ShouldHandlePatternUrls )
            m_olympiaSettingsState->shouldHandlePatternUrls = olympiaSettingsState->shouldHandlePatternUrls;

        if ( setting == AllSettings || setting == IsCookieCacheEnabled )
            m_olympiaSettingsState->isCookieCacheEnabled = olympiaSettingsState->isCookieCacheEnabled;
    }
}

WebSettings* WebSettings::pageGroupSettings(const String& pageGroupName)
{
    if (pageGroupName.isEmpty())
        return 0;

    WebSettingsMap& map = settingsMap();
    WebSettingsMap::iterator iter = map.find(pageGroupName);
    if (iter != map.end())
        return iter->second;

    WebSettings* setting = new WebSettings(pageGroupName);
    map.add(pageGroupName, setting);
    return setting;
}

WebSettings::WebSettings(const String& pageGroupName)
    : d(new WebSettingsPrivate(pageGroupName))
{
}

WebSettings::WebSettings(WebCore::Settings* settings, const String& pageGroupName)
    : d(new WebSettingsPrivate(settings, pageGroupName))
{
}

WebSettings::~WebSettings()
{
    delete d;
    d = 0;
}

// Images
bool WebSettings::loadsImagesAutomatically() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->loadsImagesAutomatically : d->m_settings->loadsImagesAutomatically();
}

void WebSettings::setLoadsImagesAutomatically(bool b)
{
    d->m_webCoreSettingsState->loadsImagesAutomatically = b;
    d->apply(LoadsImagesAutomatically, d->isGlobal());
}

bool WebSettings::shouldDrawBorderWhileLoadingImages() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->shouldDrawBorderWhileLoadingImages : d->m_settings->shouldDrawBorderWhileLoadingImages();
}

void WebSettings::setShouldDrawBorderWhileLoadingImages(bool b)
{
    d->m_webCoreSettingsState->shouldDrawBorderWhileLoadingImages = b;
    d->apply(ShouldDrawBorderWhileLoadingImages, d->isGlobal());
}

// JavaScript
bool WebSettings::isJavaScriptEnabled() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->isJavaScriptEnabled : d->m_settings->isJavaScriptEnabled();
}

void WebSettings::setJavaScriptEnabled(bool b)
{
    d->m_webCoreSettingsState->isJavaScriptEnabled = b;
    d->apply(IsJavaScriptEnabled, d->isGlobal());
}

// Font sizes
int WebSettings::defaultFixedFontSize() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->defaultFixedFontSize : d->m_settings->defaultFixedFontSize();
}

void WebSettings::setDefaultFixedFontSize(int i)
{
    d->m_webCoreSettingsState->defaultFixedFontSize = i;
    d->apply(DefaultFixedFontSize, d->isGlobal());
}

int WebSettings::defaultFontSize() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->defaultFontSize : d->m_settings->defaultFontSize();
}

void WebSettings::setDefaultFontSize(int i)
{
    d->m_webCoreSettingsState->defaultFontSize = i;
    d->apply(DefaultFontSize, d->isGlobal());
}

int WebSettings::minimumFontSize() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->minimumFontSize : d->m_settings->minimumFontSize();
}

void WebSettings::setMinimumFontSize(int i)
{
    d->m_webCoreSettingsState->minimumFontSize = i;
    d->apply(MinimumFontSize, d->isGlobal());
}

// Font families
WebCore::String WebSettings::serifFontFamily() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->serifFontFamily : d->m_settings->serifFontFamily().string();
}

void WebSettings::setSerifFontFamily(const char* s)
{
    d->m_webCoreSettingsState->serifFontFamily = s;
    d->apply(SerifFontFamily, d->isGlobal());
}

WebCore::String WebSettings::fixedFontFamily() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->fixedFontFamily : d->m_settings->fixedFontFamily().string();
}

void WebSettings::setFixedFontFamily(const char* s)
{
    d->m_webCoreSettingsState->fixedFontFamily = s;
    d->apply(FixedFontFamily, d->isGlobal());
}

WebCore::String WebSettings::sansSerifFontFamily() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->sansSerifFontFamily : d->m_settings->sansSerifFontFamily().string();
}

void WebSettings::setSansSerifFontFamily(const char* s)
{
    d->m_webCoreSettingsState->sansSerifFontFamily = s;
    d->apply(SansSerifFontFamily, d->isGlobal());
}

WebCore::String WebSettings::standardFontFamily() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->standardFontFamily : d->m_settings->standardFontFamily().string();
}

void WebSettings::setStandardFontFamily(const char* s)
{
    d->m_webCoreSettingsState->standardFontFamily = s;
    d->apply(StandardFontFamily, d->isGlobal());
}

bool WebSettings::isFrameFlatteningEnabled() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->isFrameFlatteningEnabled : d->m_settings->frameFlatteningEnabled();
}

void WebSettings::setFrameFlatteningEnabled(bool enable)
{
    d->m_webCoreSettingsState->isFrameFlatteningEnabled = enable;
    d->apply(IsFrameFlatteningEnabled, d->isGlobal());
}

// User agent
WebCore::String WebSettings::userAgentString() const
{
    // The default user agent string is empty.  We rely upon the client to set this for us.
    // We check this by asserting if the client has not done so before the first time it is needed.
    ASSERT(!d->m_olympiaSettingsState->userAgentString.isEmpty());
    return d->m_olympiaSettingsState->userAgentString;
}

void WebSettings::setUserAgentString(const char* s)
{
    d->m_olympiaSettingsState->userAgentString = s;
    d->apply(UserAgentString, d->isGlobal());
}

// Text Encoding Default
void WebSettings::setDefaultTextEncodingName(const char* s)
{
    d->m_webCoreSettingsState->defaultTextEncodingName = s;
    d->apply(DefaultTextEncodingName, d->isGlobal());
}

// Zooming
bool WebSettings::isZoomToFitOnLoad() const
{
    return d->m_olympiaSettingsState->isZoomToFitOnLoad;
}

void WebSettings::setZoomToFitOnLoad(bool b)
{
    d->m_olympiaSettingsState->isZoomToFitOnLoad = b;
    d->apply(IsZoomToFitOnLoad, d->isGlobal());
}

// Text Reflow
WebSettings::TextReflowMode WebSettings::textReflowMode() const
{
    return d->m_olympiaSettingsState->textReflowMode;
}

void WebSettings::setTextReflowMode(WebSettings::TextReflowMode textReflowMode)
{
    d->m_olympiaSettingsState->textReflowMode = textReflowMode;
    d->apply(ShouldUpdateTextReflowMode, d->isGlobal());
}

bool WebSettings::isScrollbarsEnabled() const
{
    return d->m_olympiaSettingsState->isScrollbarsEnabled;
}

void WebSettings::setScrollbarsEnabled(bool b)
{
    d->m_olympiaSettingsState->isScrollbarsEnabled = b;
    d->apply(IsScrollbarsEnabled, d->isGlobal());
}

std::vector<String> WebSettings::detectedFormats() const
{
    std::vector<String> formats;
    if (d->isGlobal())
        formats.assign(d->m_webCoreSettingsState->detectedFormats.begin(), d->m_webCoreSettingsState->detectedFormats.end() - 1);
    else
        formats.assign(d->m_settings->detectedFormats().begin(), d->m_settings->detectedFormats().end() - 1);

    return formats;
}

void WebSettings::setDetectedFormats(const std::vector<String>& formats)
{
    d->m_webCoreSettingsState->detectedFormats.clear();
    for (std::vector<String>::const_iterator i = formats.begin(); i != formats.end(); ++i)
        d->m_webCoreSettingsState->detectedFormats.append(*i);
    d->apply(DetectedFormats, d->isGlobal());
}

bool WebSettings::canJavaScriptOpenWindowsAutomatically() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->canJavaScriptOpenWindowsAutomatically : d->m_settings->javaScriptCanOpenWindowsAutomatically();
}

void WebSettings::setJavaScriptOpenWindowsAutomatically(bool b)
{
    d->m_webCoreSettingsState->canJavaScriptOpenWindowsAutomatically = b;
    d->apply(CanJavaScriptOpenWindowsAutomatically, d->isGlobal());
}

bool WebSettings::arePluginsEnabled() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->arePluginsEnabled : d->m_settings->arePluginsEnabled();
}

void WebSettings::setPluginsEnabled(bool b)
{
    d->m_webCoreSettingsState->arePluginsEnabled = b;
    d->apply(ArePluginsEnabled, d->isGlobal());
}

bool WebSettings::isGeolocationEnabled() const
{
    return d->m_olympiaSettingsState->isGeolocationEnabled;
}

void WebSettings::setGeolocationEnabled(bool b)
{
    d->m_olympiaSettingsState->isGeolocationEnabled = b;
    d->apply(IsGeolocationEnabled, d->isGlobal());
}

void WebSettings::addSupportedObjectPluginMIMEType(const char* type)
{
    if (!s_supportedObjectMIMETypes)
        s_supportedObjectMIMETypes = new HashSet<WebCore::String>;
    s_supportedObjectMIMETypes->add(type);
}

bool WebSettings::isSupportedObjectMIMEType(const WebCore::String& mimeType)
{
    if (mimeType.isEmpty())
        return false;
    if (!s_supportedObjectMIMETypes)
        return false;

    return s_supportedObjectMIMETypes->contains(mimeType);
}

String WebSettings::getNormalizedMIMEType(const String& type)
{
    const MIMETypeAssociationMap* e = mimeTypeAssociationMap;
    while (e->mimeType) {
        if (type == e->mimeType)
            return e->registerType;
        ++e;
    }

    return type;
}

int WebSettings::screenWidth()
{
    return d->m_screenWidth;
}

int WebSettings::screenHeight()
{
    return d->m_screenHeight;
}

void WebSettings::setScreenWidth(int width)
{
    d->m_screenWidth = width;
}

void WebSettings::setScreenHeight(int height)
{
    d->m_screenHeight = height;
}

IntSize WebSettings::applicationViewSize()
{
    return d->m_applicationViewSize;
}

void WebSettings::setApplicationViewSize(const IntSize& applicationViewSize)
{
    d->m_applicationViewSize = applicationViewSize;
}


bool WebSettings::doesGetFocusNodeContext() const
{
    return d->m_olympiaSettingsState->doesGetFocusNodeContext;
}

void WebSettings::setGetFocusNodeContext(bool b)
{
    d->m_olympiaSettingsState->doesGetFocusNodeContext = b;
    d->apply(DoesGetFocusNodeContext, d->isGlobal());
}

void WebSettings::setUserStyleSheetString(const char* styleSheet)
{
    const char* header = "data:text/css;charset=utf-8;base64,";

    Vector<char> data;
    data.append(styleSheet, strlen(styleSheet));

    Vector<char> base64;
    base64Encode(data, base64);

    Vector<char> url;
    url.append(header, strlen(header));
    url.append(base64);
    d->m_webCoreSettingsState->userStyleSheet = KURL(KURL(), WebCore::String(url.data(), url.size()));
    d->apply(UserStyleSheet, d->isGlobal());
}

bool WebSettings::areLinksHandledExternally() const
{
    return d->m_olympiaSettingsState->areLinksHandledExternally;
}

void WebSettings::setAreLinksHandledExternally(bool b)
{
    d->m_olympiaSettingsState->areLinksHandledExternally = b;
    d->apply(AreLinksHandledExternally, d->isGlobal());
}

// BrowserField2 settings
bool WebSettings::allowCrossSiteRequests() const
{
    return d->m_olympiaSettingsState->allowCrossSiteRequests;
}

void WebSettings::setAllowCrossSiteRequests(bool allow)
{
    d->m_olympiaSettingsState->allowCrossSiteRequests = allow;
    d->apply(AllowCrossSiteRequests, d->isGlobal());
}

bool WebSettings::isUserScalable() const
{
    return d->m_olympiaSettingsState->userScalable;
}

void WebSettings::setUserScalable(bool userScalable)
{
    d->m_olympiaSettingsState->userScalable = userScalable;
    d->apply(UserScalable, d->isGlobal());
}

int WebSettings::viewportWidth() const
{
    return d->m_olympiaSettingsState->viewportWidth;
}

void WebSettings::setViewportWidth(int vp )
{
    d->m_olympiaSettingsState->viewportWidth = vp;
    d->apply(ViewportWidth, d->isGlobal());
}

double WebSettings::initialScale() const
{
    return d->m_olympiaSettingsState->initialScale;
}

void WebSettings::setInitialScale(double iniScale)
{
    d->m_olympiaSettingsState->initialScale = iniScale;
    d->apply(InitialScale, d->isGlobal());
}

int WebSettings::firstScheduledLayoutDelay() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->firstScheduledLayoutDelay : d->m_settings->firstScheduledLayoutDelay();
}

void WebSettings::setFirstScheduledLayoutDelay(int i)
{
    d->m_webCoreSettingsState->firstScheduledLayoutDelay = i;
    d->apply(FirstScheduledLayoutDelay, d->isGlobal());
}

bool WebSettings::shouldHandlePatternUrls() const
{
    return d->m_olympiaSettingsState->shouldHandlePatternUrls;
}

void WebSettings::setShouldHandlePatternUrls(bool b)
{
    d->m_olympiaSettingsState->shouldHandlePatternUrls = b;
    d->apply(ShouldHandlePatternUrls, d->isGlobal());
}


bool WebSettings::isCookieCacheEnabled() const
{
    return d->m_olympiaSettingsState->isCookieCacheEnabled;
}

void WebSettings::setIsCookieCacheEnabled(bool b)
{
    d->m_olympiaSettingsState->isCookieCacheEnabled = b;
    d->apply(IsCookieCacheEnabled, d->isGlobal());
}

bool WebSettings::isDatabasesEnabled() const
{
    return d->m_webCoreSettingsState->isDatabasesEnabled;
}

void WebSettings::setIsDatabasesEnabled(bool enable)
{
    d->m_webCoreSettingsState->isDatabasesEnabled = enable;
    d->apply(IsDatabasesEnabled, d->isGlobal());
}

bool WebSettings::isLocalStorageEnabled() const
{
    return d->m_webCoreSettingsState->isLocalStorageEnabled;
}

void WebSettings::setIsLocalStorageEnabled(bool enable)
{
    d->m_webCoreSettingsState->isLocalStorageEnabled = enable;
    d->apply(IsLocalStorageEnabled, d->isGlobal());
}

bool WebSettings::isAppCacheEnabled() const
{
    return d->m_webCoreSettingsState->isOfflineWebApplicationCacheEnabled;
}

void WebSettings::setIsAppCacheEnabled(bool enable)
{
    d->m_webCoreSettingsState->isOfflineWebApplicationCacheEnabled = enable;
    d->apply(IsOfflineWebApplicationCacheEnabled, d->isGlobal());
}

unsigned long long WebSettings::localStorageQuota() const
{
    return d->m_webCoreSettingsState->localStorageQuota;
}

void WebSettings::setLocalStorageQuota(unsigned long long quota)
{
    d->m_webCoreSettingsState->localStorageQuota = quota;
    d->apply(LocalStorageQuota, d->isGlobal());
}

String WebSettings::localStoragePath() const
{
    return d->m_webCoreSettingsState->localStoragePath;
}

void WebSettings::setLocalStoragePath(const String& path)
{
    d->m_webCoreSettingsState->localStoragePath = path;
    d->apply(LocalStoragePath, d->isGlobal());
}

String WebSettings::databasePath() const
{
    return d->m_webCoreSettingsState->databasePath;
}

void WebSettings::setDatabasePath(const String& path)
{
    // DatabaseTracker can only be initialized for once, so it doesn't
    // make sense to change database path after DatabaseTracker has
    // already been initialized.
    static HashSet<WebCore::String> initGroups;
    if (path.isEmpty() || initGroups.contains(d->m_webCoreSettingsState->pageGroupName))
        return;

    initGroups.add(d->m_webCoreSettingsState->pageGroupName);
    d->m_webCoreSettingsState->databasePath = path;
    WebCore::databaseTrackerManager().initializeTracker(d->m_webCoreSettingsState->pageGroupName, d->m_webCoreSettingsState->databasePath);
}

String WebSettings::appCachePath() const
{
    return d->m_webCoreSettingsState->appCachePath;
}

void WebSettings::setAppCachePath(const String& path)
{
    // The directory of cacheStorage for one page group can only be initialized once.
    static HashSet<WebCore::String> initGroups;
    if (path.isEmpty() || initGroups.contains(d->m_webCoreSettingsState->pageGroupName))
        return;

    initGroups.add(d->m_webCoreSettingsState->pageGroupName);
    d->m_webCoreSettingsState->appCachePath = path;
    WebCore::cacheStorage(d->m_webCoreSettingsState->pageGroupName).setCacheDirectory(d->m_webCoreSettingsState->appCachePath);
}

String WebSettings::pageGroupName() const
{
    return d->m_webCoreSettingsState->pageGroupName;
}

bool WebSettings::isEmailMode() const
{
    return d->m_olympiaSettingsState->isEmailMode;
}

void WebSettings::setEmailMode(bool enable)
{
    d->m_olympiaSettingsState->isEmailMode = enable;
    d->apply(IsEmailMode, d->isGlobal());
}

bool WebSettings::shouldRenderAnimationsOnScroll() const
{
    return d->m_olympiaSettingsState->shouldRenderAnimationsOnScroll;
}

void WebSettings::setShouldRenderAnimationsOnScroll(bool enable)
{
    d->m_olympiaSettingsState->shouldRenderAnimationsOnScroll = enable;
    d->apply(ShouldRenderAnimationsOnScroll, d->isGlobal());
}

int WebSettings::overZoomColor() const
{
    return d->m_olympiaSettingsState->overZoomColor;
}

void WebSettings::setOverZoomColor(int color)
{
    d->m_olympiaSettingsState->overZoomColor = color;
    d->apply(OverZoomColor, d->isGlobal());
}

bool WebSettings::useWebKitCache() const
{
    return d->m_webCoreSettingsState->useWebKitCache;
}

void WebSettings::setUseWebKitCache(bool use)
{
    d->m_webCoreSettingsState->useWebKitCache = use;
    d->apply(UseWebKitCache, d->isGlobal());
}

}
}
