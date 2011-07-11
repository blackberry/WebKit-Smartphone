/*
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

#ifndef BlackBerryPlatformTextCodec_h
#define BlackBerryPlatformTextCodec_h

namespace BlackBerry {
namespace Platform {
namespace TextCodec {

    enum Result {
        successful,
        sourceBroken,
        destinationPending,
        encodingNotSupported,
    };

    struct Encoding {
        const char* m_name;
        unsigned m_id;
    };

    Result decode(unsigned encoding, const char*& sourceStart, const char* sourceEnd
        , unsigned short*& destinationStart, unsigned short* destinationEnd);

    Result encode(unsigned encoding, const unsigned short*& sourceStart, const unsigned short* sourceEnd, 
                char*& destinationStart, char* destinationEnd);

    unsigned getSupportedEncodings(Encoding* receiveBuffer, unsigned maxReceiveCount);

    unsigned getAliases(unsigned encoding, char const** receiveBuffer, unsigned maxReceiveCount);

    unsigned getEncodingID(const char* encodingName);
    bool getEncodingByAlias(const char* alias, Encoding& encoding);

} // TextCodec
} // Platform
} // Olympia

#endif // BlackBerryPlatformTextCodec_h
