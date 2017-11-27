#include <cstdlib>
#include <fstream>
#include <string>
#include <iostream>

#ifdef __linux__
	#include <sys/sysinfo.h>
	#include <unistd.h>
#endif

#ifdef _WIN32
	#define COLUMBUS_PLATFORM_WINDOWS
#endif

#ifdef _WIN64
	#define COLUMBUS_PLATFORM_WINDOWS
#endif

#ifdef COLUMBUS_PLATFORM_WINDOWS
	#include <windows.h>
	#include <psapi.h>
#endif

namespace Columbus
{

	int GetCPUCount();
	int GetCPUCacheSize();
	int GetCPUUsage();
	float GetCPUTemperature();
	unsigned long GetRAMSize();
	unsigned long GetRAMFree();
	unsigned long GetRAMThis();
	int GetRAMUsage();
	int GetGPUUsage();
	float GetGPUTemperature();
	unsigned long GetGPUMemorySize();
	unsigned long GetGPUMemoryFree();
	int GetGPUMemoryUsage();

	int GetCPUCount()
	{
		#ifdef __linux__
			return sysconf(_SC_NPROCESSORS_ONLN);
		#endif

		#ifdef COLUMBUS_PLATFORM_WINDOWS
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			return sysinfo.dwNumberOfProcessors;
		#endif

		return 0;
	}

	int GetCPUCacheSize()
	{
		#ifdef __linux__
			return sysconf(_SC_PAGESIZE);
		#endif

		#ifdef COLUMBUS_PLATFORM_WINDOWS
			SYSTEM_INFO sysinfo;
			GetSystemInfo(&sysinfo);
			return sysinfo.dwPageSize;
		#endif	

		return 0;
	}

	int GetCPUUsage()
	{
		#ifdef __linux__
			double load[3];
			getloadavg(load, 3);
			return static_cast<int>(load[0] * 100);
		#endif

		#ifdef COLUMBUS_PLATFORM_WINDOWS

		#endif

		return 0;
	}

	float GetCPUTemperature()
	{
		#ifdef __linux__
			int hwmon = 0;
			std::ifstream name1("/sys/class/hwmon/hwmon0/name");
			std::ifstream name2("/sys/class/hwmon/hwmon1/name");
			if (name1.is_open() == false || name2.is_open() == false) return 0;
			std::string tmp;

			name1 >> tmp;
			if (tmp.find("coretemp") != std::string::npos) hwmon = 0;
			name2 >> tmp;
			if (tmp.find("coretemp") != std::string::npos) hwmon = 1;

			name1.close();
			name2.close();

			float t = 0.0;
			std::string hw;
			if (hwmon == 0) hw = "0";
			if (hwmon == 1) hw = "1";

			std::ifstream tmp1(("/sys/class/hwmon/hwmon" + hw + "/temp1_input").c_str());
			std::ifstream tmp2(("/sys/class/hwmon/hwmon" + hw + "/temp2_input").c_str());
			std::ifstream tmp3(("/sys/class/hwmon/hwmon" + hw + "/temp3_input").c_str());
			if (tmp1.is_open() != false)
			{
				tmp1 >> t;
				return (t / 1000);
			}

			if (tmp2.is_open() != false)
			{
				tmp2 >> t;
				return (t / 1000);
			}

			if (tmp3.is_open() != false)
			{
				tmp1 >> t;
				return (t / 1000);
			}
			return 0.0;

		#endif

		#ifdef COLUMBUS_PLATFORM_WINDOWS
			BSTR strClassProp = SysAllocString(L"CurrentTemperature");
			std::cout << strClassProp << std::endl;
		#endif

		return 0;
	}

	unsigned long GetRAMSize()
	{
		#ifdef __linux__
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

		return 0;
	}

	unsigned long GetRAMFree()
	{
		#ifdef __linux__
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

		return 0;
	}

	unsigned long GetRAMThis()
	{
		#ifdef COLUMBUS_PLATFORM_WINDOWS
			PROCESS_MEMORY_COUNTERS meminfo;
			GetProcessMemoryInfo(GetCurrentProcess(), &meminfo, sizeof(meminfo));
			return meminfo.WorkingSetSize / (1024 * 1024);
		#endif

		return 0;
	}

	int GetRAMUsage()
	{
		#ifdef __linux__
			struct sysinfo info;
			if (sysinfo(&info) == -1) return 0;
			float usage = static_cast<float>(GetRAMFree()) / static_cast<float>(GetRAMSize());
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

	float GetGPUTemperature()
	{
		#ifdef __linux__
			int hwmon = 0;
			std::ifstream name1("/sys/class/hwmon/hwmon0/name");
			std::ifstream name2("/sys/class/hwmon/hwmon1/name");
			if (name1.is_open() == false || name2.is_open() == false) return 0;
			std::string tmp;

			name1 >> tmp;
			if (tmp.find("coretemp") == std::string::npos) hwmon = 0;
			name2 >> tmp;
			if (tmp.find("coretemp") == std::string::npos) hwmon = 1;

			name1.close();
			name2.close();

			float t = 0.0;
			std::string hw;
			if (hwmon == 0) hw = "0";
			if (hwmon == 1) hw = "1";

			std::ifstream tmp1(("/sys/class/hwmon/hwmon" + hw + "/temp1_input").c_str());
			std::ifstream tmp2(("/sys/class/hwmon/hwmon" + hw + "/temp2_input").c_str());
			std::ifstream tmp3(("/sys/class/hwmon/hwmon" + hw + "/temp3_input").c_str());
			if (tmp1.is_open() != false)
			{
				tmp1 >> t;
				return (t / 1000);
			}

			if (tmp2.is_open() != false)
			{
				tmp2 >> t;
				return (t / 1000);
			}

			if (tmp3.is_open() != false)
			{
				tmp1 >> t;
				return (t / 1000);
			}
			return 0.0;

		#endif

		return 0;
	}

}




