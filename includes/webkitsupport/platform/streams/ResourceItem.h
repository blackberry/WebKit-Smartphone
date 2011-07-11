#ifndef Olympia_Platform_ResourceItem_h
#define Olympia_Platform_ResourceItem_h

#include <string>
#include <vector>

namespace BlackBerry
{
    namespace Platform
    {
        class ResourceItem
        {
        public:

            class HttpHeader {
            public:
                HttpHeader(const char* key, const char* value);
                ~HttpHeader();

            public:
                char* m_key;
                char* m_value;
            };

            enum {
                STATE_INIT = 0,
                STATE_RECEIVING,
                STATE_RECEIVED
            };

            enum {
                SEGMENT_DEFAULT_LENGTH = 8 * 1024
            };

            typedef std::vector<HttpHeader*> HttpHeaderVector;
            typedef std::vector<char*> SegmentVector;

            ResourceItem();
            ~ResourceItem();

            void reset();

            int state() { return m_state; }
            void setState(int state) { m_state = state; }

            HttpHeaderVector& headers();
            void resetHeaders();
            const HttpHeader* header(int index);
            void appendHeader(const char* key, const char* value);
            void removeHeader(const char* key);
            void updateHeader(const char* key, const char* value);

            bool hasData();

            const std::string& content();
            void setContent(const char* data, unsigned dataLength);

            SegmentVector& segments();
            char* segment(int index);
            void appendSegment(const char* data, unsigned len);

            const std::string& url();
            void setURL(const char* url);

            const std::string& charset();
            const std::string& type();

            int httpStatus() { return m_httpStatus; }
            void setHttpStatus(int status) { m_httpStatus = status; }
            const char* httpMessage() { return m_httpMessage; }
            void setHttpMessage(const char* message);

        private:
            bool hasDataInContent();
            void resetContent();
            void ensureDataInContent();
            void generateContentFromSegments();

            bool hasDataInSegments();
            void resetSegments();
            void ensureDataInSegments();
            void generateSegmentsFromContent();

        private:
            char* m_httpMessage;
            HttpHeaderVector m_headers;
            SegmentVector m_segments;

            std::string m_url;
            std::string m_content;
            std::string m_charset;
            std::string m_type;

            int m_httpStatus;
            int m_state;
            unsigned m_segmentLength;
        };//class

    } // namespace Platform

} // namespace Olympia

#endif // Olympia_Platform_ResourceItem_h
