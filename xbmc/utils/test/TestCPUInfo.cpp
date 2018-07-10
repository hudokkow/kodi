/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#if defined(TARGET_WINDOWS)
#  include <windows.h>
#endif

#include "platform/CPUInfo.h"
#include "utils/Temperature.h"
#include "settings/AdvancedSettings.h"

#ifdef TARGET_POSIX
#include "../linux/XTimeUtils.h"
#endif

#include "gtest/gtest.h"

TEST(TestCPUInfo, GetCPUUsedPercentage)
{
  EXPECT_GE(g_cpuInfo.GetCPUUsedPercentage(), 0);
}

TEST(TestCPUInfo, GetCPUCount)
{
  EXPECT_GT(g_cpuInfo.GetCPUCount(), 0);
}

TEST(TestCPUInfo, GetCPUFrequency)
{
  EXPECT_GE(g_cpuInfo.GetCPUFrequency(), 0.f);
}

namespace
{
class TemporarySetting
{
public:

  TemporarySetting(std::string &setting, const char *newValue) :
    m_Setting(setting),
    m_OldValue(setting)
  {
    m_Setting = newValue;
  }

  ~TemporarySetting()
  {
    m_Setting = m_OldValue;
  }

private:

  std::string &m_Setting;
  std::string m_OldValue;
};
}

//Disabled for windows because there is no implementation to get the CPU temp and there will probably never be one
#ifndef TARGET_WINDOWS
TEST(TestCPUInfo, GetCPUTemperature)
{
  TemporarySetting command(g_advancedSettings.m_cpuTempCmd, "echo '50 c'");
  CTemperature t;
  EXPECT_TRUE(g_cpuInfo.GetCPUTemperature(t));
  EXPECT_TRUE(t.IsValid());
}
#endif

TEST(TestCPUInfo, GetCPUModel)
{
  std::string s = g_cpuInfo.GetCPUModel();
  EXPECT_STRNE("", s.c_str());
}

TEST(TestCPUInfo, GetCPUBogoMips)
{
  std::string s = g_cpuInfo.GetCPUBogoMips();
  EXPECT_STRNE("", s.c_str());
}

TEST(TestCPUInfo, GetCPUHardware)
{
  std::string s = g_cpuInfo.GetCPUHardware();
  EXPECT_STRNE("", s.c_str());
}

TEST(TestCPUInfo, GetCPURevision)
{
  std::string s = g_cpuInfo.GetCPURevision();
  EXPECT_STRNE("", s.c_str());
}

TEST(TestCPUInfo, GetCPUSerial)
{
  std::string s = g_cpuInfo.GetCPUSerial();
  EXPECT_STRNE("", s.c_str());
}

TEST(TestCPUInfo, CoreInfo)
{
  ASSERT_TRUE(g_cpuInfo.HasCoreId(0));
  const CoreInfo c = g_cpuInfo.GetCoreInfo(0);
  EXPECT_FALSE(c.m_strModel.empty());
}

TEST(TestCPUInfo, GetCoresUsageString)
{
  EXPECT_STRNE("", g_cpuInfo.GetCoresUsageString().c_str());
}

TEST(TestCPUInfo, GetCPUFeatures)
{
  unsigned int a = g_cpuInfo.GetCPUFeatures();
  (void)a;
}

TEST(TestCPUInfo, GetCPUUsedPercentage_output)
{
  CCPUInfo c;
  Sleep(1); //! @todo Support option from main that sets this parameter
  int r = c.GetCPUUsedPercentage();
  std::cout << "Percentage: " << testing::PrintToString(r) << std::endl;
}
