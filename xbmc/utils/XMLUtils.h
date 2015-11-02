#pragma once

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

#if (defined HAVE_CONFIG_H) && (!defined TARGET_WINDOWS)
  #include "config.h"
#endif
#ifdef TARGET_WINDOWS
#define TIXML_USE_STL
#pragma comment(lib, "tinyxmlSTL.lib")
#else
//compile fix for TinyXml < 2.6.0
#define DOCUMENT    TINYXML_DOCUMENT
#define ELEMENT     TINYXML_ELEMENT
#define COMMENT     TINYXML_COMMENT
#define UNKNOWN     TINYXML_UNKNOWN
#define TEXT        TINYXML_TEXT
#define DECLARATION TINYXML_DECLARATION
#define TYPECOUNT   TINYXML_TYPECOUNT
#endif

#include <string>
#include <stdint.h>
#include <vector>
#include <tinyxml.h>

#undef DOCUMENT
#undef ELEMENT
#undef COMMENT
#undef UNKNOWN
//#undef TEXT
#undef DECLARATION
#undef TYPECOUNT

class CDateTime;

class CXMLUtils : public TiXmlDocument
{
public:
  static bool GetHex(const TiXmlNode* pRootNode, const char* strTag, uint32_t& dwHexValue);
  static bool GetUInt(const TiXmlNode* pRootNode, const char* strTag, uint32_t& dwUIntValue);
  static bool GetLong(const TiXmlNode* pRootNode, const char* strTag, long& lLongValue);
  static bool GetFloat(const TiXmlNode* pRootNode, const char* strTag, float& value);
  static bool GetDouble(const TiXmlNode* pRootNode, const char* strTag, double &value);
  static bool GetInt(const TiXmlNode* pRootNode, const char* strTag, int& iIntValue);
  static bool GetBoolean(const TiXmlNode* pRootNode, const char* strTag, bool& bBoolValue);
  
  /*! \brief Get a string value from the xml tag
   If the specified tag isn't found strStringvalue is not modified and will contain whatever
   value it had before the method call.

   \param[in]     pRootNode the xml node that contains the tag
   \param[in]     strTag  the xml tag to read from
   \param[in,out] strStringValue  where to store the read string
   \return true on success, false if the tag isn't found
   */
  static bool GetString(const TiXmlNode* pRootNode, const char* strTag, std::string& strStringValue);

  /*! \brief Get multiple tags, concatenating the values together.
   Transforms
     <tag>value1</tag>
     <tag clear="true">value2</tag>
     ...
     <tag>valuen</tag>
   into value2<sep>...<sep>valuen, appending it to the value string. Note that <value1> is overwritten by the clear="true" tag.

   \param rootNode    the parent containing the <tag>'s.
   \param tag         the <tag> in question.
   \param separator   the separator to use when concatenating values.
   \param value [out] the resulting string. Remains untouched if no <tag> is available, else is appended (or cleared based on the clear parameter).
   \param clear       if true, clears the string prior to adding tags, if tags are available. Defaults to false.
   */
  static bool GetAdditiveString(const TiXmlNode* rootNode, const char* tag, const std::string& separator, std::string& value, bool clear = false);
  static bool GetStringArray(const TiXmlNode* rootNode, const char* tag, std::vector<std::string>& arrayValue, bool clear = false, const std::string& separator = "");
  static bool GetPath(const TiXmlNode* pRootNode, const char* strTag, std::string& strStringValue);
  static bool GetFloat(const TiXmlNode* pRootNode, const char* strTag, float& value, const float min, const float max);
  static bool GetUInt(const TiXmlNode* pRootNode, const char* strTag, uint32_t& dwUIntValue, const uint32_t min, const uint32_t max);
  static bool GetInt(const TiXmlNode* pRootNode, const char* strTag, int& iIntValue, const int min, const int max);
  static bool GetDate(const TiXmlNode* pRootNode, const char* strTag, CDateTime& date);
  static bool GetDateTime(const TiXmlNode* pRootNode, const char* strTag, CDateTime& dateTime);
  /*! \brief Fetch a std::string copy of an attribute, if it exists.  Cannot distinguish between empty and non-existent attributes.
   \param element the element to query.
   \param tag the name of the attribute.
   \return the attribute, if it exists, else an empty string
   */
  static std::string GetAttribute(const TiXmlElement *element, const char *tag);

  static void SetString(TiXmlNode* pRootNode, const char *strTag, const std::string& strValue);
  static void SetAdditiveString(TiXmlNode* pRootNode, const char *strTag, const std::string& strSeparator, const std::string& strValue);
  static void SetStringArray(TiXmlNode* pRootNode, const char *strTag, const std::vector<std::string>& arrayValue);
  static void SetInt(TiXmlNode* pRootNode, const char *strTag, int value);
  static void SetFloat(TiXmlNode* pRootNode, const char *strTag, float value);
  static void SetBoolean(TiXmlNode* pRootNode, const char *strTag, bool value);
  static void SetHex(TiXmlNode* pRootNode, const char *strTag, uint32_t value);
  static void SetPath(TiXmlNode* pRootNode, const char *strTag, const std::string& strValue);
  static void SetLong(TiXmlNode* pRootNode, const char *strTag, long iValue);
  static void SetDate(TiXmlNode* pRootNode, const char *strTag, const CDateTime& date);
  static void SetDateTime(TiXmlNode* pRootNode, const char *strTag, const CDateTime& dateTime);

  static const int path_version = 1;

  // moved from XBMCTinyXML
  CXMLUtils();
  CXMLUtils(const char*);
  bool LoadFile(const char*, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING);
  bool LoadFile(const std::string& _filename, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING);
  bool LoadFile(FILE*, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING);
  bool SaveFile(const char*) const;
  bool SaveFile(const std::string& filename) const;
  bool Parse(const char*, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING);
  bool Parse(const std::string& data, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING);
  bool Parse(const std::string& data, const std::string& dataCharset);
  inline std::string GetUsedCharset(void) const      { return m_UsedCharset; }
protected:
  bool TryParse(const std::string& data, const std::string& tryDataCharset);
  bool InternalParse(const std::string& rawdata, TiXmlEncoding encoding = TIXML_DEFAULT_ENCODING);

  std::string m_SuggestedCharset;
  std::string m_UsedCharset;
};

