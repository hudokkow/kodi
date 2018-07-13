/*
 *      Copyright (C) 2005-2018 Team XBMC
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "threads/SystemClock.h"

#include <map>
#include <string>
#include <stdio.h>
#include <time.h>


#ifdef TARGET_WINDOWS
// avoid inclusion of <windows.h> and others
typedef void* HANDLE;
typedef HANDLE PDH_HQUERY;
typedef HANDLE PDH_HCOUNTER;
#endif

class CTemperature;
#if defined(TARGET_DARWIN)
class CLinuxResourceCounter;
#endif

#define CPU_FEATURE_MMX      1 << 0
#define CPU_FEATURE_MMX2     1 << 1
#define CPU_FEATURE_SSE      1 << 2
#define CPU_FEATURE_SSE2     1 << 3
#define CPU_FEATURE_SSE3     1 << 4
#define CPU_FEATURE_SSSE3    1 << 5
#define CPU_FEATURE_SSE4     1 << 6
#define CPU_FEATURE_SSE42    1 << 7
#define CPU_FEATURE_3DNOW    1 << 8
#define CPU_FEATURE_3DNOWEXT 1 << 9
#define CPU_FEATURE_ALTIVEC  1 << 10
#define CPU_FEATURE_NEON     1 << 11

#ifdef TARGET_WINDOWS
// Defines to help with calls to CPUID
#define CPUID_INFOTYPE_STANDARD 0x00000001
#define CPUID_INFOTYPE_EXTENDED 0x80000001

// Standard Features
// Bitmasks for the values returned by a call to cpuid with eax=0x00000001
#define CPUID_00000001_ECX_SSE3  (1<<0)
#define CPUID_00000001_ECX_SSSE3 (1<<9)
#define CPUID_00000001_ECX_SSE4  (1<<19)
#define CPUID_00000001_ECX_SSE42 (1<<20)

#define CPUID_00000001_EDX_MMX   (1<<23)
#define CPUID_00000001_EDX_SSE   (1<<25)
#define CPUID_00000001_EDX_SSE2  (1<<26)

// Extended Features
// Bitmasks for the values returned by a call to cpuid with eax=0x80000001
#define CPUID_80000001_EDX_MMX2     (1<<22)
#define CPUID_80000001_EDX_MMX      (1<<23)
#define CPUID_80000001_EDX_3DNOWEXT (1<<30)
#define CPUID_80000001_EDX_3DNOW    (1<<31)

// Help with the __cpuid intrinsic of MSVC
#define CPUINFO_EAX 0
#define CPUINFO_EBX 1
#define CPUINFO_ECX 2
#define CPUINFO_EDX 3
#endif

// In milliseconds
#define MINIMUM_TIME_BETWEEN_READS 500

struct CoreInfo
{
  int m_id = 0;
#ifdef TARGET_WINDOWS_DESKTOP
  double m_fSpeed = .0;
#endif
  double m_fPct = .0;
#ifdef TARGET_POSIX
  unsigned long long m_user = 0LL;
  unsigned long long m_nice = 0LL;
  unsigned long long m_system = 0LL;
  unsigned long long m_io = 0LL;
#elif TARGET_WINDOWS_DESKTOP
  PDH_HCOUNTER m_coreCounter = nullptr;
  unsigned long long m_total = 0;
#endif
  unsigned long long m_idle = 0LL;
  std::string m_strVendor;
  std::string m_strModel;
  std::string m_strBogoMips;
  std::string m_strHardware;
  std::string m_strRevision;
  std::string m_strSerial;
  bool operator<(const CoreInfo& other) const { return m_id < other.m_id; }
};

class CCPUInfo
{
public:
  CCPUInfo();
  ~CCPUInfo();

  int GetCPUUsedPercentage();
  int GetCPUCount() const { return m_cpuCount; }
  float GetCPUFrequency();
  bool GetCPUTemperature(CTemperature& temperature);
  std::string& GetCPUModel() { return m_cpuModel; }
  std::string& GetCPUBogoMips() { return m_cpuBogoMips; }
  std::string& GetCPUHardware() { return m_cpuHardware; }
  std::string& GetCPURevision() { return m_cpuRevision; }
  std::string& GetCPUSerial() { return m_cpuSerial; }

  const CoreInfo &GetCoreInfo(int nCoreId);
  bool HasCoreId(int nCoreId) const;

  std::string GetCoresUsageString() const;

  unsigned int GetCPUFeatures() const { return m_cpuFeatures; }

private:
  CCPUInfo(const CCPUInfo&) = delete;
  CCPUInfo& operator=(const CCPUInfo&) = delete;
  bool ReadProcStat(unsigned long long& user, unsigned long long& nice, unsigned long long& system,
                    unsigned long long& idle, unsigned long long& io);
  void ReadCPUFeatures();
  static bool HasNeon();

#ifdef TARGET_POSIX
  FILE* m_fProcStat = nullptr;
  FILE* m_fProcTemperature = nullptr;
  FILE* m_fCPUFreq = nullptr;
  bool m_cpuInfoForFreq = false;
#ifdef TARGET_DARWIN
  CLinuxResourceCounter *m_pResourceCounter = new CLinuxResourceCounter();
#endif
#elif TARGET_WINDOWS
  PDH_HQUERY m_cpuQueryFreq = nullptr;
  PDH_HQUERY m_cpuQueryLoad = nullptr;
  PDH_HCOUNTER m_cpuFreqCounter = nullptr;
#endif

  unsigned long long m_userTicks;
  unsigned long long m_niceTicks;
  unsigned long long m_systemTicks;
  unsigned long long m_idleTicks;
  unsigned long long m_ioTicks;

  int m_lastUsedPercentage = 0;
  XbmcThreads::EndTime m_nextUsedReadTime;
  std::string m_cpuModel;
  std::string m_cpuBogoMips;
  std::string m_cpuHardware;
  std::string m_cpuRevision;
  std::string m_cpuSerial;
  int m_cpuCount;
  unsigned int m_cpuFeatures = 0;

  std::map<int, CoreInfo> m_cores;

};

extern CCPUInfo g_cpuInfo;
