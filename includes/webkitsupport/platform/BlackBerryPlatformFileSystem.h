/*
 * Copyright (C) 2009 Research In Motion Limited. http://www.rim.com/
 */

#ifndef BlackBerryPlatformFileSystem_h
#define BlackBerryPlatformFileSystem_h

#include <string>
#include <vector>

namespace BlackBerry {
    namespace Platform {

        enum {
            FileNameMaxLength = 256,
        };

        typedef int FileHandle;

        bool fileExists(const char* filename);

        bool getFileSize(const char* filename, long long& size);
        bool getFileModificationTime(const char* filename, time_t& time);

        bool deleteFile(const char* filename);
        bool deleteEmptyDirectory(const char* path);

        bool emptyDirectory(const char* path);

        bool createDirectory(const char* path);
        std::string openTemporaryFile(const char* prefix, FileHandle& handle);

        bool fileRead(FileHandle handle, char* data, size_t length);
        bool fileWrite(FileHandle handle, const char* data, size_t length);
        void fileClose(FileHandle handle);
        FileHandle fileOpen(const char* path, int flag);

        bool listDirectory(const char* path, const char* filter, std::vector<std::string>& lists);

        //encrypted file support
        bool prepareEncryptedFile(const char * filename);
        typedef int (*GetEncryptionInfoFunctionPtrType)(const char*, unsigned char**, unsigned int*, unsigned int*);
        typedef void (*ReleaseEncryptionInfoFunctionPtrType)(unsigned char*);
        void assignGetEncryptionInfoFunction(GetEncryptionInfoFunctionPtrType);
        void assignReleaseKeyFunction(ReleaseEncryptionInfoFunctionPtrType);

    } // namespace Olympia
} // namespace Platform

#endif // BlackBerryPlatformFileSystem_h
