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

#include "Artist.h"
#include "utils/XMLUtils.h"
#include "settings/AdvancedSettings.h"

#include <algorithm>

void CArtist::MergeScrapedArtist(const CArtist& source, bool override /* = true */)
{
  /*
   We don't merge musicbrainz artist ID so that a refresh of artist information
   allows a lookup based on name rather than directly (re)using musicbrainz.
   In future, we may wish to be able to override lookup by musicbrainz so
   this might be dropped.
   */
  //  strMusicBrainzArtistID = source.strMusicBrainzArtistID;
  if ((override && !source.strArtist.empty()) || strArtist.empty())
    strArtist = source.strArtist;

  genre = source.genre;
  strBiography = source.strBiography;
  styles = source.styles;
  moods = source.moods;
  instruments = source.instruments;
  strBorn = source.strBorn;
  strFormed = source.strFormed;
  strDied = source.strDied;
  strDisbanded = source.strDisbanded;
  yearsActive = source.yearsActive;
  thumbURL = source.thumbURL;
  fanart = source.fanart;
  discography = source.discography;
}


bool CArtist::Load(const TiXmlElement *artist, bool append, bool prioritise)
{
  if (!artist) return false;
  if (!append)
    Reset();

  CXMLUtils::GetString(artist,                "name", strArtist);
  CXMLUtils::GetString(artist, "musicBrainzArtistID", strMusicBrainzArtistID);

  CXMLUtils::GetStringArray(artist,       "genre", genre, prioritise, g_advancedSettings.m_musicItemSeparator);
  CXMLUtils::GetStringArray(artist,       "style", styles, prioritise, g_advancedSettings.m_musicItemSeparator);
  CXMLUtils::GetStringArray(artist,        "mood", moods, prioritise, g_advancedSettings.m_musicItemSeparator);
  CXMLUtils::GetStringArray(artist, "yearsactive", yearsActive, prioritise, g_advancedSettings.m_musicItemSeparator);
  CXMLUtils::GetStringArray(artist, "instruments", instruments, prioritise, g_advancedSettings.m_musicItemSeparator);

  CXMLUtils::GetString(artist,      "born", strBorn);
  CXMLUtils::GetString(artist,    "formed", strFormed);
  CXMLUtils::GetString(artist, "biography", strBiography);
  CXMLUtils::GetString(artist,      "died", strDied);
  CXMLUtils::GetString(artist, "disbanded", strDisbanded);

  size_t iThumbCount = thumbURL.m_url.size();
  std::string xmlAdd = thumbURL.m_xml;

  const TiXmlElement* thumb = artist->FirstChildElement("thumb");
  while (thumb)
  {
    thumbURL.ParseElement(thumb);
    if (prioritise)
    {
      std::string temp;
      temp << *thumb;
      xmlAdd = temp+xmlAdd;
    }
    thumb = thumb->NextSiblingElement("thumb");
  }
  // prefix thumbs from nfos
  if (prioritise && iThumbCount && iThumbCount != thumbURL.m_url.size())
  {
    rotate(thumbURL.m_url.begin(),
           thumbURL.m_url.begin()+iThumbCount, 
           thumbURL.m_url.end());
    thumbURL.m_xml = xmlAdd;
  }
  const TiXmlElement* node = artist->FirstChildElement("album");
  while (node)
  {
    const TiXmlNode* title = node->FirstChild("title");
    if (title && title->FirstChild())
    {
      std::string strTitle = title->FirstChild()->Value();
      std::string strYear;
      const TiXmlNode* year = node->FirstChild("year");
      if (year && year->FirstChild())
        strYear = year->FirstChild()->Value();
      discography.push_back(make_pair(strTitle,strYear));
    }
    node = node->NextSiblingElement("album");
  }

  // fanart
  const TiXmlElement *fanart2 = artist->FirstChildElement("fanart");
  if (fanart2)
  {
    // we prefix to handle mixed-mode nfo's with fanart set
    if (prioritise)
    {
      std::string temp;
      temp << *fanart2;
      fanart.m_xml = temp+fanart.m_xml;
    }
    else
      fanart.m_xml << *fanart2;
    fanart.Unpack();
  }

  return true;
}

bool CArtist::Save(TiXmlNode *node, const std::string &tag, const std::string& strPath)
{
  if (!node) return false;

  // we start with a <tag> tag
  TiXmlElement artistElement(tag.c_str());
  TiXmlNode *artist = node->InsertEndChild(artistElement);

  if (!artist) return false;

  CXMLUtils::SetString(artist,                      "name", strArtist);
  CXMLUtils::SetString(artist,       "musicBrainzArtistID", strMusicBrainzArtistID);
  CXMLUtils::SetStringArray(artist,                "genre", genre);
  CXMLUtils::SetStringArray(artist,                "style", styles);
  CXMLUtils::SetStringArray(artist,                 "mood", moods);
  CXMLUtils::SetStringArray(artist,          "yearsactive", yearsActive);
  CXMLUtils::SetStringArray(artist,          "instruments", instruments);
  CXMLUtils::SetString(artist,                      "born", strBorn);
  CXMLUtils::SetString(artist,                    "formed", strFormed);
  CXMLUtils::SetString(artist,                 "biography", strBiography);
  CXMLUtils::SetString(artist,                      "died", strDied);
  CXMLUtils::SetString(artist,                 "disbanded", strDisbanded);
  if (!thumbURL.m_xml.empty())
  {
    CXMLUtils doc;
    doc.Parse(thumbURL.m_xml);
    const TiXmlNode* thumb = doc.FirstChild("thumb");
    while (thumb)
    {
      artist->InsertEndChild(*thumb);
      thumb = thumb->NextSibling("thumb");
    }
  }
  CXMLUtils::SetString(artist,        "path", strPath);
  if (fanart.m_xml.size())
  {
    CXMLUtils doc;
    doc.Parse(fanart.m_xml);
    artist->InsertEndChild(*doc.RootElement());
  }

  // albums
  for (std::vector<std::pair<std::string,std::string> >::const_iterator it = discography.begin(); it != discography.end(); ++it)
  {
    // add a <album> tag
    TiXmlElement cast("album");
    TiXmlNode *node = artist->InsertEndChild(cast);
    TiXmlElement title("title");
    TiXmlNode *titleNode = node->InsertEndChild(title);
    TiXmlText name(it->first);
    titleNode->InsertEndChild(name);
    TiXmlElement year("year");
    TiXmlNode *yearNode = node->InsertEndChild(year);
    TiXmlText name2(it->second);
    yearNode->InsertEndChild(name2);
  }

  return true;
}

void CArtist::SetDateAdded(const std::string& strDateAdded)
{
  dateAdded.SetFromDBDateTime(strDateAdded);
}

