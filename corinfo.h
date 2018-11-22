/*
* Part of corinfo, a library for obtaining CPU, RAM and GPU information.
* https://github.com/ColumbusUtrigas/corinfo
*
* Copyright (c) 2018 ColumbusUtrigas.
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#ifndef CORINFO_H
#define CORINFO_H

#if !defined(__linux) && !defined(_WIN32)
	#error "Unsupported platform"
#endif

#ifdef __linux
	#include <sys/sysinfo.h>
	#include <sys/statvfs.h>
	#include <sys/types.h>
	#include <unistd.h>
#endif

#ifdef _WIN32
	#include <windows.h>
	#include <Powrprof.h>
	#pragma comment(lib, "Powrprof.lib")
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/*!
* @file corinfo.h
*/

struct corinfo_cpu
{
	uint32_t Count;     //!< Number of CPU cores.
	uint32_t Frequency; //!< CPU frequency, in MHz.
	uint32_t Family;
	uint32_t Model;

	uint8_t Vendor[12];
	uint8_t Name[48];

	uint8_t MMX;
	uint8_t SSE;
	uint8_t SSE2;
	uint8_t SSE3;
	uint8_t SSE41;
	uint8_t SSE42;
	uint8_t AVX;
};

struct corinfo_ram
{
	uint64_t Total; //!< Physical RAM size, in KB.
	uint64_t Free;  //!< Available physical RAM size, in KB.
	uint32_t Usage; //!< A number between 0 and 100 that specifies the approximate percentage of physical memory that is in use.
};

struct corinfo_hdd
{
	uint64_t Total; //!< HDD size, in KB.
	uint64_t Free;  //!< Available HDD size, in KB.
	uint32_t Usage; //!< A number between 0 and 100 that specifies the approximate percentage of HDD memory that is in use.
};

struct corinfo
{
	struct corinfo_cpu Cpu;
	struct corinfo_ram Ram;
	struct corinfo_hdd Hdd;
};

/**
* @brief Main library function.
*
* ## Example
*
* ```c
* struct corinfo info;
* if (corinfo_GetInfo(&info) == -1) error();
* ```
*
* @param info Pointer to corinfo struct to store data.
* @return 0 in success, otherwise -1.
*/
int corinfo_GetInfo(struct corinfo* info);

static int __cpu_info(struct corinfo* info);
static int __ram_info(struct corinfo* info);
static int __hdd_info(struct corinfo* info);

#ifdef __linux
	static char* skip_whitespace(char* str)
	{
		while (*str == ' ' || *str == '\n' || *str == '\t' || *str == '\r' || *str == '\f' || *str == '\v') str++;
		return str;
	}

	int __cpu_info(struct corinfo* info)
	{
		info->Cpu.Count = sysconf(_SC_NPROCESSORS_ONLN);

		FILE* f = fopen("/proc/cpuinfo", "r");
		if (f == NULL) return -1;

		char*   line = NULL;
		size_t  line_length = 0;
		ssize_t line_read = 0;

		while ((line_read = getline(&line, &line_length, f)) != -1)
		{
			if (memcmp(line, "cpu MHz", 7) == 0)
			{
				info->Cpu.Frequency = atoi(strchr(line, ':') + 1);
			}

			if (memcmp(line, "cpu family", 10) == 0)
			{
				info->Cpu.Family = atoi(strchr(line, ':') + 1);
			}

			if (memcmp(line, "model\t", 6) == 0)
			{
				info->Cpu.Model = atoi(strchr(line, ':') + 1);
			}

			if (memcmp(line, "vendor_id", 9) == 0)
			{
				memcpy(info->Cpu.Vendor, &line[line_read] - 13, 12);
			}

			if (memcmp(line, "model name", 10) == 0)
			{
				char* str = skip_whitespace(strchr(line, ':') + 1);
				size_t len = strlen(str);
				for (int i = 0; i < len; i++) if (str[i] == '\n') str[i] = ' ';
				memcpy(info->Cpu.Name, str, len);
			}

			if (memcmp(line, "flags", 5) == 0)
			{
				char* str = strchr(line, ':') + 1;

				info->Cpu.MMX   = strstr(str, " mmx")    != NULL;
				info->Cpu.SSE   = strstr(str, " sse")    != NULL;
				info->Cpu.SSE2  = strstr(str, " sse2")   != NULL;
				info->Cpu.SSE3  = strstr(str, " ssse3")  != NULL;
				info->Cpu.SSE41 = strstr(str, " sse4_1") != NULL;
				info->Cpu.SSE42 = strstr(str, " sse4_2") != NULL;
				info->Cpu.AVX   = strstr(str, " avx")    != NULL;
			}
		}

		if (line) free(line);

		fclose(f);

		return 0;
	}

	int __ram_info(struct corinfo* info)
	{
		struct sysinfo sys;
		if (sysinfo(&sys) == -1) return -1;

		info->Ram.Total = sys.totalram / 1024;
		info->Ram.Free  = sys.freeram / 1024;

		if (info->Ram.Total != 0)
		{
			info->Ram.Usage = 100 - (info->Ram.Free / (float)info->Ram.Total) * 100;
		}

		return 0;
	}

	int __hdd_info(struct corinfo* info)
	{
		struct statvfs hd;
		if (statvfs("./", &hd) == -1) return -1;

		info->Hdd.Total = hd.f_frsize * hd.f_blocks / 1024;
		info->Hdd.Free  = hd.f_frsize * hd.f_bavail / 1024;

		if (info->Hdd.Total != 0)
		{
			info->Hdd.Usage = 100 - (info->Hdd.Free / (double)info->Hdd.Total) * 100;
		}

		return 0;
	}

	int corinfo_GetInfo(struct corinfo* info)
	{
		if (info == NULL) return -1;
		*info = (struct corinfo){0};

		if (__cpu_info(info) == -1) return -1;
		if (__ram_info(info) == -1) return -1;
		if (__hdd_info(info) == -1) return -1;

		return 0;
	}
#endif



#ifdef _WIN32
	int corinfo_GetInfo(struct corinfo* info)
	{
		if (info == NULL) return -1;
		__null_corinfo(info);

		SYSTEM_INFO sys;
		MEMORYSTATUSEX mem;

		struct
		{
			ULONG  Number;
			ULONG  MaxMhz;
			ULONG  CurrentMhz;
			ULONG  MhzLimit;
			ULONG  MaxIdleState;
			ULONG  CurrentIdleState;
		} PROCESSOR_POWER_INFORMATION, *PPI;

		mem.dwLength = sizeof(mem);

		GetSystemInfo(&sys);
		GlobalMemoryStatusEx(&mem);

		BYTE* buf = (BYTE*)malloc(sizeof(PROCESSOR_POWER_INFORMATION) * 4);
		if (buf == NULL) return -1;

		CallNtPowerInformation(ProcessorInformation, NULL, 0, buf, sizeof(PROCESSOR_POWER_INFORMATION) * sys.dwNumberOfProcessors);

		PPI = buf;

		int cpu[4];
		// CPUID 0x00 code returns in EAX maximum CPUID code and vendor string in EBX, EDX, ECX.
		__cpuid(cpu, 0x00000000);
		__vendor_string(cpu, info->VendorString);
		__brand_string(cpu, info->BrandString);
		__extensions(cpu, info);

		info->CpuCount = sys.dwNumberOfProcessors;
		info->CpuFrequency = PPI->MaxMhz;

		info->RamSize  = mem.ullTotalPhys / 1024;
		info->RamFree  = mem.ullAvailPhys / 1024;
		info->RamUsage = mem.dwMemoryLoad;

		free(buf);

		return 0;
	}
#endif

#endif


