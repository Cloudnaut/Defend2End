#include "Basics.hpp"
//#include "GlobalDefinitions.hpp"

#ifdef _WIN32
#include <Windows.h>
#elif  __unix__
//IncludePath
#endif

namespace akilib
{

#ifdef _WIN32
	std::string GetExecFilePath()
	{
		HMODULE hModule = GetModuleHandle(NULL);
		char FilePath[MAX_PATH];
		GetModuleFileName(hModule, FilePath, MAX_PATH);
		return FilePath;
	}
#elif __unix__
	std::string GetExecFilePath()
	{
	}
#endif

	std::string GetExecDirectory()
	{
		char Directory[MAX_PATH];
		strcpy_s(Directory, GetExecFilePath().c_str());

		char *pos = strrchr(Directory, '\\');
		if (pos != NULL)
		{
			*pos = '\0'; //this will put the null terminator here. you can also copy to another string if you want
		}

		strcat_s(Directory, "\\");

		return Directory;
	}
}