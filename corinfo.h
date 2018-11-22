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
	#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/*!
* @file corinfo.h
*/

struct corinfo
{
	uint32_t CpuCount;     //!< Number of CPU cores.
	uint32_t CpuFrequency; //!< CPU frequency, in MHz.

	uint8_t  VendorString[12];
	uint8_t  BrandString[48];

	uint8_t MMX;
	uint8_t SSE;
	uint8_t SSE2;
	uint8_t SSE3;
	uint8_t SSE41;
	uint8_t SSE42;
	uint8_t AVX;

	uint32_t RamSize;   //!< Physical RAM size, in KB.
	uint32_t RamFree;   //!< Available physical RAM size, in KB.
	uint32_t RamUsage;  //!< A number between 0 and 100 that specifies the approximate percentage of physical memory that is in use.
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

#ifdef __linux
	#define __cpuid(cpu, command) \
	        asm volatile \
	        ("cpuid" : "=a" (cpu[0]), "=b" (cpu[1]), "=c" (cpu[2]), "=d" (cpu[3]) \
	         : "a" (command), "c" (0));
#endif

static void __null_corinfo(struct corinfo* info);
static void __vendor_string(int cpu[4], uint8_t* string);
static void __brand_string(int cpu[4], uint8_t* string);
static void __extensions(int cpu[4], struct corinfo* info);

#ifdef __linux
	int corinfo_GetInfo(struct corinfo* info)
	{
		if (info == NULL) return -1;
		__null_corinfo(info);

		struct sysinfo sys;
		if (sysinfo(&sys) == -1) return -1;

		int cpu[4];
		// CPUID 0x00 code returns in EAX maximum CPUID code and vendor string in EBX, EDX, ECX.
		__cpuid(cpu, 0x00000000);
		__vendor_string(cpu, info->VendorString);
		__brand_string(cpu, info->BrandString);
		__extensions(cpu, info);

		info->CpuCount = sysconf(_SC_NPROCESSORS_ONLN);

		// /proc/cpuinfo reading is necessary for CPUs not implementing CPUID 0x16 code.
		FILE* f = fopen("/proc/cpuinfo", "rb");
		if (f == NULL) return -1;

		char*   line = NULL;
		size_t  line_length = 0;
		ssize_t line_read = 0;

		while ((line_read = getline(&line, &line_length, f)) != -1)
		{
			if (memcmp(line, "cpu MHz", 7) == 0)
			{
				info->CpuFrequency = atoi(strchr(line, ':') + 2);
				break;
			}
		}

		if (line) free(line);

		fclose(f);

		info->RamSize = sys.totalram / 1024;
		info->RamFree = sys.freeram / 1024;
		info->RamUsage = 100 - (info->RamFree / (float)info->RamSize) * 100;

		return 0;
	}
#endif



#ifdef _WIN32
	int corinfo_GetInfo(struct corinfo* info)
	{

	}
#endif


void __null_corinfo(struct corinfo* info)
{
	info->CpuCount = 0;
	info->CpuFrequency = 0;

	for (int i = 0; i < 12; i++) info->VendorString[i] = 0;
	for (int i = 0; i < 48; i++) info->BrandString[i]  = 0;

	info->MMX   = 0;
	info->SSE   = 0;
	info->SSE2  = 0;
	info->SSE3  = 0;
	info->SSE41 = 0;
	info->SSE42 = 0;
	info->AVX   = 0;

	info->RamSize  = 0;
	info->RamFree  = 0;
	info->RamUsage = 0;
}

void __vendor_string(int cpu[4], uint8_t* string)
{
	for (int i = 0; i < 4; i++)
	{
		string[i + 0] = (cpu[1] >> (i * 8)) & 0xFF;
		string[i + 4] = (cpu[3] >> (i * 8)) & 0xFF;
		string[i + 8] = (cpu[2] >> (i * 8)) & 0xFF;
	}
}

void __brand_string(int cpu[4], uint8_t* string)
{
	int commands[3] = { 0x80000002, 0x80000003, 0x80000004 };
	int offset = 0;

	for (int i = 0; i < 3; i++)
	{
		__cpuid(cpu, commands[i]);
		for (int j = 0; j < 4; j++)
		{
			string[offset + j + 0]  = (cpu[0] >> (j * 8)) & 0xFF;
			string[offset + j + 4]  = (cpu[1] >> (j * 8)) & 0xFF;
			string[offset + j + 8]  = (cpu[2] >> (j * 8)) & 0xFF;
			string[offset + j + 12] = (cpu[3] >> (j * 8)) & 0xFF;
		}

		offset += 16;
	}
}

void __extensions(int cpu[4], struct corinfo* info)
{
	__cpuid(cpu, 0x00000001);
	info->MMX   = (cpu[3] >> 23) & 0x1;
	info->SSE   = (cpu[3] >> 25) & 0x1;
	info->SSE2  = (cpu[3] >> 26) & 0x1;
	info->SSE3  = (cpu[2] >>  0) & 0x1;
	info->SSE41 = (cpu[2] >> 19) & 0x1;
	info->SSE42 = (cpu[2] >> 20) & 0x1;
	info->AVX   = (cpu[2] >> 28) & 0x1;
}

#undef __cpuid

#endif


