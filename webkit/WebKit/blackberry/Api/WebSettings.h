/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef WebSettings_h
#define WebSettings_h

#include "BlackBerryGlobal.h"
#include "WebString.h"

namespace WTF {
    class String;
}
namespace WebCore {
    class FrameLoaderClientBlackBerry;
    class IntSize;
    class Page;
}

namespace Olympia {
namespace WebKit {

class WebString;
class WebSettingsPrivate;

class OLYMPIA_EXPORT WebSettings {
public:

    enum TextReflowMode { TextReflowDisabled, TextReflowEnabled, TextReflowEnabledOnlyForBlockZoom };

    static WebSettings* pageGroupSettings(const WebString& pageGroupName);

    // FIXME: Consider which settings below should be made static so as to enforce
    // that they apply to all pages or do we wish to maintain maximum flexibility?


    // FIXME: Need to find a way to provide getters for the settings that return
    // strings using some kind of thread safe copy...

    // XSS Auditor
    bool xssAuditorEnabled() const;
    void setXSSAuditorEnabled(bool);

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

    WebString localStoragePath() const;
    void setLocalStoragePath(const WebString& path);

    WebString databasePath() const;
    void setDatabasePath(const WebString& path);

    WebString appCachePath() const;
    void setAppCachePath(const WebString& path);

    WebString pageGroupName() const;

    // Object MIMEType
    static void addSupportedObjectPluginMIMEType(const char*);
    static bool isSupportedObjectMIMEType(const WTF::String& mimeType);
    static WebString getNormalizedMIMEType(const WebString&);

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

    bool isWritingDirectionRTL() const;
    void setWritingDirectionRTL(bool);

    bool useWebKitCache() const;
    void setUseWebKitCache(bool use);

    // Frame flattening
    bool isFrameFlatteningEnabled() const;
    void setFrameFlatteningEnabled(bool enable);

    // Direct rendering to canvas
    bool isDirectRenderingToCanvasEnabled() const;
    void setDirectRenderingToCanvasEnabled(bool enable);

    // Plugin Maximum number
    unsigned maxPluginInstances() const;
    void setMaxPluginInstances(unsigned num);

private:
    // These are set via WebPage
    void setScreenWidth(int width);
    void setScreenHeight(int height);
    void setApplicationViewSize(const WebCore::IntSize&);

    // These are for internal usage inside of WebKit so as not to cause a copy
    WTF::String serifFontFamily() const;
    WTF::String fixedFontFamily() const;
    WTF::String sansSerifFontFamily() const;
    WTF::String standardFontFamily() const;
    WTF::String userAgentString() const;

    friend class WebPage;
    friend class WebPagePrivate;
    friend class WebSettingsPrivate;
    friend class WebCore::FrameLoaderClientBlackBerry;
    WebSettings(const WebString&);
    WebSettings(WebCore::Page*, const WebString&);
    ~WebSettings();

    WebSettingsPrivate* d;
};

}
}

#endif // WebSettings_h
