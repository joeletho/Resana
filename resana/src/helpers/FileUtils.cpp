#include "rspch.h"

#include "FileUtils.h"

#include "fstream"

namespace RESANA {

	bool FileUtils::ValidatePath(const std::string& filepath)
	{
		if (filepath.empty())
		{
			RS_CORE_ERROR("Filepath empty!");
			return false;
		}

		std::string result;
		std::ifstream in;
		in.open(filepath);

		if (in.is_open())
		{
			in.close();
			return true;
		}

		RS_CORE_ERROR("'{0}' Could not open file!", filepath);
		return false;
	}

	std::string FileUtils::ReadFile(const std::string& filepath)
	{
		if (!ValidatePath(filepath)) { return {}; }

		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);

		// Get the size of the file, reserve the size in a string, and load everything from the file into the string
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}

		return result;
	}

	std::wstring FileUtils::GetFilenameFromPath(const std::wstring& filepath)
	{
		/* Extract name from filename
		 "assets/shaders/default.glsl" -- UNIX
		 "assets\shaders\default.glsl" -- Windows
		 */
		auto lastSlash = filepath.find_last_of(L"/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto count = filepath.size() - std::string::npos;

		return filepath.substr(lastSlash, count);
	}

	std::string FileUtils::GetFilenameFromPath(const std::string& filepath)
	{
		/* Extract name from filename
		 "assets/shaders/default.glsl" -- UNIX
		 "assets\shaders\default.glsl" -- Windows
		 */
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto count = filepath.size() - std::string::npos;

		return filepath.substr(lastSlash, count);
	}

	std::wstring FileUtils::GetNameFromPath(const std::wstring& filepath)
	{
		/* Extract name from filename
		 "assets/shaders/default.glsl" -- UNIX
		 "assets\shaders\default.glsl" -- Windows
		 */
		auto lastSlash = filepath.find_last_of(L"/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.find_first_of('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;

		return filepath.substr(lastSlash, count);
	}

	std::string FileUtils::GetNameFromPath(const std::string& filepath)
	{
		/* Extract name from filename
		 "assets/shaders/default.glsl" -- UNIX
		 "assets\shaders\default.glsl" -- Windows
		 */
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.find_first_of('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;

		return filepath.substr(lastSlash, count);
	}

	std::wstring FileUtils::ToWString(const wchar_t wchar[260])
	{
		std::wstring wstring(wchar);
		return wstring;
	}

}