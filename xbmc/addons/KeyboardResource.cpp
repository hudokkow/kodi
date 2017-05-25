/*
*      Copyright (C) 2017 Team Kodi
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
*  along with Kodi; see the file COPYING.  If not, see
*  <http://www.gnu.org/licenses/>.
*
*/
#include "KeyboardResource.h"
#include "filesystem/File.h"
#include "filesystem/SpecialProtocol.h"

using namespace XFILE;

namespace ADDON
{

std::unique_ptr<CKeyboardResource> CKeyboardResource::FromExtension(AddonProps props, const cp_extension_t* ext)
{
  return std::unique_ptr<CKeyboardResource>(new CKeyboardResource(std::move(props)));
}

bool CKeyboardResource::IsInUse() const
{
  return false;
}

void CKeyboardResource::OnPostInstall(bool update, bool modal)
{
}

bool CKeyboardResource::HasKeyboard(std::string& path, const std::string& file) const
{
#ifdef TARGET_POSIX
  std::string result = CSpecialProtocol::TranslatePathConvertCase(Path() + "/resources/" + file);
#else
  std::string result = Path() + "/resources/" + file;
#endif
  if (CFile::Exists(result))
  {
    path = result;
    return true;
  }

  return false;
}

}
