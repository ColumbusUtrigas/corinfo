#include <cstdlib>
#include <fstream>
#include <string>

#ifdef __linux__
	#include <sys/sysinfo.h>
#endif

namespace Columbus
{

	int GetCPUCount();
	int GetCPUCacheSize();
	unsigned long GetRAMSize();
	unsigned long GetRamAvaliable();

	int GetCPUCount()
	{
		#ifdef __linux__
			int counter = 0;
			std::ifstream cpuinfo("/proc/cpuinfo");
			if (cpuinfo.is_open() == false) return 0;

			std::string line;

			while(getline(cpuinfo, line))
			{
				if (line.find("processor") != std::string::npos)
					counter++;
			}

			cpuinfo.close();
			return counter;
		#endif
	}

	int GetCPUCacheSize()
	{
		#ifdef __linux__
			std::ifstream cpuinfo("/proc/cpuinfo");
			if (cpuinfo.is_open() == false) return 0;

			std::string line;

			while(getline(cpuinfo, line))
			{
				if (line.find("cache size") != std::string::npos)
				{
					size_t sz = line.find(":") + 1;
					size_t kb = line.find("KB");
					return std::atoi(line.substr(sz, kb - sz).c_str());
				}
			}

			cpuinfo.close();
			return 0;
		#endif
	}

	unsigned long GetRAMSize()
	{
		#ifdef __linux__
			struct sysinfo info;
			if (sysinfo(&info) == -1) return 0;
			return info.totalram;
		#endif
	}

}




