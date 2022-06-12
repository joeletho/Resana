#pragma once

#include <string>
#include <tchar.h>

namespace RESANA {

    class FileUtils {
    public:
        static bool ValidatePath(const std::string &filepath);

        static std::string ReadFile(const std::string &filepath);


        static std::wstring GetFilenameFromPath(const std::wstring &filepath);
        static std::string GetFilenameFromPath(const std::string &filepath);
        static std::wstring GetNameFromPath(const std::wstring &filepath);
        static std::string GetNameFromPath(const std::string &filepath);

        static std::wstring ToWString(const wchar_t wchar[260]);
    };


}