/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "WorkQueueItem.h"

#include "CString.h"
#include "DumpRenderTreeBlackBerry.h"
#include "Frame.h"
#include "KURL.h"
#include "OwnArrayPtr.h"
#include "Page.h"
#include "PlatformString.h"
#include "WebPage.h"

#include "NotImplemented.h"

bool LoadItem::invoke() const
{
    size_t targetArrSize = JSStringGetMaximumUTF8CStringSize(m_target.get());
    size_t urlArrSize = JSStringGetMaximumUTF8CStringSize(m_url.get());
    OwnArrayPtr<char> target(new char[targetArrSize]);
    OwnArrayPtr<char> url(new char[urlArrSize]);
    size_t targetLen = JSStringGetUTF8CString(m_target.get(), target.get(), targetArrSize) - 1;
    JSStringGetUTF8CString(m_url.get(), url.get(), urlArrSize);

    WebCore::Frame* frame;
    if (target && targetLen)
        frame = mainFrame->tree()->find(target.get());
    else
        frame = mainFrame;

    if (!frame)
        return false;

    WebCore::KURL kurl = WebCore::KURL(WebCore::KURL(), url.get());
    frame->loader()->load(kurl, false);
    return true;
}

bool LoadHTMLStringItem::invoke() const
{
    //FIXME: NOT IMPLEMENTED
    return false;
}

bool ReloadItem::invoke() const
{
    mainFrame->loader()->reload(true);
    return true;
}

bool ScriptItem::invoke() const
{
    notImplemented();
    return false;
}

bool BackForwardItem::invoke() const
{
    return Olympia::WebKit::DumpRenderTree::currentInstance()->page()->goBackOrForward(m_howFar);
}
