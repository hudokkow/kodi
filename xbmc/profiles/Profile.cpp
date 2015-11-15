/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "Profile.h"
#include "GUIInfoManager.h"
#include "utils/XMLUtils.h"

CProfile::CLock::CLock(LockType type, const std::string &password):
  code(password)
{
  programs = false;
  pictures = false;
  files = false;
  video = false;
  music = false;
  settings = LOCK_LEVEL::NONE;
  addonManager = false;
  mode = type;
}

void CProfile::CLock::Validate()
{
  if (mode != LOCK_MODE_EVERYONE && (code == "-" || code.empty()))
    mode = LOCK_MODE_EVERYONE;
  
  if (code.empty() || mode == LOCK_MODE_EVERYONE)
    code = "-";
}

CProfile::CProfile(const std::string &directory, const std::string &name, const int id):
  m_directory(directory),
  m_name(name)
{
  m_id = id;
  m_bDatabases = true;
  m_bCanWrite = true;
  m_bSources = true;
  m_bCanWriteSources = true;
  m_bAddons = true;
}

CProfile::~CProfile(void)
{}

void CProfile::setDate()
{
  std::string strDate = g_infoManager.GetDate(true);
  std::string strTime = g_infoManager.GetTime();
  if (strDate.empty() || strTime.empty())
    setDate("-");
  else
    setDate(strDate+" - "+strTime);
}

void CProfile::Load(const TiXmlNode *node, int nextIdProfile)
{
  if (!CXMLUtils::GetInt(node, "id", m_id))
    m_id = nextIdProfile; 

  CXMLUtils::GetString(node, "name", m_name);
  CXMLUtils::GetPath(node, "directory", m_directory);
  CXMLUtils::GetPath(node, "thumbnail", m_thumb);
  CXMLUtils::GetBoolean(node, "hasdatabases", m_bDatabases);
  CXMLUtils::GetBoolean(node, "canwritedatabases", m_bCanWrite);
  CXMLUtils::GetBoolean(node, "hassources", m_bSources);
  CXMLUtils::GetBoolean(node, "canwritesources", m_bCanWriteSources);
  CXMLUtils::GetBoolean(node, "lockaddonmanager", m_locks.addonManager);
  int settings = m_locks.settings;
  CXMLUtils::GetInt(node, "locksettings", settings);
  m_locks.settings = (LOCK_LEVEL::SETTINGS_LOCK)settings;
  CXMLUtils::GetBoolean(node, "lockfiles", m_locks.files);
  CXMLUtils::GetBoolean(node, "lockmusic", m_locks.music);
  CXMLUtils::GetBoolean(node, "lockvideo", m_locks.video);
  CXMLUtils::GetBoolean(node, "lockpictures", m_locks.pictures);
  CXMLUtils::GetBoolean(node, "lockprograms", m_locks.programs);
  
  int lockMode = m_locks.mode;
  CXMLUtils::GetInt(node, "lockmode", lockMode);
  m_locks.mode = (LockType)lockMode;
  if (m_locks.mode > LOCK_MODE_QWERTY || m_locks.mode < LOCK_MODE_EVERYONE)
    m_locks.mode = LOCK_MODE_EVERYONE;
  
  CXMLUtils::GetString(node, "lockcode", m_locks.code);
  CXMLUtils::GetString(node, "lastdate", m_date);
}

void CProfile::Save(TiXmlNode *root) const
{
  TiXmlElement profileNode("profile");
  TiXmlNode *node = root->InsertEndChild(profileNode);

  CXMLUtils::SetInt(node, "id", m_id);
  CXMLUtils::SetString(node, "name", m_name);
  CXMLUtils::SetPath(node, "directory", m_directory);
  CXMLUtils::SetPath(node, "thumbnail", m_thumb);
  CXMLUtils::SetBoolean(node, "hasdatabases", m_bDatabases);
  CXMLUtils::SetBoolean(node, "canwritedatabases", m_bCanWrite);
  CXMLUtils::SetBoolean(node, "hassources", m_bSources);
  CXMLUtils::SetBoolean(node, "canwritesources", m_bCanWriteSources);
  CXMLUtils::SetBoolean(node, "lockaddonmanager", m_locks.addonManager);
  CXMLUtils::SetInt(node, "locksettings", m_locks.settings);
  CXMLUtils::SetBoolean(node, "lockfiles", m_locks.files);
  CXMLUtils::SetBoolean(node, "lockmusic", m_locks.music);
  CXMLUtils::SetBoolean(node, "lockvideo", m_locks.video);
  CXMLUtils::SetBoolean(node, "lockpictures", m_locks.pictures);
  CXMLUtils::SetBoolean(node, "lockprograms", m_locks.programs);

  CXMLUtils::SetInt(node, "lockmode", m_locks.mode);
  CXMLUtils::SetString(node,"lockcode", m_locks.code);
  CXMLUtils::SetString(node, "lastdate", m_date);
}

void CProfile::SetLocks(const CProfile::CLock &locks)
{
  m_locks = locks;
  m_locks.Validate();
}
