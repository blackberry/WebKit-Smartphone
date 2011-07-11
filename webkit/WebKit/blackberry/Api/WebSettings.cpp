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
#include "GroupSettings.h"
#include "HashMap.h"
#include "DatabaseSync.h"
#include "DatabaseTrackerManager.h"
#include "OlympiaPlatformSettings.h"
#include "Page.h"
#include "PageGroup.h"
#include "PlatformString.h"
#include "Settings.h"

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
    IsFrameFlatteningEnabled,
    IsWritingDirectionRTL,
    IsDirectRenderingToCanvasEnabled,
    IsXSSAuditorEnabled,
    MaxPluginInstances
};

typedef WTF::HashMap<WTF::String, WebSettings*> WebSettingsMap;

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
        , xssAuditorEnabled(false)
    {
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
    bool xssAuditorEnabled;

    int defaultFixedFontSize;
    int defaultFontSize;
    int minimumFontSize;
    int firstScheduledLayoutDelay;
    unsigned long long localStorageQuota;

    WTF::String serifFontFamily;
    WTF::String fixedFontFamily;
    WTF::String sansSerifFontFamily;
    WTF::String standardFontFamily;
    WTF::String defaultTextEncodingName;
    WTF::String localStoragePath;
    WTF::String databasePath;
    WTF::String appCachePath;
    WTF::String pageGroupName;

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
        , isWritingDirectionRTL(false)
        , isDirectRenderingToCanvasEnabled(false)
        , maxPluginInstances(1)
    {
    }

    ~OlympiaSettingsState()
    {
    }

    // All settings that are Olympia specific and do not have a corresponding setting
    // in WebCore::Settings can store state here!
    WTF::String userAgentString;
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
    bool isWritingDirectionRTL;
    bool isDirectRenderingToCanvasEnabled;
    unsigned maxPluginInstances;
};

typedef WTF::HashMap<WTF::String, WebString> MIMETypeAssociationMap;

const MIMETypeAssociationMap& mimeTypeAssociationMap()
{
    static MIMETypeAssociationMap* s_map = 0;
    if (!s_map) {
        s_map = new MIMETypeAssociationMap;
        s_map->add("image/x-ms-bmp", "image/bmp");
        s_map->add("image/x-windows-bmp", "image/bmp");
        s_map->add("image/x-bmp", "image/bmp");
        s_map->add("image/x-bitmap", "image/bmp");
        s_map->add("image/x-ms-bitmap", "image/bmp");
        s_map->add("image/jpg", "image/jpeg");
        s_map->add("image/pjpeg", "image/jpeg");
        s_map->add("image/x-png", "image/png");
        s_map->add("image/vnd.rim.png", "image/png");
        s_map->add("image/ico", "image/vnd.microsoft.icon");
        s_map->add("image/icon", "image/vnd.microsoft.icon");
        s_map->add("text/ico", "image/vnd.microsoft.icon");
        s_map->add("application/ico", "image/vnd.microsoft.icon");
        s_map->add("image/x-icon", "image/vnd.microsoft.icon");
        s_map->add("audio/vnd.qcelp", "audio/qcelp");
        s_map->add("audio/qcp", "audio/qcelp");
        s_map->add("audio/vnd.qcp", "audio/qcelp");
        s_map->add("audio/wav", "audio/x-wav");
        s_map->add("audio/mid", "audio/midi");
        s_map->add("audio/sp-midi", "audio/midi");
        s_map->add("audio/x-mid", "audio/midi");
        s_map->add("audio/x-midi", "audio/midi");
        s_map->add("audio/x-mpeg", "audio/mpeg");
        s_map->add("audio/mp3", "audio/mpeg");
        s_map->add("audio/x-mp3", "audio/mpeg");
        s_map->add("audio/mpeg3", "audio/mpeg");
        s_map->add("audio/x-mpeg3", "audio/mpeg");
        s_map->add("audio/mpg3", "audio/mpeg");
        s_map->add("audio/mpg", "audio/mpeg");
        s_map->add("audio/x-mpg", "audio/mpeg");
        s_map->add("audio/m4a", "audio/mp4");
        s_map->add("audio/x-m4a", "audio/mp4");
        s_map->add("audio/x-mp4", "audio/mp4");
        s_map->add("audio/x-aac", "audio/aac");
        s_map->add("audio/x-amr", "audio/amr");
        s_map->add("audio/mpegurl", "audio/x-mpegurl");
        s_map->add("video/3gp", "video/3gpp");
        s_map->add("video/avi", "video/x-msvideo");
        s_map->add("video/x-m4v", "video/mp4");
        s_map->add("video/x-quicktime", "video/quicktime");
        s_map->add("application/java", "application/java-archive");
        s_map->add("application/x-java-archive", "application/java-archive");
        s_map->add("application/x-zip-compressed", "application/zip");
    }
    return *s_map;
}

static HashSet<WTF::String>* s_supportedObjectMIMETypes;

WebCore::Settings* getSettings(WebCore::Page* page) {
    ASSERT(page);
    return page->settings();
}

WebCore::GroupSettings* getGroupSettings(WebCore::Page* page) {
    ASSERT(page);
    return page->group().groupSettings();
}

WebSettingsPrivate::WebSettingsPrivate(const WebString& pageGroupName)
    : m_page(0)
    , m_webCoreSettingsState(new WebCoreSettingsState)
    , m_olympiaSettingsState(new OlympiaSettingsState)
    , m_screenWidth(0)
    , m_screenHeight(0)
{
    m_webCoreSettingsState->pageGroupName = pageGroupName;
}

WebSettingsPrivate::WebSettingsPrivate(WebCore::Page* page, const WebString& pageGroupName)
    : m_page(page)
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

    m_page = 0; // FIXME: We don't own this so perhaps it should be a RefPtr?
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

        WebCore::Settings* settings = getSettings(m_page);

        if (setting == AllSettings || setting == IsXSSAuditorEnabled)
            settings->setXSSAuditorEnabled(webcoreSettingsState->xssAuditorEnabled);

        if (setting == AllSettings || setting == LoadsImagesAutomatically)
            settings->setLoadsImagesAutomatically(webcoreSettingsState->loadsImagesAutomatically);

        if (setting == AllSettings || setting == ShouldDrawBorderWhileLoadingImages)
            settings->setShouldDrawBorderWhileLoadingImages(webcoreSettingsState->shouldDrawBorderWhileLoadingImages);

        if (setting == AllSettings || setting == IsJavaScriptEnabled)
            settings->setJavaScriptEnabled(webcoreSettingsState->isJavaScriptEnabled);

        if (setting == AllSettings || setting == DefaultFixedFontSize)
            settings->setDefaultFixedFontSize(webcoreSettingsState->defaultFixedFontSize);

        if (setting == AllSettings || setting == DefaultFontSize)
            settings->setDefaultFontSize(webcoreSettingsState->defaultFontSize);

        if (setting == AllSettings || setting == MinimumFontSize)
            settings->setMinimumFontSize(webcoreSettingsState->minimumFontSize);

        if (setting == AllSettings || setting == SerifFontFamily)
            settings->setSerifFontFamily(webcoreSettingsState->serifFontFamily);

        if (setting == AllSettings || setting == FixedFontFamily)
            settings->setFixedFontFamily(webcoreSettingsState->fixedFontFamily);

        if (setting == AllSettings || setting == SansSerifFontFamily)
            settings->setSansSerifFontFamily(webcoreSettingsState->sansSerifFontFamily);

        if (setting == AllSettings || setting == StandardFontFamily)
            settings->setStandardFontFamily(webcoreSettingsState->standardFontFamily);

        if (setting == AllSettings || setting == CanJavaScriptOpenWindowsAutomatically)
            settings->setJavaScriptCanOpenWindowsAutomatically(webcoreSettingsState->canJavaScriptOpenWindowsAutomatically);

        if (setting == AllSettings || setting == ArePluginsEnabled)
            settings->setPluginsEnabled(webcoreSettingsState->arePluginsEnabled);

        if (setting == AllSettings || setting == DefaultTextEncodingName)
            settings->setDefaultTextEncodingName(webcoreSettingsState->defaultTextEncodingName);

        if (setting == AllSettings || setting == UserStyleSheet)
            settings->setUserStyleSheetLocation(webcoreSettingsState->userStyleSheet);

        if (setting == AllSettings || setting == FirstScheduledLayoutDelay)
            settings->setFirstScheduledLayoutDelay(webcoreSettingsState->firstScheduledLayoutDelay);

        if (setting == AllSettings || setting == UseWebKitCache)
            settings->setUseCache(webcoreSettingsState->useWebKitCache);
#if ENABLE(DATABASE)
        if (setting == AllSettings || setting == LocalStoragePath)
            settings->setLocalStorageDatabasePath(webcoreSettingsState->localStoragePath);

        if (setting == AllSettings || setting == IsDatabasesEnabled) {
            WebCore::Database::setIsAvailable(webcoreSettingsState->isDatabasesEnabled);
            WebCore::DatabaseSync::setIsAvailable(webcoreSettingsState->isDatabasesEnabled);
        }

        if (setting == AllSettings || setting == IsLocalStorageEnabled)
            settings->setLocalStorageEnabled(webcoreSettingsState->isLocalStorageEnabled);

        if (setting == AllSettings || setting == IsOfflineWebApplicationCacheEnabled)
            settings->setOfflineWebApplicationCacheEnabled(webcoreSettingsState->isOfflineWebApplicationCacheEnabled);

        if (setting == AllSettings || setting == LocalStorageQuota)
            getGroupSettings(m_page)->setLocalStorageQuotaBytes(webcoreSettingsState->localStorageQuota);
        
        if (setting == AllSettings || setting == IsFrameFlatteningEnabled)
            settings->setFrameFlatteningEnabled(webcoreSettingsState->isFrameFlatteningEnabled);
#endif

        // These are *NOT* exposed via the API so just always set them if this
        // is global and we're setting all the settings...
        if (setting == AllSettings && global) {
            settings->setJavaEnabled(webcoreSettingsState->isJavaEnabled);
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
            settings->setTextReflowEnabled(m_olympiaSettingsState->textReflowMode == WebSettings::TextReflowEnabled);
        }

        if (setting == AllSettings || setting == IsEmailMode) {
            m_olympiaSettingsState->isEmailMode = olympiaSettingsState->isEmailMode;
            // FIXME: See RIM Bug #746.
            // We don't want HTMLTokenizer to yield for anything other than email
            // case because call to currentTime() is too expensive at currentTime() :(
            settings->setShouldUseFirstScheduledLayoutDelay(m_olympiaSettingsState->isEmailMode);
            settings->setProcessHTTPEquiv(!m_olympiaSettingsState->isEmailMode);
        }

        if (setting == AllSettings || setting == ShouldRenderAnimationsOnScroll)
            m_olympiaSettingsState->shouldRenderAnimationsOnScroll = olympiaSettingsState->shouldRenderAnimationsOnScroll;

        if (setting == AllSettings || setting == OverZoomColor)
            m_olympiaSettingsState->overZoomColor = olympiaSettingsState->overZoomColor;

        if (setting == AllSettings || setting == IsWritingDirectionRTL)
            m_olympiaSettingsState->isWritingDirectionRTL = olympiaSettingsState->isWritingDirectionRTL;

        if (setting == AllSettings || setting == IsDirectRenderingToCanvasEnabled)
            m_olympiaSettingsState->isDirectRenderingToCanvasEnabled = olympiaSettingsState->isDirectRenderingToCanvasEnabled;

        if (setting == AllSettings || setting == MaxPluginInstances)
            m_olympiaSettingsState->maxPluginInstances = olympiaSettingsState->maxPluginInstances;

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

WebSettings* WebSettings::pageGroupSettings(const WebString& pageGroupName)
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

WebSettings::WebSettings(const WebString& pageGroupName)
    : d(new WebSettingsPrivate(pageGroupName))
{
}

WebSettings::WebSettings(WebCore::Page* page, const WebString& pageGroupName)
    : d(new WebSettingsPrivate(page, pageGroupName))
{
}

WebSettings::~WebSettings()
{
    delete d;
    d = 0;
}

bool WebSettings::xssAuditorEnabled() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->xssAuditorEnabled : getSettings(d->m_page)->xssAuditorEnabled();
}

void WebSettings::setXSSAuditorEnabled(bool xssAuditorEnabled)
{
    d->m_webCoreSettingsState->xssAuditorEnabled = xssAuditorEnabled;
    d->apply(IsXSSAuditorEnabled, d->isGlobal());
}

// Images
bool WebSettings::loadsImagesAutomatically() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->loadsImagesAutomatically : getSettings(d->m_page)->loadsImagesAutomatically();
}

void WebSettings::setLoadsImagesAutomatically(bool b)
{
    d->m_webCoreSettingsState->loadsImagesAutomatically = b;
    d->apply(LoadsImagesAutomatically, d->isGlobal());
}

bool WebSettings::shouldDrawBorderWhileLoadingImages() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->shouldDrawBorderWhileLoadingImages : getSettings(d->m_page)->shouldDrawBorderWhileLoadingImages();
}

void WebSettings::setShouldDrawBorderWhileLoadingImages(bool b)
{
    d->m_webCoreSettingsState->shouldDrawBorderWhileLoadingImages = b;
    d->apply(ShouldDrawBorderWhileLoadingImages, d->isGlobal());
}

// JavaScript
bool WebSettings::isJavaScriptEnabled() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->isJavaScriptEnabled : getSettings(d->m_page)->isJavaScriptEnabled();
}

void WebSettings::setJavaScriptEnabled(bool b)
{
    d->m_webCoreSettingsState->isJavaScriptEnabled = b;
    d->apply(IsJavaScriptEnabled, d->isGlobal());
}

// Font sizes
int WebSettings::defaultFixedFontSize() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->defaultFixedFontSize : getSettings(d->m_page)->defaultFixedFontSize();
}

void WebSettings::setDefaultFixedFontSize(int i)
{
    d->m_webCoreSettingsState->defaultFixedFontSize = i;
    d->apply(DefaultFixedFontSize, d->isGlobal());
}

int WebSettings::defaultFontSize() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->defaultFontSize : getSettings(d->m_page)->defaultFontSize();
}

void WebSettings::setDefaultFontSize(int i)
{
    d->m_webCoreSettingsState->defaultFontSize = i;
    d->apply(DefaultFontSize, d->isGlobal());
}

int WebSettings::minimumFontSize() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->minimumFontSize : getSettings(d->m_page)->minimumFontSize();
}

void WebSettings::setMinimumFontSize(int i)
{
    d->m_webCoreSettingsState->minimumFontSize = i;
    d->apply(MinimumFontSize, d->isGlobal());
}

// Font families
WTF::String WebSettings::serifFontFamily() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->serifFontFamily : getSettings(d->m_page)->serifFontFamily().string();
}

void WebSettings::setSerifFontFamily(const char* s)
{
    d->m_webCoreSettingsState->serifFontFamily = s;
    d->apply(SerifFontFamily, d->isGlobal());
}

WTF::String WebSettings::fixedFontFamily() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->fixedFontFamily : getSettings(d->m_page)->fixedFontFamily().string();
}

void WebSettings::setFixedFontFamily(const char* s)
{
    d->m_webCoreSettingsState->fixedFontFamily = s;
    d->apply(FixedFontFamily, d->isGlobal());
}

WTF::String WebSettings::sansSerifFontFamily() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->sansSerifFontFamily : getSettings(d->m_page)->sansSerifFontFamily().string();
}

void WebSettings::setSansSerifFontFamily(const char* s)
{
    d->m_webCoreSettingsState->sansSerifFontFamily = s;
    d->apply(SansSerifFontFamily, d->isGlobal());
}

WTF::String WebSettings::standardFontFamily() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->standardFontFamily : getSettings(d->m_page)->standardFontFamily().string();
}

void WebSettings::setStandardFontFamily(const char* s)
{
    d->m_webCoreSettingsState->standardFontFamily = s;
    d->apply(StandardFontFamily, d->isGlobal());
}

bool WebSettings::isFrameFlatteningEnabled() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->isFrameFlatteningEnabled : getSettings(d->m_page)->frameFlatteningEnabled();
}

void WebSettings::setFrameFlatteningEnabled(bool enable)
{
    d->m_webCoreSettingsState->isFrameFlatteningEnabled = enable;
    d->apply(IsFrameFlatteningEnabled, d->isGlobal());
}

// User agent
WTF::String WebSettings::userAgentString() const
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

bool WebSettings::canJavaScriptOpenWindowsAutomatically() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->canJavaScriptOpenWindowsAutomatically : getSettings(d->m_page)->javaScriptCanOpenWindowsAutomatically();
}

void WebSettings::setJavaScriptOpenWindowsAutomatically(bool b)
{
    d->m_webCoreSettingsState->canJavaScriptOpenWindowsAutomatically = b;
    d->apply(CanJavaScriptOpenWindowsAutomatically, d->isGlobal());
}

bool WebSettings::arePluginsEnabled() const
{
    return d->isGlobal() ? d->m_webCoreSettingsState->arePluginsEnabled : getSettings(d->m_page)->arePluginsEnabled();
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
        s_supportedObjectMIMETypes = new HashSet<WTF::String>;
    s_supportedObjectMIMETypes->add(type);
}

bool WebSettings::isSupportedObjectMIMEType(const WTF::String& mimeType)
{
    if (mimeType.isEmpty())
        return false;
    if (!s_supportedObjectMIMETypes)
        return false;

    return s_supportedObjectMIMETypes->contains(getNormalizedMIMEType(mimeType));
}

WebString WebSettings::getNormalizedMIMEType(const WebString& type)
{
    MIMETypeAssociationMap::const_iterator i = mimeTypeAssociationMap().find(type);
    return i == mimeTypeAssociationMap().end() ? type : i->second;
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
    d->m_webCoreSettingsState->userStyleSheet = KURL(KURL(), WTF::String(url.data(), url.size()));
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
    return d->isGlobal() ? d->m_webCoreSettingsState->firstScheduledLayoutDelay : getSettings(d->m_page)->firstScheduledLayoutDelay();
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

WebString WebSettings::localStoragePath() const
{
    return d->m_webCoreSettingsState->localStoragePath;
}

void WebSettings::setLocalStoragePath(const WebString& path)
{
    d->m_webCoreSettingsState->localStoragePath = path;
    d->apply(LocalStoragePath, d->isGlobal());
}

WebString WebSettings::databasePath() const
{
    return d->m_webCoreSettingsState->databasePath;
}

void WebSettings::setDatabasePath(const WebString& path)
{
    // DatabaseTracker can only be initialized for once, so it doesn't
    // make sense to change database path after DatabaseTracker has
    // already been initialized.
    static HashSet<WTF::String> initGroups;
    if (path.isEmpty() || initGroups.contains(d->m_webCoreSettingsState->pageGroupName))
        return;

    initGroups.add(d->m_webCoreSettingsState->pageGroupName);
    d->m_webCoreSettingsState->databasePath = path;
    WebCore::databaseTrackerManager().initializeTracker(d->m_webCoreSettingsState->pageGroupName, d->m_webCoreSettingsState->databasePath);
}

WebString WebSettings::appCachePath() const
{
    return d->m_webCoreSettingsState->appCachePath;
}

void WebSettings::setAppCachePath(const WebString& path)
{
    // The directory of cacheStorage for one page group can only be initialized once.
    static HashSet<WTF::String> initGroups;
    if (path.isEmpty() || initGroups.contains(d->m_webCoreSettingsState->pageGroupName))
        return;

    initGroups.add(d->m_webCoreSettingsState->pageGroupName);
    d->m_webCoreSettingsState->appCachePath = path;
    WebCore::cacheStorage(d->m_webCoreSettingsState->pageGroupName).setCacheDirectory(d->m_webCoreSettingsState->appCachePath);
}

WebString WebSettings::pageGroupName() const
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

bool WebSettings::isDirectRenderingToCanvasEnabled() const
{
    return d->m_olympiaSettingsState->isDirectRenderingToCanvasEnabled;
}

void WebSettings::setDirectRenderingToCanvasEnabled(bool enabled)
{
    d->m_olympiaSettingsState->isDirectRenderingToCanvasEnabled = enabled;
    d->apply(IsDirectRenderingToCanvasEnabled, d->isGlobal());
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

bool WebSettings::isWritingDirectionRTL() const
{
    return d->m_olympiaSettingsState->isWritingDirectionRTL;
}

void WebSettings::setWritingDirectionRTL(bool isRTL)
{
    d->m_olympiaSettingsState->isWritingDirectionRTL = isRTL;
    d->apply(IsWritingDirectionRTL, d->isGlobal());
}

unsigned WebSettings::maxPluginInstances() const
{
    return d->m_olympiaSettingsState->maxPluginInstances;
}

void WebSettings::setMaxPluginInstances(unsigned num)
{
    d->m_olympiaSettingsState->maxPluginInstances = num;
    d->apply(MaxPluginInstances, d->isGlobal());
}

}
}
