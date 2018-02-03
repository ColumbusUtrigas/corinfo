#ifdef __linux
	#define COLUMBUS_PLATFORM_LINUX
	#include <sys/sysinfo.h>
	#include <unistd.h>
#endif

#include <cstdlib>

namespace Columbus
{

	class Info
	{
	public:
		static unsigned int getCPUCount();

		static unsigned long getRAMSize();
		static unsigned long getRAMFree();
		static int getRAMUsage();
	};

	unsigned int Info::getCPUCount()
	{
		#ifdef COLUMBUS_PLATFORM_LINUX
			return sysconf(_SC_NPROCESSORS_ONLN);
		#endif
	}

	unsigned long Info::getRAMSize()
	{
		#ifdef COLUMBUS_PLATFORM_LINUX
			struct sysinfo info;
			if (sysinfo(&info) == -1) return 0;
			return info.totalram / (1024 * 1024);
		#endif
	}

	unsigned long Info::getRAMFree()
	{
		#ifdef COLUMBUS_PLATFORM_LINUX
			struct sysinfo info;
			if (sysinfo(&info) == -1) return 0;
			return info.freeram / (1024 * 1024);
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
	}

}


#undef COLUMBUS_PLATFORM_LINUX





