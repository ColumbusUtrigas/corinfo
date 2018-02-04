#ifdef _WIN32
	#define COLUMBUS_PLATFORM_WINDOWS
	#include <windows.h>
#endif

#ifdef __linux
	#define COLUMBUS_PLATFORM_LINUX
	#include <sys/sysinfo.h>
	#include <unistd.h>
#endif

#include <cstdio>

namespace Columbus
{

	class Info
	{
	public:
		static unsigned int getCPUCount();
		static unsigned int getCPUSpeed();

		static unsigned long getRAMSize();
		static unsigned long getRAMFree();
		static int getRAMUsage();
	};

	unsigned int Info::getCPUCount()
	{
		#ifdef COLUMBUS_PLATFORM_LINUX
			return sysconf(_SC_NPROCESSORS_ONLN);
		#endif

		#ifdef COLUMBUS_PLATFORM_WINDOWS
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			return sysinfo.dwNumberOfProcessors;
		#endif
	}

	unsigned int Info::getCPUSpeed()
	{
		#ifdef COLUMBUS_PLATFORM_WINDOWS
			wchar_t Buffer[_MAX_PATH];
			DWORD BufSize = _MAX_PATH;
			DWORD dwMHz = _MAX_PATH;
			HKEY hKey;

			long lError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
				L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey);

			if (lError != ERROR_SUCCESS)
			{
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, lError, 0, Buffer, _MAX_PATH, 0);
				wprintf(Buffer);
				return 0;
			}

			RegQueryValueEx(hKey, L"~MHz", NULL, NULL, (LPBYTE)&dwMHz, &BufSize);
			return dwMHz;
		#endif
	}

	unsigned long Info::getRAMSize()
	{
		#ifdef COLUMBUS_PLATFORM_LINUX
			struct sysinfo info;
			if (sysinfo(&info) == -1) return 0;
			return info.totalram / (1024 * 1024);
		#endif

		#ifdef COLUMBUS_PLATFORM_WINDOWS
			MEMORYSTATUSEX memInfo;
			memInfo.dwLength = sizeof(memInfo);
			GlobalMemoryStatusEx(&memInfo);
			return memInfo.ullTotalPhys / (1024 * 1024);
		#endif
	}

	unsigned long Info::getRAMFree()
	{
		#ifdef COLUMBUS_PLATFORM_LINUX
			struct sysinfo info;
			if (sysinfo(&info) == -1) return 0;
			return info.freeram / (1024 * 1024);
		#endif

		#ifdef COLUMBUS_PLATFORM_WINDOWS
			MEMORYSTATUSEX memInfo;
			memInfo.dwLength = sizeof(memInfo);
			GlobalMemoryStatusEx(&memInfo);
			return memInfo.ullAvailPhys / (1024 * 1024);
		#endif
	}

	int Info::getRAMUsage()
	{
		#ifdef COLUMBUS_PLATFORM_LINUX
			struct sysinfo info;
			if (sysinfo(&info) == -1) return 0;
			float usage = static_cast<float>(getRAMFree()) / static_cast<float>(getRAMSize());
			return static_cast<int>(100 - usage * 100);
		#endif

		#ifdef COLUMBUS_PLATFORM_WINDOWS
			MEMORYSTATUSEX memInfo;
			memInfo.dwLength = sizeof(memInfo);
			GlobalMemoryStatusEx(&memInfo);
			return memInfo.dwMemoryLoad;
		#endif

		return 0;
	}

}


#undef COLUMBUS_PLATFORM_WINDOWS
#undef COLUMBUS_PLATFORM_LINUX





