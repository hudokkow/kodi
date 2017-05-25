#pragma once
/*
 *      Copyright (C) 2017 Team Kodi
 *      http://xbmc.org
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

#include "addons/Resource.h"
#include <memory>

namespace ADDON
{
class CKeyboardResource : public CResource
{
public:
  static std::unique_ptr<CKeyboardResource> FromExtension(AddonProps props,
                                                      const cp_extension_t* ext);

  explicit CKeyboardResource(AddonProps props) : CResource(std::move(props)) {}

  //! \brief Returns whether keyboard is in use
  virtual bool IsInUse() const;

  //! \brief Check whether file is allowed or not (no filters here).
  bool IsAllowed(const std::string& file) const override { return true; }

  //! \brief Check whether add-on has keyboard
  //! \param[out] path Full path to keyboard if found.
  //! \param[in] file File name of keyboard.
  //! \return True if keyboard was found, false otherwise.
  bool HasKeyboard(std::string& path, const std::string& file) const;

  //! \brief Callback executed after installation
  virtual void OnPostInstall(bool update, bool modal);
};

}
