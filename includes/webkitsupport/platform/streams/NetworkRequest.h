/*
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 */

#ifndef Olympia_Platform_NetworkRequest_h
#define Olympia_Platform_NetworkRequest_h

#include <limits.h> // for INT_MAX
#include <string>
#include <vector>

namespace BlackBerry {

    namespace Platform {

        class NetworkRequestPrivate;

        class NetworkRequest {
        public:

            NetworkRequest();
            ~NetworkRequest();

            enum CachePolicy {
                UseProtocolCachePolicy, // normal load
                ReloadIgnoringCacheData, // reload
                ReturnCacheDataElseLoad, // back/forward or encoding change - allow stale data
                ReturnCacheDataDontLoad, // results of a post - allow stale data and only use cache
            };

            enum TargetType {
                TargetIsUnknown, // Also used in apps that don't care about the type
                TargetIsMainFrame,
                TargetIsSubframe,
                TargetIsSubresource, // Resource is a generic subresource.  (Generally a specific type should be specified)
                TargetIsStyleSheet,
                TargetIsScript,
                TargetIsFontResource,
                TargetIsImage,
                TargetIsObject,
                TargetIsMedia,
                TargetIsXMLHTTPRequest,
                TargetIsWorker,
                TargetIsSharedWorker,
            };

            // matches unspecifiedTimeoutInterval in WebKit's ResourceRequest
            static const int MAX_TIMEOUT = INT_MAX;

            struct FormDataChunk {
                bool isData;
                std::vector<char> buf;
                std::wstring filename;
            };
            typedef std::vector<FormDataChunk> FormDataList;

            typedef std::vector< std::pair<std::string, std::string> > HeaderList;

            void setRequestUrl(const char* url, const char* method, CachePolicy cachePolicy = UseProtocolCachePolicy, TargetType targetType = TargetIsUnknown, double timeout = MAX_TIMEOUT);
            void setRequestInitial(double timeout = MAX_TIMEOUT);
            void setData(const char* buf, size_t len);
            void addMultipartData(const char* buf, size_t len);
            void addMultipartFilename(const unsigned short* filename, size_t len);
            void addHeader(const char* key, const char* value);

            // to avoid excess copying, these methods return an in-place reference to the data
            // it is the callers responsibility to not delete the NetworkRequest while it is still using these references
            bool isInitial() const;
            std::string& getUrlRef() const;
            std::string& getMethodRef() const;
            CachePolicy getCachePolicy() const;
            TargetType getTargetType() const;
            double getTimeout() const;
            FormDataList& getFormDataListRef() const;
            HeaderList& getHeaderListRef() const;

        private:
            NetworkRequestPrivate* d; 
        };

    } // namespace Olympia

} // namespace Platform

#endif // Olympia_Platform_NetworkRequest_h
