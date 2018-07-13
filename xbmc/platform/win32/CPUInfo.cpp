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

#include "platform/win32/CharsetConverter.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/Temperature.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>
#include <intrin.h>
#include <Pdh.h>
#include <PdhMsg.h>

#pragma comment(lib, "Pdh.lib")

CCPUInfo::CCPUInfo()
{
  using KODI::PLATFORM::WINDOWS::FromW;

  HKEY hKeyCpuRoot;

  if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor", 0, KEY_READ, &hKeyCpuRoot) == ERROR_SUCCESS)
  {
    DWORD num = 0;
    std::vector<CoreInfo> cpuCores;
    wchar_t subKeyName[200]; // more than enough
    DWORD subKeyNameLen = sizeof(subKeyName) / sizeof(wchar_t);
    while (RegEnumKeyExW(hKeyCpuRoot, num++, subKeyName, &subKeyNameLen, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS)
    {
      HKEY hCpuKey;
      if (RegOpenKeyExW(hKeyCpuRoot, subKeyName, 0, KEY_QUERY_VALUE, &hCpuKey) == ERROR_SUCCESS)
      {
        CoreInfo cpuCore;
        if (swscanf_s(subKeyName, L"%i", &cpuCore.m_id) != 1)
          cpuCore.m_id = num - 1;
        wchar_t buf[300]; // more than enough
        DWORD bufSize = sizeof(buf);
        DWORD valType;
        if (RegQueryValueExW(hCpuKey, L"ProcessorNameString", nullptr, &valType, LPBYTE(buf), &bufSize) == ERROR_SUCCESS &&
            valType == REG_SZ)
        {
          cpuCore.m_strModel = FromW(buf, bufSize / sizeof(wchar_t));
          cpuCore.m_strModel = cpuCore.m_strModel.substr(0, cpuCore.m_strModel.find(char(0))); // remove extra null terminations
          StringUtils::RemoveDuplicatedSpacesAndTabs(cpuCore.m_strModel);
          StringUtils::Trim(cpuCore.m_strModel);
        }
        bufSize = sizeof(buf);
        if (RegQueryValueExW(hCpuKey, L"VendorIdentifier", nullptr, &valType, LPBYTE(buf), &bufSize) == ERROR_SUCCESS &&
            valType == REG_SZ)
        {
          cpuCore.m_strVendor = FromW(buf, bufSize / sizeof(wchar_t));
          cpuCore.m_strVendor = cpuCore.m_strVendor.substr(0, cpuCore.m_strVendor.find(char(0))); // remove extra null terminations
        }
        DWORD mhzVal;
        bufSize = sizeof(mhzVal);
        if (RegQueryValueExW(hCpuKey, L"~MHz", nullptr, &valType, LPBYTE(&mhzVal), &bufSize) == ERROR_SUCCESS &&
            valType == REG_DWORD)
          cpuCore.m_fSpeed = double(mhzVal);

        RegCloseKey(hCpuKey);

        if (cpuCore.m_strModel.empty())
          cpuCore.m_strModel = "Unknown";
        cpuCores.push_back(cpuCore);
      }
      subKeyNameLen = sizeof(subKeyName) / sizeof(wchar_t); // restore length value
    }
    RegCloseKey(hKeyCpuRoot);
    std::sort(cpuCores.begin(), cpuCores.end()); // sort cores by id
    for (size_t i = 0; i < cpuCores.size(); i++)
      m_cores[i] = cpuCores[i]; // add in sorted order
  }

  if (!m_cores.empty())
    m_cpuModel = m_cores.begin()->second.m_strModel;
  else
    m_cpuModel = "Unknown";

  SYSTEM_INFO siSysInfo;
  GetNativeSystemInfo(&siSysInfo);
  m_cpuCount = siSysInfo.dwNumberOfProcessors;

  if (PdhOpenQueryW(nullptr, 0, &m_cpuQueryFreq) == ERROR_SUCCESS)
  {
    if (PdhAddEnglishCounterW(m_cpuQueryFreq, L"\\Processor Information(0,0)\\Processor Frequency", 0, &m_cpuFreqCounter) != ERROR_SUCCESS)
      m_cpuFreqCounter = nullptr;
  }
  else
    m_cpuQueryFreq = nullptr;

  if (PdhOpenQueryW(nullptr, 0, &m_cpuQueryLoad) == ERROR_SUCCESS)
  {
    for (size_t i = 0; i < m_cores.size(); i++)
    {
      if (PdhAddEnglishCounterW(m_cpuQueryLoad, StringUtils::Format(L"\\Processor(%d)\\%% Idle Time", int(i)).c_str(), 0, &m_cores[i].m_coreCounter) != ERROR_SUCCESS)
        m_cores[i].m_coreCounter = nullptr;
    }
  }
  else
    m_cpuQueryLoad = nullptr;

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

CCPUInfo::~CCPUInfo()
{
  if (m_cpuQueryFreq)
    PdhCloseQuery(m_cpuQueryFreq);

  if (m_cpuQueryLoad)
    PdhCloseQuery(m_cpuQueryLoad);
}

float CCPUInfo::GetCPUFrequency()
{
  // Get CPU frequency, scaled to MHz.
  if (m_cpuFreqCounter && PdhCollectQueryData(m_cpuQueryFreq) == ERROR_SUCCESS)
  {
    PDH_RAW_COUNTER cnt;
    DWORD cntType;
    if (PdhGetRawCounterValue(m_cpuFreqCounter, &cntType, &cnt) == ERROR_SUCCESS &&
        (cnt.CStatus == PDH_CSTATUS_VALID_DATA || cnt.CStatus == PDH_CSTATUS_NEW_DATA))
    {
      return float(cnt.FirstValue);
    }
  }

  if (!m_cores.empty())
    return float(m_cores.begin()->second.m_fSpeed);
  else
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
  FILETIME idleTime;
  FILETIME kernelTime;
  FILETIME userTime;
  if (GetSystemTimes(&idleTime, &kernelTime, &userTime) == 0)
    return false;

  idle = (uint64_t(idleTime.dwHighDateTime) << 32) + uint64_t(idleTime.dwLowDateTime);
  // returned "kernelTime" includes "idleTime"
  system = (uint64_t(kernelTime.dwHighDateTime) << 32) + uint64_t(kernelTime.dwLowDateTime) - idle;
  user = (uint64_t(userTime.dwHighDateTime) << 32) + uint64_t(userTime.dwLowDateTime);

  if (m_cpuFreqCounter && PdhCollectQueryData(m_cpuQueryLoad) == ERROR_SUCCESS)
  {
    for (std::map<int, CoreInfo>::iterator it = m_cores.begin(); it != m_cores.end(); ++it)
    {
      CoreInfo& curCore = it->second; // simplify usage
      PDH_RAW_COUNTER cnt;
      DWORD cntType;
      if (curCore.m_coreCounter && PdhGetRawCounterValue(curCore.m_coreCounter, &cntType, &cnt) == ERROR_SUCCESS &&
          (cnt.CStatus == PDH_CSTATUS_VALID_DATA || cnt.CStatus == PDH_CSTATUS_NEW_DATA))
      {
        const LONGLONG coreTotal = cnt.SecondValue,
                       coreIdle  = cnt.FirstValue;
        const LONGLONG deltaTotal = coreTotal - curCore.m_total,
                       deltaIdle  = coreIdle - curCore.m_idle;
        const double load = static_cast<double>((deltaTotal - deltaIdle) * 100.0) / static_cast<double>(deltaTotal);

        // win32 has some problems with calculation of load if load close to zero
        curCore.m_fPct = (load < 0) ? 0 : load;
        if (load >= 0 || deltaTotal > 5 * 10 * 1000 * 1000) // do not update (smooth) values for 5 seconds on negative loads
        {
          curCore.m_total = coreTotal;
          curCore.m_idle = coreIdle;
        }
      }
      else
        curCore.m_fPct = static_cast<double>(m_lastUsedPercentage); // use CPU average as fallback
    }
  }
  else
    for (std::map<int, CoreInfo>::iterator it = m_cores.begin(); it != m_cores.end(); ++it)
      it->second.m_fPct = static_cast<double>(m_lastUsedPercentage); // use CPU average as fallback

  return true;
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

