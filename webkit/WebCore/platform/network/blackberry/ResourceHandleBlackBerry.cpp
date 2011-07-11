/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#include "config.h"
#include "ResourceHandle.h"
#include "ResourceHandleClient.h"
#include "ResourceHandleInternal.h"

#include "EventLoop.h"
#include "Frame.h"
#include "FrameLoaderClientBlackBerry.h"
#include "NetworkManager.h"
#include "Page.h"
#include "PageGroupLoadDeferrer.h"
#include "ResourceError.h"
#include "ResourceRequest.h"
#include "ResourceResponse.h"
#include "SharedBuffer.h"
#include "ThreadableLoader.h"   // for StoredCredentials
#include "streams/IStream.h"

#include "NotImplemented.h"

using Olympia::WebKit::NetworkManager;

namespace WebCore {

class WebCoreSynchronousLoader : public ResourceHandleClient {
public:
    WebCoreSynchronousLoader();

    virtual void didReceiveResponse(ResourceHandle*, const ResourceResponse&);
    virtual void didReceiveData(ResourceHandle*, const char*, int, int lengthReceived);
    virtual void didFinishLoading(ResourceHandle*);
    virtual void didFail(ResourceHandle*, const ResourceError&);

    ResourceResponse resourceResponse() const { return m_response; }
    ResourceError resourceError() const { return m_error; }
    Vector<char> data() const { return m_data; }
    bool isDone() const { return m_isDone; }

private:
    ResourceResponse m_response;
    ResourceError m_error;
    Vector<char> m_data;
    bool m_isDone;
};

WebCoreSynchronousLoader::WebCoreSynchronousLoader()
    : m_isDone(false)
{
}

void WebCoreSynchronousLoader::didReceiveResponse(ResourceHandle*, const ResourceResponse& response)
{
    m_response = response;
}

void WebCoreSynchronousLoader::didReceiveData(ResourceHandle*, const char* data, int length, int)
{
    m_data.append(data, length);
}

void WebCoreSynchronousLoader::didFinishLoading(ResourceHandle*)
{
    m_isDone = true;
}

void WebCoreSynchronousLoader::didFail(ResourceHandle*, const ResourceError& error)
{
    m_error = error;
    m_isDone = true;
}

ResourceHandleInternal::~ResourceHandleInternal()
{
    notImplemented();
}

ResourceHandle::~ResourceHandle()
{
    notImplemented();
}

bool ResourceHandle::loadsBlocked()
{
    notImplemented();
    return false;
}

void ResourceHandle::platformSetDefersLoading(bool defersLoading)
{
    NetworkManager::instance()->setDefersLoading(this, defersLoading);
}

bool ResourceHandle::start(NetworkingContext* context)
{
    if (!context || !context->isValid())
        return false;

    // FIXME: clean up use of Frame now that we have NetworkingContext (see RIM Bug #1515)
    Frame* frame = context->wrappedFrame();
    if (!frame || !frame->loader() || !frame->loader()->client() || !client())
        return false;
    int playerId = static_cast<FrameLoaderClientBlackBerry*>(frame->loader()->client())->playerId();
    return NetworkManager::instance()->startJob(playerId, this, *frame, d->m_defersLoading);
}

bool ResourceHandle::supportsBufferedData()
{
    notImplemented();
    return false;
}

bool ResourceHandle::willLoadFromCache(ResourceRequest&, Frame*)
{
    notImplemented();
    return false;
}

void ResourceHandle::cancel()
{
    NetworkManager::instance()->stopJob(this);
}

void ResourceHandle::loadResourceSynchronously(NetworkingContext* context, const ResourceRequest& request, StoredCredentials, ResourceError& error, ResourceResponse& response, Vector<char>& data)
{
    if (!context || !context->isValid()) {
        ASSERT(false && "loadResourceSynchronously called with invalid networking context");
        return;
    }

    // FIXME: clean up use of Frame now that we have NetworkingContext (see RIM Bug #1515)
    Frame* frame = context->wrappedFrame();
    if (!frame || !frame->loader() || !frame->loader()->client() || !frame->page()) {
        ASSERT(false && "loadResourceSynchronously called without a frame or frame client");
        return;
    }

    PageGroupLoadDeferrer deferrer(frame->page(), true);
    TimerBase::fireTimersInNestedEventLoop();

    int playerId = static_cast<FrameLoaderClientBlackBerry*>(frame->loader()->client())->playerId();

    WebCoreSynchronousLoader syncLoader;

    bool defersLoading = false;
    bool shouldContentSniff = false;

    RefPtr<ResourceHandle> handle = adoptRef(new ResourceHandle(request, &syncLoader, defersLoading, shouldContentSniff));
    NetworkManager::instance()->startJob(playerId, handle, *frame, defersLoading);

    static const double s_syncLoadTimeOut = 60.0; // seconds

    double startTime = currentTime();
    EventLoop loop;
    while (!syncLoader.isDone() && !loop.ended()) {
        loop.cycle();
        if (currentTime() - startTime > s_syncLoadTimeOut) {
            handle->cancel();
            error = ResourceError(ResourceError::platformErrorDomain, Olympia::Platform::IStream::StatusNetworkError, request.url().string(), "Time out");
            return;
        }
    }

    error = syncLoader.resourceError();
    data = syncLoader.data();
    response = syncLoader.resourceResponse();
}

WTF::PassRefPtr<SharedBuffer> ResourceHandle::bufferedData()
{
    notImplemented();
    return 0;
}

} // namespace WebCore
