/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "DragImage.h"

#include "CachedImage.h"
#include "FloatSize.h"
#include "Image.h"

#include "NotImplemented.h"

namespace WebCore {

void* createDragImageFromImage(Image*)
{
    notImplemented();
    return 0;
}

void* createDragImageIconForCachedImage(CachedImage*)
{
    notImplemented();
    return 0;
}

void deleteDragImage(void*)
{
    notImplemented();
}

void* dissolveDragImageToFraction(void*, float)
{
    notImplemented();
    return 0;
}

void* scaleDragImage(void*, FloatSize)
{
    notImplemented();
    return 0;
}

IntSize dragImageSize(void*)
{
    notImplemented();
    return IntSize();
}

} // namespace WebCore
