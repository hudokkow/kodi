/*
*      Copyright (C) 2015 Team Kodi
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
#include "KeyboardResource.h"
#include "URL.h"
#include "addons/AddonManager.h"
#include "input/KeyboardLayoutManager.h"
#include "filesystem/File.h"
#include "settings/Settings.h"
#include "utils/StringUtils.h"

namespace ADDON
{

CKeyboardResource::CKeyboardResource(const CKeyboardResource &rhs)
  : CResource(rhs)
{ }

AddonPtr CKeyboardResource::Clone() const
{
  return AddonPtr(new CKeyboardResource(*this));
}

bool CKeyboardResource::IsAllowed(const std::string &file) const
{
  return file.empty() || StringUtils::EqualsNoCase(file, "keyboard.xml");
}

bool CKeyboardResource::IsInUse() const
{
  return CSettings::GetInstance().GetString(CSettings::SETTING_LOCALE_KEYBOARDLAYOUTS) == ID();
}

void CKeyboardResource::OnPostInstall(bool update, bool modal)
{
  if (IsInUse())
    CKeyboardLayoutManager::GetInstance().Load();
}

} /* namespace ADDON */
