/*
 * Copyright (C) 2009 Torch Mobile Inc. http://www.torchmobile.com/
 */

#include "config.h"
#include "FileSystem.h"

#include "CString.h"
#include "NotImplemented.h"
#include "PlatformString.h"
#include "Vector.h"

#include <algorithm>

namespace WebCore {

bool deleteEmptyDirectory(String const& path)
{
    return Olympia::Platform::deleteEmptyDirectory(path.latin1().data());
}

bool deleteFile(String const& path)
{
    return Olympia::Platform::deleteFile(path.latin1().data());
}

bool fileExists(String const& path)
{
    return Olympia::Platform::fileExists(path.latin1().data());
}

bool getFileModificationTime(const String & path, time_t& result)
{
    return Olympia::Platform::getFileModificationTime(path.latin1().data(), result);
}

bool getFileSize(String const& path, long long& result)
{
    return Olympia::Platform::getFileSize(path.latin1().data(), result);
}

String homeDirectoryPath()
{
    notImplemented();
    return String();
}

String pathGetFileName(String const& path)
{
    return path.substring(path.reverseFind('/') + 1);
}

String directoryName(String const& path)
{
    String dirName = path;
    dirName.truncate(dirName.length() - pathGetFileName(path).length());
    return dirName;
}

Vector<String> listDirectory(const String& path, const String& filter)
{
    Vector<String> entries;

    std::vector<std::string> lists;
    if (!Olympia::Platform::listDirectory(path.latin1().data(), filter.latin1().data(), lists))
        return entries;

    for (int i = 0; i < lists.size(); ++i)
        entries.append(lists.at(i).c_str());

    return entries;
}

bool unloadModule(void*)
{
    notImplemented();
    return false;
}

int writeToFile(PlatformFileHandle handle, char const* data, int length)
{
    if (isHandleValid(handle))
        return Olympia::Platform::FileWrite((Olympia::Platform::FileHandle)handle, data, length);

    return 0;
}

void closeFile(PlatformFileHandle& handle)
{
    if (isHandleValid(handle))
        Olympia::Platform::FileClose((Olympia::Platform::FileHandle)handle);
}

CString openTemporaryFile(char const* prefix, PlatformFileHandle& handle)
{
    std::string tempfile = Olympia::Platform::openTemporaryFile(prefix, (Olympia::Platform::FileHandle&)handle);
    return tempfile.c_str();
}

String pathByAppendingComponent(const String& path, const String& component)
{
    if (component.isEmpty())
        return path;

    Vector<UChar, Olympia::Platform::FileNameMaxLength> buffer;

    buffer.append(path.characters(), path.length());

    if (buffer.last() != L'/' && component[0] != L'/')
        buffer.append(L'/');

    buffer.append(component.characters(), component.length());

    return String(buffer.data(), buffer.size());
}

bool makeAllDirectories(const String& path)
{
    int lastDivPos = path.reverseFind('/');
    int endPos = path.length();
    if (lastDivPos == path.length() - 1) {
        endPos -= 1;
        lastDivPos = path.reverseFind('/', lastDivPos);
    }

    if ((lastDivPos > 0) && !makeAllDirectories(path.substring(0, lastDivPos)))
        return false;

    String folder(path.substring(0, endPos));
    return Olympia::Platform::createDirectory(folder.latin1().data());
}

} // namespace WebCore
