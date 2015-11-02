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

#include <string>
#include <vector>
#include <cstdlib>

#include "CharsetConverter.h"
#include "CharsetDetection.h"
#include "filesystem/File.h"
#include "LangInfo.h"
#include "log.h"
#include "RegExp.h"
#include "StringUtils.h"
#include "URL.h"
#include "Utf8Utils.h"
#include "XMLUtils.h"

#ifdef TARGET_WINDOWS
#include "PlatformDefs.h" //for strcasecmp
#endif

#define MAX_ENTITY_LENGTH 8 // size of largest entity "&#xNNNN;"
#define BUFFER_SIZE 4096

bool CXMLUtils::GetHex(const TiXmlNode* pRootNode, const char* strTag, uint32_t& hexValue)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
  if (!pNode || !pNode->FirstChild()) return false;
  return sscanf(pNode->FirstChild()->Value(), "%x", (uint32_t*)&hexValue) == 1;
}


bool CXMLUtils::GetUInt(const TiXmlNode* pRootNode, const char* strTag, uint32_t& uintValue)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
  if (!pNode || !pNode->FirstChild()) return false;
  uintValue = atol(pNode->FirstChild()->Value());
  return true;
}

bool CXMLUtils::GetUInt(const TiXmlNode* pRootNode, const char* strTag, uint32_t &value, const uint32_t min, const uint32_t max)
{
  if (GetUInt(pRootNode, strTag, value))
  {
    if (value < min) value = min;
    if (value > max) value = max;
    return true;
  }
  return false;
}

bool CXMLUtils::GetLong(const TiXmlNode* pRootNode, const char* strTag, long& lLongValue)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
  if (!pNode || !pNode->FirstChild()) return false;
  lLongValue = atol(pNode->FirstChild()->Value());
  return true;
}

bool CXMLUtils::GetInt(const TiXmlNode* pRootNode, const char* strTag, int& iIntValue)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
  if (!pNode || !pNode->FirstChild()) return false;
  iIntValue = atoi(pNode->FirstChild()->Value());
  return true;
}

bool CXMLUtils::GetInt(const TiXmlNode* pRootNode, const char* strTag, int &value, const int min, const int max)
{
  if (GetInt(pRootNode, strTag, value))
  {
    if (value < min) value = min;
    if (value > max) value = max;
    return true;
  }
  return false;
}

bool CXMLUtils::GetDouble(const TiXmlNode *root, const char *tag, double &value)
{
  const TiXmlNode* node = root->FirstChild(tag);
  if (!node || !node->FirstChild()) return false;
  value = atof(node->FirstChild()->Value());
  return true;
}

bool CXMLUtils::GetFloat(const TiXmlNode* pRootNode, const char* strTag, float& value)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
  if (!pNode || !pNode->FirstChild()) return false;
  value = (float)atof(pNode->FirstChild()->Value());
  return true;
}

bool CXMLUtils::GetFloat(const TiXmlNode* pRootElement, const char *tagName, float& fValue, const float fMin, const float fMax)
{
  if (GetFloat(pRootElement, tagName, fValue))
  { // check range
    if (fValue < fMin) fValue = fMin;
    if (fValue > fMax) fValue = fMax;
    return true;
  }
  return false;
}

bool CXMLUtils::GetBoolean(const TiXmlNode* pRootNode, const char* strTag, bool& bBoolValue)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
  if (!pNode || !pNode->FirstChild()) return false;
  std::string strEnabled = pNode->FirstChild()->ValueStr();
  StringUtils::ToLower(strEnabled);
  if (strEnabled == "off" || strEnabled == "no" || strEnabled == "disabled" || strEnabled == "false" || strEnabled == "0" )
    bBoolValue = false;
  else
  {
    bBoolValue = true;
    if (strEnabled != "on" && strEnabled != "yes" && strEnabled != "enabled" && strEnabled != "true")
      return false; // invalid bool switch - it's probably some other string.
  }
  return true;
}

bool CXMLUtils::GetString(const TiXmlNode* pRootNode, const char* strTag, std::string& strStringValue)
{
  const TiXmlElement* pElement = pRootNode->FirstChildElement(strTag);
  if (!pElement) return false;

  const char* encoded = pElement->Attribute("urlencoded");
  const TiXmlNode* pNode = pElement->FirstChild();
  if (pNode != NULL)
  {
    strStringValue = pNode->ValueStr();
    if (encoded && strcasecmp(encoded,"yes") == 0)
      strStringValue = CURL::Decode(strStringValue);
    return true;
  }
  strStringValue.clear();
  return true;
}

bool CXMLUtils::GetAdditiveString(const TiXmlNode* pRootNode, const char* strTag,
                                 const std::string& strSeparator, std::string& strStringValue,
                                 bool clear)
{
  std::string strTemp;
  const TiXmlElement* node = pRootNode->FirstChildElement(strTag);
  bool bResult=false;
  if (node && node->FirstChild() && clear)
    strStringValue.clear();
  while (node)
  {
    if (node->FirstChild())
    {
      bResult = true;
      strTemp = node->FirstChild()->Value();
      const char* clear=node->Attribute("clear");
      if (strStringValue.empty() || (clear && strcasecmp(clear,"true")==0))
        strStringValue = strTemp;
      else
        strStringValue += strSeparator+strTemp;
    }
    node = node->NextSiblingElement(strTag);
  }

  return bResult;
}

/*!
  Parses the XML for multiple tags of the given name.
  Does not clear the array to support chaining.
*/
bool CXMLUtils::GetStringArray(const TiXmlNode* pRootNode, const char* strTag, std::vector<std::string>& arrayValue, bool clear /* = false */, const std::string& separator /* = "" */)
{
  std::string strTemp;
  const TiXmlElement* node = pRootNode->FirstChildElement(strTag);
  bool bResult=false;
  if (node && node->FirstChild() && clear)
    arrayValue.clear();
  while (node)
  {
    if (node->FirstChild())
    {
      bResult = true;
      strTemp = node->FirstChild()->ValueStr();

      const char* clearAttr = node->Attribute("clear");
      if (clearAttr && strcasecmp(clearAttr, "true") == 0)
        arrayValue.clear();

      if (strTemp.empty())
        continue;

      if (separator.empty())
        arrayValue.push_back(strTemp);
      else
      {
        std::vector<std::string> tempArray = StringUtils::Split(strTemp, separator);
        arrayValue.insert(arrayValue.end(), tempArray.begin(), tempArray.end());
      }
    }
    node = node->NextSiblingElement(strTag);
  }

  return bResult;
}

bool CXMLUtils::GetPath(const TiXmlNode* pRootNode, const char* strTag, std::string& strStringValue)
{
  const TiXmlElement* pElement = pRootNode->FirstChildElement(strTag);
  if (!pElement) return false;

  const char* encoded = pElement->Attribute("urlencoded");
  const TiXmlNode* pNode = pElement->FirstChild();
  if (pNode != NULL)
  {
    strStringValue = pNode->Value();
    if (encoded && strcasecmp(encoded,"yes") == 0)
      strStringValue = CURL::Decode(strStringValue);
    return true;
  }
  strStringValue.clear();
  return false;
}

bool CXMLUtils::GetDate(const TiXmlNode* pRootNode, const char* strTag, CDateTime& date)
{
  std::string strDate;
  if (GetString(pRootNode, strTag, strDate) && !strDate.empty())
  {
    date.SetFromDBDate(strDate);
    return true;
  }

  return false;
}

bool CXMLUtils::GetDateTime(const TiXmlNode* pRootNode, const char* strTag, CDateTime& dateTime)
{
  std::string strDateTime;
  if (GetString(pRootNode, strTag, strDateTime) && !strDateTime.empty())
  {
    dateTime.SetFromDBDateTime(strDateTime);
    return true;
  }

  return false;
}

std::string CXMLUtils::GetAttribute(const TiXmlElement *element, const char *tag)
{
  if (element)
  {
    const char *attribute = element->Attribute(tag);
    if (attribute)
      return attribute;
  }
  return "";
}

void CXMLUtils::SetAdditiveString(TiXmlNode* pRootNode, const char *strTag, const std::string& strSeparator, const std::string& strValue)
{
  std::vector<std::string> list = StringUtils::Split(strValue, strSeparator);
  for (std::vector<std::string>::const_iterator i = list.begin(); i != list.end(); ++i)
    SetString(pRootNode, strTag, *i);
}

void CXMLUtils::SetStringArray(TiXmlNode* pRootNode, const char *strTag, const std::vector<std::string>& arrayValue)
{
  for (unsigned int i = 0; i < arrayValue.size(); i++)
    SetString(pRootNode, strTag, arrayValue.at(i));
}

void CXMLUtils::SetString(TiXmlNode* pRootNode, const char *strTag, const std::string& strValue)
{
  TiXmlElement newElement(strTag);
  TiXmlNode *pNewNode = pRootNode->InsertEndChild(newElement);
  if (pNewNode)
  {
    TiXmlText value(strValue);
    pNewNode->InsertEndChild(value);
  }
}

void CXMLUtils::SetInt(TiXmlNode* pRootNode, const char *strTag, int value)
{
  std::string strValue = StringUtils::Format("%i", value);
  SetString(pRootNode, strTag, strValue);
}

void CXMLUtils::SetLong(TiXmlNode* pRootNode, const char *strTag, long value)
{
  std::string strValue = StringUtils::Format("%ld", value);
  SetString(pRootNode, strTag, strValue);
}

void CXMLUtils::SetFloat(TiXmlNode* pRootNode, const char *strTag, float value)
{
  std::string strValue = StringUtils::Format("%f", value);
  SetString(pRootNode, strTag, strValue);
}

void CXMLUtils::SetBoolean(TiXmlNode* pRootNode, const char *strTag, bool value)
{
  SetString(pRootNode, strTag, value ? "true" : "false");
}

void CXMLUtils::SetHex(TiXmlNode* pRootNode, const char *strTag, uint32_t value)
{
  std::string strValue = StringUtils::Format("%x", value);
  SetString(pRootNode, strTag, strValue);
}

void CXMLUtils::SetPath(TiXmlNode* pRootNode, const char *strTag, const std::string& strValue)
{
  TiXmlElement newElement(strTag);
  newElement.SetAttribute("pathversion", path_version);
  TiXmlNode *pNewNode = pRootNode->InsertEndChild(newElement);
  if (pNewNode)
  {
    TiXmlText value(strValue);
    pNewNode->InsertEndChild(value);
  }
}

void CXMLUtils::SetDate(TiXmlNode* pRootNode, const char *strTag, const CDateTime& date)
{
  SetString(pRootNode, strTag, date.IsValid() ? date.GetAsDBDate() : "");
}

void CXMLUtils::SetDateTime(TiXmlNode* pRootNode, const char *strTag, const CDateTime& dateTime)
{
  SetString(pRootNode, strTag, dateTime.IsValid() ? dateTime.GetAsDBDateTime() : "");
}

// moved from XBMCTinyXML
CXMLUtils::CXMLUtils()
: TiXmlDocument()
{
}

CXMLUtils::CXMLUtils(const char *documentName)
: TiXmlDocument(documentName)
{
}

bool CXMLUtils::LoadFile(const char *_filename, TiXmlEncoding encoding)
{
  return LoadFile(std::string(_filename), encoding);
}

bool CXMLUtils::LoadFile(const std::string& _filename, TiXmlEncoding encoding)
{
  value = _filename.c_str();

  XFILE::CFile file;
  XFILE::auto_buffer buffer;

  if (file.LoadFile(value, buffer) <= 0)
  {
    SetError(TIXML_ERROR_OPENING_FILE, NULL, NULL, TIXML_ENCODING_UNKNOWN);
    return false;
  }

  // Delete the existing data:
  Clear();
  location.Clear();

  std::string data(buffer.get(), buffer.length());
  buffer.clear(); // free memory early

  if (encoding == TIXML_ENCODING_UNKNOWN)
    Parse(data, file.GetContentCharset());
  else
    Parse(data, encoding);

  if (Error())
    return false;
  return true;
}

bool CXMLUtils::LoadFile(FILE *f, TiXmlEncoding encoding)
{
  std::string data;
  char buf[BUFFER_SIZE];
  memset(buf, 0, BUFFER_SIZE);
  int result;
  while ((result = fread(buf, 1, BUFFER_SIZE, f)) > 0)
    data.append(buf, result);
  return Parse(data, encoding);
}

bool CXMLUtils::SaveFile(const char *_filename) const
{
  return SaveFile(std::string(_filename));
}

bool CXMLUtils::SaveFile(const std::string& filename) const
{
  XFILE::CFile file;
  if (file.OpenForWrite(filename, true))
  {
    TiXmlPrinter printer;
    Accept(&printer);
    return file.Write(printer.CStr(), printer.Size()) == static_cast<ssize_t>(printer.Size());
  }
  return false;
}

bool CXMLUtils::Parse(const char *_data, TiXmlEncoding encoding)
{
  return Parse(std::string(_data), encoding);
}

bool CXMLUtils::Parse(const std::string& data, const std::string& dataCharset)
{
  m_SuggestedCharset = dataCharset;
  StringUtils::ToUpper(m_SuggestedCharset);
  return Parse(data, TIXML_ENCODING_UNKNOWN);
}

bool CXMLUtils::Parse(const std::string& data, TiXmlEncoding encoding /*= TIXML_DEFAULT_ENCODING */)
{
  m_UsedCharset.clear();
  if (encoding != TIXML_ENCODING_UNKNOWN)
  { // encoding != TIXML_ENCODING_UNKNOWN means "do not use m_SuggestedCharset and charset detection"
    m_SuggestedCharset.clear();
    if (encoding == TIXML_ENCODING_UTF8)
      m_UsedCharset = "UTF-8";

    return InternalParse(data, encoding);
  }

  if (!m_SuggestedCharset.empty() && TryParse(data, m_SuggestedCharset))
    return true;

  std::string detectedCharset;
  if (CCharsetDetection::DetectXmlEncoding(data, detectedCharset) && TryParse(data, detectedCharset))
  {
    if (!m_SuggestedCharset.empty())
      CLog::Log(LOGWARNING, "%s: \"%s\" charset was used instead of suggested charset \"%s\" for %s", __FUNCTION__, m_UsedCharset.c_str(), m_SuggestedCharset.c_str(),
                  (value.empty() ? "XML data" : ("file \"" + value + "\"").c_str()));

    return true;
  }

  // check for valid UTF-8
  if (m_SuggestedCharset != "UTF-8" && detectedCharset != "UTF-8" && CUtf8Utils::isValidUtf8(data) &&
      TryParse(data, "UTF-8"))
  {
    if (!m_SuggestedCharset.empty())
      CLog::Log(LOGWARNING, "%s: \"%s\" charset was used instead of suggested charset \"%s\" for %s", __FUNCTION__, m_UsedCharset.c_str(), m_SuggestedCharset.c_str(),
                  (value.empty() ? "XML data" : ("file \"" + value + "\"").c_str()));
    else if (!detectedCharset.empty())
      CLog::Log(LOGWARNING, "%s: \"%s\" charset was used instead of detected charset \"%s\" for %s", __FUNCTION__, m_UsedCharset.c_str(), detectedCharset.c_str(),
                  (value.empty() ? "XML data" : ("file \"" + value + "\"").c_str()));
    return true;
  }

  // fallback: try user GUI charset
  if (TryParse(data, g_langInfo.GetGuiCharSet()))
  {
    if (!m_SuggestedCharset.empty())
      CLog::Log(LOGWARNING, "%s: \"%s\" charset was used instead of suggested charset \"%s\" for %s", __FUNCTION__, m_UsedCharset.c_str(), m_SuggestedCharset.c_str(),
                  (value.empty() ? "XML data" : ("file \"" + value + "\"").c_str()));
    else if (!detectedCharset.empty())
      CLog::Log(LOGWARNING, "%s: \"%s\" charset was used instead of detected charset \"%s\" for %s", __FUNCTION__, m_UsedCharset.c_str(), detectedCharset.c_str(),
                  (value.empty() ? "XML data" : ("file \"" + value + "\"").c_str()));
    return true;
  }

  // can't detect correct data charset, try to process data as is
  if (InternalParse(data, TIXML_ENCODING_UNKNOWN))
  {
    if (!m_SuggestedCharset.empty())
      CLog::Log(LOGWARNING, "%s: Processed %s as unknown encoding instead of suggested \"%s\"", __FUNCTION__,
                  (value.empty() ? "XML data" : ("file \"" + value + "\"").c_str()), m_SuggestedCharset.c_str());
    else if (!detectedCharset.empty())
      CLog::Log(LOGWARNING, "%s: Processed %s as unknown encoding instead of detected \"%s\"", __FUNCTION__,
                  (value.empty() ? "XML data" : ("file \"" + value + "\"").c_str()), detectedCharset.c_str());
    return true;
  }

  return false;
}

bool CXMLUtils::TryParse(const std::string& data, const std::string& tryDataCharset)
{
  if (tryDataCharset == "UTF-8")
    InternalParse(data, TIXML_ENCODING_UTF8); // process data without conversion
  else if (!tryDataCharset.empty())
  {
    std::string converted;
    /* some wrong conversions can leave US-ASCII XML header and structure untouched but break non-English data
     * so conversion must fail on wrong character and then other encodings will be tried */
    if (!g_charsetConverter.ToUtf8(tryDataCharset, data, converted, true) || converted.empty())
      return false; // can't convert data

    InternalParse(converted, TIXML_ENCODING_UTF8);
  }
  else
    InternalParse(data, TIXML_ENCODING_LEGACY);

  // 'Error()' contains result of last run of 'TiXmlDocument::Parse()'
  if (Error())
  {
    Clear();
    location.Clear();

    return false;
  }

  m_UsedCharset = tryDataCharset;
  return true;
}

bool CXMLUtils::InternalParse(const std::string& rawdata, TiXmlEncoding encoding /*= TIXML_DEFAULT_ENCODING */)
{
  // Preprocess string, replacing '&' with '&amp; for invalid XML entities
  size_t pos = rawdata.find('&');
  if (pos == std::string::npos)
    return (TiXmlDocument::Parse(rawdata.c_str(), NULL, encoding) != NULL); // nothing to fix, process data directly

  std::string data(rawdata);
  CRegExp re(false, CRegExp::asciiOnly, "^&(amp|lt|gt|quot|apos|#x[a-fA-F0-9]{1,4}|#[0-9]{1,5});.*");
  do
  {
    if (re.RegFind(data, pos, MAX_ENTITY_LENGTH) < 0)
      data.insert(pos + 1, "amp;");
    pos = data.find('&', pos + 1);
  } while (pos != std::string::npos);

  return (TiXmlDocument::Parse(data.c_str(), NULL, encoding) != NULL);
}
