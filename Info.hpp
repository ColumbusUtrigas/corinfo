#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace Columbus
{

	int GetCPUCount();



	int GetCPUCount()
	{
		#ifdef __linux__
			int counter = 0;

			FILE *cpuinfo = fopen("/proc/cpuinfo", "rb");
			
			char* str;
			size_t size;

			while(getline(&str, &size, cpuinfo) != -1)
			{
				if (strstr(str, "processor") != 0)
					counter++;
			}
			
			fclose(cpuinfo);
			return counter;
		#endif
	}

}




