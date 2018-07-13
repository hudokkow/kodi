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

#include "CPUInfo.h"

#include "utils/StringUtils.h"

#include <map>
#include <string>

#if defined(TARGET_LINUX) && defined(HAS_NEON) && !defined(TARGET_ANDROID)
#include <elf.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/hwcap.h>
#include <linux/auxvec.h>
#endif

int CCPUInfo::GetCPUUsedPercentage()
{
  int result = 0;

  if (!m_nextUsedReadTime.IsTimePast())
    return m_lastUsedPercentage;

#if defined(TARGET_DARWIN)
  result = m_pResourceCounter->GetCPUUsage();
#else
  unsigned long long userTicks;
  unsigned long long niceTicks;
  unsigned long long systemTicks;
  unsigned long long idleTicks;
  unsigned long long ioTicks;

  if (!ReadProcStat(userTicks, niceTicks, systemTicks, idleTicks, ioTicks))
    return m_lastUsedPercentage;

  userTicks -= m_userTicks;
  niceTicks -= m_niceTicks;
  systemTicks -= m_systemTicks;
  idleTicks -= m_idleTicks;
  ioTicks -= m_ioTicks;

  if (userTicks + niceTicks + systemTicks + idleTicks + ioTicks == 0)
    return m_lastUsedPercentage;
  result = static_cast<int>(double(userTicks + niceTicks + systemTicks) * 100.0 / double(userTicks + niceTicks + systemTicks + idleTicks + ioTicks) + 0.5);

  m_userTicks += userTicks;
  m_niceTicks += niceTicks;
  m_systemTicks += systemTicks;
  m_idleTicks += idleTicks;
  m_ioTicks += ioTicks;
#endif
  m_lastUsedPercentage = result;
  m_nextUsedReadTime.Set(MINIMUM_TIME_BETWEEN_READS);

  return result;
}

bool CCPUInfo::HasCoreId(int nCoreId) const
{
  std::map<int, CoreInfo>::const_iterator iter = m_cores.find(nCoreId);
  if (iter != m_cores.end())
    return true;
  return false;
}

const CoreInfo &CCPUInfo::GetCoreInfo(int nCoreId)
{
  std::map<int, CoreInfo>::iterator iter = m_cores.find(nCoreId);
  if (iter != m_cores.end())
    return iter->second;

  static CoreInfo dummy;
  return dummy;
}

std::string CCPUInfo::GetCoresUsageString() const
{
  std::string strCores;
  if (!m_cores.empty())
  {
    for (std::map<int, CoreInfo>::const_iterator it = m_cores.begin(); it != m_cores.end(); ++it)
    {
      if (!strCores.empty())
        strCores += ' ';
      if (it->second.m_fPct < 10.0)
        strCores += StringUtils::Format("#%d: %1.1f%%", it->first, it->second.m_fPct);
      else
        strCores += StringUtils::Format("#%d: %3.0f%%", it->first, it->second.m_fPct);
    }
  }
  else
  {
    strCores += StringUtils::Format("%3.0f%%", static_cast<double>(m_lastUsedPercentage));
  }
  return strCores;
}

bool CCPUInfo::HasNeon()
{
  static int has_neon = -1;
#if defined (TARGET_ANDROID)
  if (has_neon == -1)
    has_neon = (CAndroidFeatures::HasNeon()) ? 1 : 0;

#elif defined(TARGET_DARWIN_IOS)
  has_neon = 1;

#elif defined(TARGET_LINUX) && defined(HAS_NEON)
  #if defined(__LP64__)
  has_neon = 1;
#else
  if (has_neon == -1)
  {
    has_neon = 0;
    // why are we not looking at the Features in
    // /proc/cpuinfo for neon ?
    int fd = open("/proc/self/auxv", O_RDONLY);
    if (fd >= 0)
    {
      Elf32_auxv_t auxv;
      while (read(fd, &auxv, sizeof(Elf32_auxv_t)) == sizeof(Elf32_auxv_t))
      {
        if (auxv.a_type == AT_HWCAP)
        {
          has_neon = (auxv.a_un.a_val & HWCAP_NEON) ? 1 : 0;
          break;
        }
      }
      close(fd);
    }
  }
#endif

#endif

  return has_neon == 1;
}

CCPUInfo g_cpuInfo;

