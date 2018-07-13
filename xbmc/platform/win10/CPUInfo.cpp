/*
 *      Copyright (C) 2005-2018 Team Kodi
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

#include "platform/CPUInfo.h"

#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/Temperature.h"

#include <string>
#include <intrin.h>
#include <Pdh.h>
#include <PdhMsg.h>

#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.System.Diagnostics.h>

CCPUInfo::CCPUInfo()
{
  SYSTEM_INFO siSysInfo;
  GetNativeSystemInfo(&siSysInfo);
  m_cpuCount = siSysInfo.dwNumberOfProcessors;
  m_cpuModel = "Unknown";

  StringUtils::Replace(m_cpuModel, '\r', ' ');
  StringUtils::Replace(m_cpuModel, '\n', ' ');
  StringUtils::RemoveDuplicatedSpacesAndTabs(m_cpuModel);
  StringUtils::Trim(m_cpuModel);

  /* Set some default for empty string variables */
  if (m_cpuBogoMips.empty())
    m_cpuBogoMips = "N/A";
  if (m_cpuHardware.empty())
    m_cpuHardware = "N/A";
  if (m_cpuRevision.empty())
    m_cpuRevision = "N/A";
  if (m_cpuSerial.empty())
    m_cpuSerial = "N/A";

  ReadProcStat(m_userTicks, m_niceTicks, m_systemTicks, m_idleTicks, m_ioTicks);
  m_nextUsedReadTime.Set(MINIMUM_TIME_BETWEEN_READS);

  ReadCPUFeatures();

  // Set MMX2 when SSE is present as SSE is a superset of MMX2 and Intel doesn't set the MMX2 cap
  if (m_cpuFeatures & CPU_FEATURE_SSE)
    m_cpuFeatures |= CPU_FEATURE_MMX2;

  if (HasNeon())
    m_cpuFeatures |= CPU_FEATURE_NEON;
}

CCPUInfo::~CCPUInfo() = default;

float CCPUInfo::GetCPUFrequency()
{
  CLog::Log(LOGDEBUG, "%s is not implemented", __FUNCTION__);
  return 0.f;
}

bool CCPUInfo::GetCPUTemperature(CTemperature& temperature)
{
  CLog::Log(LOGDEBUG, "%s is not implemented", __FUNCTION__);
  return false;
}

bool CCPUInfo::ReadProcStat(unsigned long long& user, unsigned long long& nice,
                            unsigned long long& system, unsigned long long& idle, unsigned long long& io)
{
  nice = 0;
  io = 0;

  if (winrt::Windows::Foundation::Metadata::ApiInformation::IsTypePresent(L"Windows.System.Diagnostics.SystemDiagnosticInfo"))
  {
    auto diagnostic = winrt::Windows::System::Diagnostics::SystemDiagnosticInfo::GetForCurrentSystem();
    auto usage = diagnostic.CpuUsage();
    auto report = usage.GetReport();

    user = report.UserTime().count();
    idle = report.IdleTime().count();
    system = report.KernelTime().count() - idle;
    return true;
  }
  else
    return false;
}

void CCPUInfo::ReadCPUFeatures()
{
#ifndef _M_ARM
  int CPUInfo[4]; // receives EAX, EBX, ECD and EDX in that order

  __cpuid(CPUInfo, 0);
  int MaxStdInfoType = CPUInfo[0];

  if (MaxStdInfoType >= CPUID_INFOTYPE_STANDARD)
  {
    __cpuid(CPUInfo, CPUID_INFOTYPE_STANDARD);
    if (CPUInfo[CPUINFO_EDX] & CPUID_00000001_EDX_MMX)
      m_cpuFeatures |= CPU_FEATURE_MMX;
    if (CPUInfo[CPUINFO_EDX] & CPUID_00000001_EDX_SSE)
      m_cpuFeatures |= CPU_FEATURE_SSE;
    if (CPUInfo[CPUINFO_EDX] & CPUID_00000001_EDX_SSE2)
      m_cpuFeatures |= CPU_FEATURE_SSE2;
    if (CPUInfo[CPUINFO_ECX] & CPUID_00000001_ECX_SSE3)
      m_cpuFeatures |= CPU_FEATURE_SSE3;
    if (CPUInfo[CPUINFO_ECX] & CPUID_00000001_ECX_SSSE3)
      m_cpuFeatures |= CPU_FEATURE_SSSE3;
    if (CPUInfo[CPUINFO_ECX] & CPUID_00000001_ECX_SSE4)
      m_cpuFeatures |= CPU_FEATURE_SSE4;
    if (CPUInfo[CPUINFO_ECX] & CPUID_00000001_ECX_SSE42)
      m_cpuFeatures |= CPU_FEATURE_SSE42;
  }

  __cpuid(CPUInfo, 0x80000000);
  int MaxExtInfoType = CPUInfo[0];

  if (MaxExtInfoType >= CPUID_INFOTYPE_EXTENDED)
  {
    __cpuid(CPUInfo, CPUID_INFOTYPE_EXTENDED);

    if (CPUInfo[CPUINFO_EDX] & CPUID_80000001_EDX_MMX)
      m_cpuFeatures |= CPU_FEATURE_MMX;
    if (CPUInfo[CPUINFO_EDX] & CPUID_80000001_EDX_MMX2)
      m_cpuFeatures |= CPU_FEATURE_MMX2;
    if (CPUInfo[CPUINFO_EDX] & CPUID_80000001_EDX_3DNOW)
      m_cpuFeatures |= CPU_FEATURE_3DNOW;
    if (CPUInfo[CPUINFO_EDX] & CPUID_80000001_EDX_3DNOWEXT)
      m_cpuFeatures |= CPU_FEATURE_3DNOWEXT;
  }
#endif // ! _M_ARM
}

