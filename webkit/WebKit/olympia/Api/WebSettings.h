/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef WebSettings_h
#define WebSettings_h

#include "OlympiaGlobal.h"
#include "OlympiaString.h"

#include <vector>

namespace WebCore {
    class FrameLoaderClientOlympia;
    class IntSize;
    class Settings;
    class String;
}

namespace Olympia {
namespace WebKit {

class String;
class WebSettingsPrivate;

class OLYMPIA_EXPORT WebSettings {
public:

    enum TextReflowMode { TextReflowDisabled, TextReflowEnabled, TextReflowEnabledOnlyForBlockZoom };

    static WebSettings* pageGroupSettings(const String& pageGroupName);

    // FIXME: Consider which settings below should be made static so as to enforce
    // that they apply to all pages or do we wish to maintain maximum flexibility?


    // FIXME: Need to find a way to provide getters for the settings that return
    // strings using some kind of thread safe copy...

    // Images
    bool loadsImagesAutomatically() const;
    void setLoadsImagesAutomatically(bool);

    bool shouldDrawBorderWhileLoadingImages() const;
    void setShouldDrawBorderWhileLoadingImages(bool);

    // JavaScript
    bool isJavaScriptEnabled() const;
    void setJavaScriptEnabled(bool);

    // Font sizes
    int defaultFixedFontSize() const;
    void setDefaultFixedFontSize(int);

    int defaultFontSize() const;
    void setDefaultFontSize(int);

    int minimumFontSize() const;
    void setMinimumFontSize(int);

    // Font families
    void setSerifFontFamily(const char*);
    void setFixedFontFamily(const char*);
    void setSansSerifFontFamily(const char*);
    void setStandardFontFamily(const char*);

    // User agent
    void setUserAgentString(const char*);

    // Default Text Encoding
    void setDefaultTextEncodingName(const char*);

    // Zooming
    bool isZoomToFitOnLoad() const;
    void setZoomToFitOnLoad(bool);

    // Text Reflow
    TextReflowMode textReflowMode() const;
    void setTextReflowMode(TextReflowMode);

    // Scrollbars
    bool isScrollbarsEnabled() const;
    void setScrollbarsEnabled(bool);

    // Format Detection
    std::vector<String> detectedFormats() const;
    void setDetectedFormats(const std::vector<String>&);

    // Javascript Popups
    // FIXME: Consider renaming this method upstream, where it is called javaScriptCanOpenWindowsAutomatically
    bool canJavaScriptOpenWindowsAutomatically() const;
    void setJavaScriptOpenWindowsAutomatically(bool);

    // Plugins
    bool arePluginsEnabled() const;
    void setPluginsEnabled(bool);

    // Geolocation
    bool isGeolocationEnabled() const;
    void setGeolocationEnabled(bool);

    // Context Info
    bool doesGetFocusNodeContext() const;
    void setGetFocusNodeContext(bool);

    // Style Sheet
    void setUserStyleSheetString(const char*);

    // External link handlers
    bool areLinksHandledExternally() const;
    void setAreLinksHandledExternally(bool);

    // BrowserField2 settings
    void setAllowCrossSiteRequests(bool allow);
    bool allowCrossSiteRequests() const;
    bool isUserScalable() const;
    void setUserScalable(bool userScalable);
    int viewportWidth() const;
    void setViewportWidth(int vp);
    double initialScale() const;
    void setInitialScale(double iniScale);

    // First Layout Delay
    int firstScheduledLayoutDelay() const;
    void setFirstScheduledLayoutDelay(int);

    // Whether to include pattern: in the list of string patterns
    bool shouldHandlePatternUrls() const;
    void setShouldHandlePatternUrls(bool);

    // Cache settings
    bool isCookieCacheEnabled() const;
    void setIsCookieCacheEnabled(bool);

    // Web storage settings
    bool isLocalStorageEnabled() const;
    void setIsLocalStorageEnabled(bool enable);

    bool isDatabasesEnabled() const;
    void setIsDatabasesEnabled(bool enable);

    bool isAppCacheEnabled() const;
    void setIsAppCacheEnabled(bool enable);

    unsigned long long localStorageQuota() const;
    void setLocalStorageQuota(unsigned long long quota);

    String localStoragePath() const;
    void setLocalStoragePath(const String& path);

    String databasePath() const;
    void setDatabasePath(const String& path);

    String appCachePath() const;
    void setAppCachePath(const String& path);

    String pageGroupName() const;

    // Object MIMEType
    static void addSupportedObjectPluginMIMEType(const char*);
    static bool isSupportedObjectMIMEType(const WebCore::String& mimeType);
    static String getNormalizedMIMEType(const String&);

    int screenWidth();
    int screenHeight();

    WebCore::IntSize applicationViewSize();

    bool isEmailMode() const;
    void setEmailMode(bool enable);

    bool shouldRenderAnimationsOnScroll() const;
    void setShouldRenderAnimationsOnScroll(bool enable);

    // OverZoom Background
    int overZoomColor() const;
    void setOverZoomColor(int);

    bool WebSettings::useWebKitCache() const;
    void WebSettings::setUseWebKitCache(bool use);

    // Frame flattening
    bool isFrameFlatteningEnabled() const;
    void setFrameFlatteningEnabled(bool enable);

private:
    // These are set via WebPage
    void setScreenWidth(int width);
    void setScreenHeight(int height);
    void setApplicationViewSize(const WebCore::IntSize&);

    // These are for internal usage inside of WebKit so as not to cause a copy
    WebCore::String serifFontFamily() const;
    WebCore::String fixedFontFamily() const;
    WebCore::String sansSerifFontFamily() const;
    WebCore::String standardFontFamily() const;
    WebCore::String userAgentString() const;

    friend class WebPage;
    friend class WebPagePrivate;
    friend class WebSettingsPrivate;
    friend class WebCore::FrameLoaderClientOlympia;
    WebSettings(const String&);
    WebSettings(WebCore::Settings*, const String&);
    ~WebSettings();

    WebSettingsPrivate* d;
};

}
}

#endif // WebSettings_h
