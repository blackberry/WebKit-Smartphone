/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 */

#ifndef ClipboardOlympia_h
#define ClipboardOlympia_h

#include "Clipboard.h"
#include "CachedResourceClient.h"

namespace WebCore {

    class ClipboardBlackBerry : public Clipboard, public CachedResourceClient {
        public:
            static PassRefPtr<ClipboardBlackBerry> create(ClipboardAccessPolicy policy)
            {
                return adoptRef(new ClipboardBlackBerry(policy));
            }
            virtual ~ClipboardBlackBerry();

            void clearData(const String& type);
            void clearAllData();
            String getData(const String& type, bool& success) const;
            bool setData(const String& type, const String& data);

            // extensions beyond IE's API
            virtual HashSet<String> types() const;
            virtual PassRefPtr<FileList> files() const;
            virtual DragImageRef createDragImage(IntPoint& dragLoc) const;
            virtual void declareAndWriteDragImage(Element*, const KURL&, const String& title, Frame*);
            virtual void writeURL(const KURL&, const String&, Frame*);
            virtual void writeRange(Range*, Frame*);
            virtual void writePlainText(const String&);

            virtual bool hasData();

            virtual void setDragImage(CachedImage*, const IntPoint&);
            virtual void setDragImageElement(Node*, const IntPoint&);

        private:
            ClipboardBlackBerry(ClipboardAccessPolicy);
    };
}

#endif
