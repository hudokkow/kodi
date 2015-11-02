/*
 *      Copyright (C) 2013 Team XBMC
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

#include <string>

#include <limits.h>

#include "MediaSettings.h"
#include "Application.h"
#include "PlayListPlayer.h"
#include "dialogs/GUIDialogContextMenu.h"
#include "dialogs/GUIDialogFileBrowser.h"
#include "interfaces/builtins/Builtins.h"
#include "music/MusicDatabase.h"
#include "messaging/ApplicationMessenger.h"
#include "messaging/helpers/DialogHelper.h"
#include "profiles/ProfilesManager.h"
#include "settings/lib/Setting.h"
#include "settings/Settings.h"
#include "storage/MediaManager.h"
#include "threads/SingleLock.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/XMLUtils.h"
#include "utils/Variant.h"
#include "video/VideoDatabase.h"
#include "cores/AudioEngine/DSPAddons/ActiveAEDSP.h"

using namespace KODI::MESSAGING;

using KODI::MESSAGING::HELPERS::DialogResponse;

CMediaSettings::CMediaSettings()
{
  m_watchedModes["files"] = WatchedModeAll;
  m_watchedModes["movies"] = WatchedModeAll;
  m_watchedModes["tvshows"] = WatchedModeAll;
  m_watchedModes["musicvideos"] = WatchedModeAll;

  m_musicPlaylistRepeat = false;
  m_musicPlaylistShuffle = false;
  m_videoPlaylistRepeat = false;
  m_videoPlaylistShuffle = false;

  m_videoStartWindowed = false;
  m_additionalSubtitleDirectoryChecked = 0;

  m_musicNeedsUpdate = 0;
  m_videoNeedsUpdate = 0;
}

CMediaSettings::~CMediaSettings()
{ }

CMediaSettings& CMediaSettings::GetInstance()
{
  static CMediaSettings sMediaSettings;
  return sMediaSettings;
}

bool CMediaSettings::Load(const TiXmlNode *settings)
{
  if (settings == NULL)
    return false;

  CSingleLock lock(m_critical);
  const TiXmlElement *pElement = settings->FirstChildElement("defaultvideosettings");
  if (pElement != NULL)
  {
    int deinterlaceMode;
    bool deinterlaceModePresent = CXMLUtils::GetInt(pElement, "deinterlacemode", deinterlaceMode, VS_DEINTERLACEMODE_OFF, VS_DEINTERLACEMODE_FORCE);
    int interlaceMethod;
    bool interlaceMethodPresent = CXMLUtils::GetInt(pElement, "interlacemethod", interlaceMethod, VS_INTERLACEMETHOD_AUTO, VS_INTERLACEMETHOD_MAX);
    // For smooth conversion of settings stored before the deinterlaceMode existed
    if (!deinterlaceModePresent && interlaceMethodPresent)
    {
      if (interlaceMethod == VS_INTERLACEMETHOD_NONE)
      {
        deinterlaceMode = VS_DEINTERLACEMODE_OFF;
        interlaceMethod = VS_INTERLACEMETHOD_AUTO;
      }
      else if (interlaceMethod == VS_INTERLACEMETHOD_AUTO)
        deinterlaceMode = VS_DEINTERLACEMODE_AUTO;
      else
        deinterlaceMode = VS_DEINTERLACEMODE_FORCE;
    }
    m_defaultVideoSettings.m_DeinterlaceMode = (EDEINTERLACEMODE)deinterlaceMode;
    m_defaultVideoSettings.m_InterlaceMethod = (EINTERLACEMETHOD)interlaceMethod;
    int scalingMethod;
    if (!CXMLUtils::GetInt(pElement, "scalingmethod", scalingMethod, VS_SCALINGMETHOD_NEAREST, VS_SCALINGMETHOD_MAX))
      scalingMethod = (int)VS_SCALINGMETHOD_LINEAR;
    m_defaultVideoSettings.m_ScalingMethod = (ESCALINGMETHOD)scalingMethod;

    CXMLUtils::GetInt(pElement, "viewmode", m_defaultVideoSettings.m_ViewMode, ViewModeNormal, ViewModeCustom);
    if (!CXMLUtils::GetFloat(pElement, "zoomamount", m_defaultVideoSettings.m_CustomZoomAmount, 0.5f, 2.0f))
      m_defaultVideoSettings.m_CustomZoomAmount = 1.0f;
    if (!CXMLUtils::GetFloat(pElement, "pixelratio", m_defaultVideoSettings.m_CustomPixelRatio, 0.5f, 2.0f))
      m_defaultVideoSettings.m_CustomPixelRatio = 1.0f;
    if (!CXMLUtils::GetFloat(pElement, "verticalshift", m_defaultVideoSettings.m_CustomVerticalShift, -2.0f, 2.0f))
      m_defaultVideoSettings.m_CustomVerticalShift = 0.0f;
    if (!CXMLUtils::GetFloat(pElement, "volumeamplification", m_defaultVideoSettings.m_VolumeAmplification, VOLUME_DRC_MINIMUM * 0.01f, VOLUME_DRC_MAXIMUM * 0.01f))
      m_defaultVideoSettings.m_VolumeAmplification = VOLUME_DRC_MINIMUM * 0.01f;
    if (!CXMLUtils::GetFloat(pElement, "noisereduction", m_defaultVideoSettings.m_NoiseReduction, 0.0f, 1.0f))
      m_defaultVideoSettings.m_NoiseReduction = 0.0f;
    CXMLUtils::GetBoolean(pElement, "postprocess", m_defaultVideoSettings.m_PostProcess);
    if (!CXMLUtils::GetFloat(pElement, "sharpness", m_defaultVideoSettings.m_Sharpness, -1.0f, 1.0f))
      m_defaultVideoSettings.m_Sharpness = 0.0f;
    CXMLUtils::GetBoolean(pElement, "outputtoallspeakers", m_defaultVideoSettings.m_OutputToAllSpeakers);
    CXMLUtils::GetBoolean(pElement, "showsubtitles", m_defaultVideoSettings.m_SubtitleOn);
    if (!CXMLUtils::GetFloat(pElement, "brightness", m_defaultVideoSettings.m_Brightness, 0, 100))
      m_defaultVideoSettings.m_Brightness = 50;
    if (!CXMLUtils::GetFloat(pElement, "contrast", m_defaultVideoSettings.m_Contrast, 0, 100))
      m_defaultVideoSettings.m_Contrast = 50;
    if (!CXMLUtils::GetFloat(pElement, "gamma", m_defaultVideoSettings.m_Gamma, 0, 100))
      m_defaultVideoSettings.m_Gamma = 20;
    if (!CXMLUtils::GetFloat(pElement, "audiodelay", m_defaultVideoSettings.m_AudioDelay, -10.0f, 10.0f))
      m_defaultVideoSettings.m_AudioDelay = 0.0f;
    if (!CXMLUtils::GetFloat(pElement, "subtitledelay", m_defaultVideoSettings.m_SubtitleDelay, -10.0f, 10.0f))
      m_defaultVideoSettings.m_SubtitleDelay = 0.0f;
    CXMLUtils::GetBoolean(pElement, "nonlinstretch", m_defaultVideoSettings.m_CustomNonLinStretch);
    if (!CXMLUtils::GetInt(pElement, "stereomode", m_defaultVideoSettings.m_StereoMode))
      m_defaultVideoSettings.m_StereoMode = 0;

    m_defaultVideoSettings.m_SubtitleCached = false;
  }

  pElement = settings->FirstChildElement("defaultaudiosettings");
  if (pElement != NULL)
  {
    if (!CXMLUtils::GetInt(pElement, "masterstreamtype", m_defaultAudioSettings.m_MasterStreamType))
      m_defaultAudioSettings.m_MasterStreamType = AE_DSP_ASTREAM_AUTO;
    if (!CXMLUtils::GetInt(pElement, "masterstreamtypesel", m_defaultAudioSettings.m_MasterStreamTypeSel))
      m_defaultAudioSettings.m_MasterStreamTypeSel = AE_DSP_ASTREAM_AUTO;
    if (!CXMLUtils::GetInt(pElement, "masterstreambase", m_defaultAudioSettings.m_MasterStreamBase))
      m_defaultAudioSettings.m_MasterStreamBase = AE_DSP_ABASE_STEREO;

    std::string strTag;
    for (int type = AE_DSP_ASTREAM_BASIC; type < AE_DSP_ASTREAM_MAX; type++)
    {
      for (int base = AE_DSP_ABASE_STEREO; base < AE_DSP_ABASE_MAX; base++)
      {
        int tmp;
        strTag = StringUtils::Format("masterprocess_%i_%i", type, base);
        if (CXMLUtils::GetInt(pElement, strTag.c_str(), tmp))
          m_defaultAudioSettings.m_MasterModes[type][base] = tmp;
        else
          m_defaultAudioSettings.m_MasterModes[type][base] = 0;
      }
    }
  }

  // mymusic settings
  pElement = settings->FirstChildElement("mymusic");
  if (pElement != NULL)
  {
    const TiXmlElement *pChild = pElement->FirstChildElement("playlist");
    if (pChild != NULL)
    {
      CXMLUtils::GetBoolean(pChild, "repeat", m_musicPlaylistRepeat);
      CXMLUtils::GetBoolean(pChild, "shuffle", m_musicPlaylistShuffle);
    }
    if (!CXMLUtils::GetInt(pElement, "needsupdate", m_musicNeedsUpdate, 0, INT_MAX))
      m_musicNeedsUpdate = 0;
  }
  
  // Read the watchmode settings for the various media views
  pElement = settings->FirstChildElement("myvideos");
  if (pElement != NULL)
  {
    int tmp;
    if (CXMLUtils::GetInt(pElement, "watchmodemovies", tmp, (int)WatchedModeAll, (int)WatchedModeWatched))
      m_watchedModes["movies"] = (WatchedMode)tmp;
    if (CXMLUtils::GetInt(pElement, "watchmodetvshows", tmp, (int)WatchedModeAll, (int)WatchedModeWatched))
      m_watchedModes["tvshows"] = (WatchedMode)tmp;
    if (CXMLUtils::GetInt(pElement, "watchmodemusicvideos", tmp, (int)WatchedModeAll, (int)WatchedModeWatched))
      m_watchedModes["musicvideos"] = (WatchedMode)tmp;

    const TiXmlElement *pChild = pElement->FirstChildElement("playlist");
    if (pChild != NULL)
    {
      CXMLUtils::GetBoolean(pChild, "repeat", m_videoPlaylistRepeat);
      CXMLUtils::GetBoolean(pChild, "shuffle", m_videoPlaylistShuffle);
    }
    if (!CXMLUtils::GetInt(pElement, "needsupdate", m_videoNeedsUpdate, 0, INT_MAX))
      m_videoNeedsUpdate = 0;
  }

  return true;
}

void CMediaSettings::OnSettingsLoaded()
{
  g_playlistPlayer.SetRepeat(PLAYLIST_MUSIC, m_musicPlaylistRepeat ? PLAYLIST::REPEAT_ALL : PLAYLIST::REPEAT_NONE);
  g_playlistPlayer.SetShuffle(PLAYLIST_MUSIC, m_musicPlaylistShuffle);
  g_playlistPlayer.SetRepeat(PLAYLIST_VIDEO, m_videoPlaylistRepeat ? PLAYLIST::REPEAT_ALL : PLAYLIST::REPEAT_NONE);
  g_playlistPlayer.SetShuffle(PLAYLIST_VIDEO, m_videoPlaylistShuffle);
}

bool CMediaSettings::Save(TiXmlNode *settings) const
{
  if (settings == NULL)
    return false;

  CSingleLock lock(m_critical);
  // default video settings
  TiXmlElement videoSettingsNode("defaultvideosettings");
  TiXmlNode *pNode = settings->InsertEndChild(videoSettingsNode);
  if (pNode == NULL)
    return false;

  CXMLUtils::SetInt(pNode, "deinterlacemode", m_defaultVideoSettings.m_DeinterlaceMode);
  CXMLUtils::SetInt(pNode, "interlacemethod", m_defaultVideoSettings.m_InterlaceMethod);
  CXMLUtils::SetInt(pNode, "scalingmethod", m_defaultVideoSettings.m_ScalingMethod);
  CXMLUtils::SetFloat(pNode, "noisereduction", m_defaultVideoSettings.m_NoiseReduction);
  CXMLUtils::SetBoolean(pNode, "postprocess", m_defaultVideoSettings.m_PostProcess);
  CXMLUtils::SetFloat(pNode, "sharpness", m_defaultVideoSettings.m_Sharpness);
  CXMLUtils::SetInt(pNode, "viewmode", m_defaultVideoSettings.m_ViewMode);
  CXMLUtils::SetFloat(pNode, "zoomamount", m_defaultVideoSettings.m_CustomZoomAmount);
  CXMLUtils::SetFloat(pNode, "pixelratio", m_defaultVideoSettings.m_CustomPixelRatio);
  CXMLUtils::SetFloat(pNode, "verticalshift", m_defaultVideoSettings.m_CustomVerticalShift);
  CXMLUtils::SetFloat(pNode, "volumeamplification", m_defaultVideoSettings.m_VolumeAmplification);
  CXMLUtils::SetBoolean(pNode, "outputtoallspeakers", m_defaultVideoSettings.m_OutputToAllSpeakers);
  CXMLUtils::SetBoolean(pNode, "showsubtitles", m_defaultVideoSettings.m_SubtitleOn);
  CXMLUtils::SetFloat(pNode, "brightness", m_defaultVideoSettings.m_Brightness);
  CXMLUtils::SetFloat(pNode, "contrast", m_defaultVideoSettings.m_Contrast);
  CXMLUtils::SetFloat(pNode, "gamma", m_defaultVideoSettings.m_Gamma);
  CXMLUtils::SetFloat(pNode, "audiodelay", m_defaultVideoSettings.m_AudioDelay);
  CXMLUtils::SetFloat(pNode, "subtitledelay", m_defaultVideoSettings.m_SubtitleDelay);
  CXMLUtils::SetBoolean(pNode, "nonlinstretch", m_defaultVideoSettings.m_CustomNonLinStretch);
  CXMLUtils::SetInt(pNode, "stereomode", m_defaultVideoSettings.m_StereoMode);

  // default audio settings for dsp addons
  TiXmlElement audioSettingsNode("defaultaudiosettings");
  pNode = settings->InsertEndChild(audioSettingsNode);
  if (pNode == NULL)
    return false;

  CXMLUtils::SetInt(pNode, "masterstreamtype", m_defaultAudioSettings.m_MasterStreamType);
  CXMLUtils::SetInt(pNode, "masterstreamtypesel", m_defaultAudioSettings.m_MasterStreamTypeSel);
  CXMLUtils::SetInt(pNode, "masterstreambase", m_defaultAudioSettings.m_MasterStreamBase);

  std::string strTag;
  for (int type = AE_DSP_ASTREAM_BASIC; type < AE_DSP_ASTREAM_MAX; type++)
  {
    for (int base = AE_DSP_ABASE_STEREO; base < AE_DSP_ABASE_MAX; base++)
    {
      strTag = StringUtils::Format("masterprocess_%i_%i", type, base);
      CXMLUtils::SetInt(pNode, strTag.c_str(), m_defaultAudioSettings.m_MasterModes[type][base]);
    }
  }

  // mymusic
  pNode = settings->FirstChild("mymusic");
  if (pNode == NULL)
  {
    TiXmlElement videosNode("mymusic");
    pNode = settings->InsertEndChild(videosNode);
    if (pNode == NULL)
      return false;
  }

  TiXmlElement musicPlaylistNode("playlist");
  TiXmlNode *playlistNode = pNode->InsertEndChild(musicPlaylistNode);
  if (playlistNode == NULL)
    return false;
  CXMLUtils::SetBoolean(playlistNode, "repeat", m_musicPlaylistRepeat);
  CXMLUtils::SetBoolean(playlistNode, "shuffle", m_musicPlaylistShuffle);

  CXMLUtils::SetInt(pNode, "needsupdate", m_musicNeedsUpdate);

  // myvideos
  pNode = settings->FirstChild("myvideos");
  if (pNode == NULL)
  {
    TiXmlElement videosNode("myvideos");
    pNode = settings->InsertEndChild(videosNode);
    if (pNode == NULL)
      return false;
  }

  CXMLUtils::SetInt(pNode, "watchmodemovies", m_watchedModes.find("movies")->second);
  CXMLUtils::SetInt(pNode, "watchmodetvshows", m_watchedModes.find("tvshows")->second);
  CXMLUtils::SetInt(pNode, "watchmodemusicvideos", m_watchedModes.find("musicvideos")->second);

  TiXmlElement videoPlaylistNode("playlist");
  playlistNode = pNode->InsertEndChild(videoPlaylistNode);
  if (playlistNode == NULL)
    return false;
  CXMLUtils::SetBoolean(playlistNode, "repeat", m_videoPlaylistRepeat);
  CXMLUtils::SetBoolean(playlistNode, "shuffle", m_videoPlaylistShuffle);

  CXMLUtils::SetInt(pNode, "needsupdate", m_videoNeedsUpdate);

  return true;
}

void CMediaSettings::OnSettingAction(const CSetting *setting)
{
  if (setting == NULL)
    return;

  const std::string &settingId = setting->GetId();
  if (settingId == CSettings::SETTING_MUSICLIBRARY_CLEANUP)
  {
    if (HELPERS::ShowYesNoDialogText(CVariant{313}, CVariant{333}) == DialogResponse::YES)
      g_application.StartMusicCleanup(true);
  }
  else if (settingId == CSettings::SETTING_MUSICLIBRARY_EXPORT)
    CBuiltins::GetInstance().Execute("exportlibrary(music)");
  else if (settingId == CSettings::SETTING_MUSICLIBRARY_IMPORT)
  {
    std::string path;
    VECSOURCES shares;
    g_mediaManager.GetLocalDrives(shares);
    g_mediaManager.GetNetworkLocations(shares);
    g_mediaManager.GetRemovableDrives(shares);

    if (CGUIDialogFileBrowser::ShowAndGetFile(shares, "musicdb.xml", g_localizeStrings.Get(651) , path))
    {
      CMusicDatabase musicdatabase;
      musicdatabase.Open();
      musicdatabase.ImportFromXML(path);
      musicdatabase.Close();
    }
  }
  else if (settingId == CSettings::SETTING_VIDEOLIBRARY_CLEANUP)
  {
    if (HELPERS::ShowYesNoDialogText(CVariant{313}, CVariant{333}) == DialogResponse::YES)
      g_application.StartVideoCleanup(true);
  }
  else if (settingId == CSettings::SETTING_VIDEOLIBRARY_EXPORT)
    CBuiltins::GetInstance().Execute("exportlibrary(video)");
  else if (settingId == CSettings::SETTING_VIDEOLIBRARY_IMPORT)
  {
    std::string path;
    VECSOURCES shares;
    g_mediaManager.GetLocalDrives(shares);
    g_mediaManager.GetNetworkLocations(shares);
    g_mediaManager.GetRemovableDrives(shares);

    if (CGUIDialogFileBrowser::ShowAndGetDirectory(shares, g_localizeStrings.Get(651) , path))
    {
      CVideoDatabase videodatabase;
      videodatabase.Open();
      videodatabase.ImportFromXML(path);
      videodatabase.Close();
    }
  }
}

int CMediaSettings::GetWatchedMode(const std::string &content) const
{
  CSingleLock lock(m_critical);
  WatchedModes::const_iterator it = m_watchedModes.find(GetWatchedContent(content));
  if (it != m_watchedModes.end())
    return it->second;

  return WatchedModeAll;
}

void CMediaSettings::SetWatchedMode(const std::string &content, WatchedMode mode)
{
  CSingleLock lock(m_critical);
  WatchedModes::iterator it = m_watchedModes.find(GetWatchedContent(content));
  if (it != m_watchedModes.end())
    it->second = mode;
}

void CMediaSettings::CycleWatchedMode(const std::string &content)
{
  CSingleLock lock(m_critical);
  WatchedModes::iterator it = m_watchedModes.find(GetWatchedContent(content));
  if (it != m_watchedModes.end())
  {
    it->second = (WatchedMode)((int)it->second + 1);
    if (it->second > WatchedModeWatched)
      it->second = WatchedModeAll;
  }
}

std::string CMediaSettings::GetWatchedContent(const std::string &content)
{
  if (content == "seasons" || content == "episodes")
    return "tvshows";

  return content;
}
