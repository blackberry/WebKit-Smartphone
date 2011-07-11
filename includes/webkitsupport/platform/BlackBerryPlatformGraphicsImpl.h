/*
 * Copyright (C) Research In Motion, Limited 2009-2010. All rights reserved.
 */

// Private header, buffer structures might change. Don't include this in WebKit.
#ifndef BlackBerryPlatformGraphicsImpl_h
#define BlackBerryPlatformGraphicsImpl_h

#include "BlackBerryPlatformGraphics.h"
#include "BlackBerryPlatformPrimitives.h"

// Whether to use neon rasterops
#ifndef USE_NEON_RASTER_OPERATIONS
#define USE_NEON_RASTER_OPERATIONS 0
#endif

namespace BlackBerry {
namespace Platform {
namespace Graphics {

bool prefer32BitBuffers();

struct Buffer
{
public:
    enum Format {
        ARGB8888,
        RGB565,
        Alpha4,
        RGB565WithAlpha4
    };

    static Format preferredFormat();
    static unsigned int strideForWidthAndFormat(unsigned int width, Format);

    virtual ~Buffer();

    unsigned char* m_data;
    BlackBerry::Platform::IntSize m_size;
    unsigned int m_stride;
    Format m_format;

protected:
    Buffer();
    void initializeMetadata(const BlackBerry::Platform::IntSize&, Format, unsigned int stride);
};

struct MallocBuffer : public Buffer
{
    MallocBuffer(const BlackBerry::Platform::IntSize&, Format);
    ~MallocBuffer();
};

struct SharedBuffer : public Buffer
{
    SharedBuffer(const BlackBerry::Platform::IntSize&, Format);
    ~SharedBuffer();
};

/* An RGB565 buffer with Alpha4 buffer on the side */
struct SharedBufferWithAlpha : public SharedBuffer
{
    SharedBufferWithAlpha(const BlackBerry::Platform::IntSize&);

    SharedBuffer alpha;
};

/* A buffer that neither creates nor deletes the buffer data. */
struct WrapperBuffer : public Buffer
{
    WrapperBuffer(unsigned char* data, const BlackBerry::Platform::IntSize&, Format, unsigned int stride);

    unsigned int m_bytes;
};

} // namespace Graphics
} // namespace Platform
} // namespace Olympia

#endif // BlackBerryPlatformGraphicsImpl_h
