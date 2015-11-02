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

#include "test/TestUtils.h"
#include "utils/XMLUtils.h"
#include "utils/StringUtils.h"
#include "XBDateTime.h"

#include "gtest/gtest.h"

TEST(TestXMLUtils, GetHex)
{
  CXMLUtils a;
  uint32_t ref, val;

  a.Parse("<root><node>0xFF</node></root>");
  EXPECT_TRUE(CXMLUtils::GetHex(a.RootElement(), "node", val));

  ref = 0xFF;
  EXPECT_EQ(ref, val);
}

TEST(TestXMLUtils, GetUInt)
{
  CXMLUtils a;
  uint32_t ref, val;

  a.Parse("<root><node>1000</node></root>");
  EXPECT_TRUE(CXMLUtils::GetUInt(a.RootElement(), "node", val));

  ref = 1000;
  EXPECT_EQ(ref, val);
}

TEST(TestXMLUtils, GetLong)
{
  CXMLUtils a;
  long ref, val;

  a.Parse("<root><node>1000</node></root>");
  EXPECT_TRUE(CXMLUtils::GetLong(a.RootElement(), "node", val));

  ref = 1000;
  EXPECT_EQ(ref, val);
}

TEST(TestXMLUtils, GetFloat)
{
  CXMLUtils a;
  float ref, val;

  a.Parse("<root><node>1000.1f</node></root>");
  EXPECT_TRUE(CXMLUtils::GetFloat(a.RootElement(), "node", val));
  EXPECT_TRUE(CXMLUtils::GetFloat(a.RootElement(), "node", val, 1000.0f,
                                 1000.2f));
  ref = 1000.1f;
  EXPECT_EQ(ref, val);
}

TEST(TestXMLUtils, GetDouble)
{
  CXMLUtils a;
  double val;
  std::string refstr, valstr;

  a.Parse("<root><node>1000.1f</node></root>");
  EXPECT_TRUE(CXMLUtils::GetDouble(a.RootElement(), "node", val));

  refstr = "1000.100000";
  valstr = StringUtils::Format("%f", val);
  EXPECT_STREQ(refstr.c_str(), valstr.c_str());
}

TEST(TestXMLUtils, GetInt)
{
  CXMLUtils a;
  int ref, val;

  a.Parse("<root><node>1000</node></root>");
  EXPECT_TRUE(CXMLUtils::GetInt(a.RootElement(), "node", val));
  EXPECT_TRUE(CXMLUtils::GetInt(a.RootElement(), "node", val, 999, 1001));

  ref = 1000;
  EXPECT_EQ(ref, val);
}

TEST(TestXMLUtils, GetBoolean)
{
  CXMLUtils a;
  bool ref, val;

  a.Parse("<root><node>true</node></root>");
  EXPECT_TRUE(CXMLUtils::GetBoolean(a.RootElement(), "node", val));

  ref = true;
  EXPECT_EQ(ref, val);
}

TEST(TestXMLUtils, GetString)
{
  CXMLUtils a;
  std::string ref, val;

  a.Parse("<root><node>some string</node></root>");
  EXPECT_TRUE(CXMLUtils::GetString(a.RootElement(), "node", val));

  ref = "some string";
  EXPECT_STREQ(ref.c_str(), val.c_str());
}

TEST(TestXMLUtils, GetAdditiveString)
{
  CXMLUtils a, b;
  std::string ref, val;

  a.Parse("<root>\n"
          "  <node>some string1</node>\n"
          "  <node>some string2</node>\n"
          "  <node>some string3</node>\n"
          "  <node>some string4</node>\n"
          "  <node>some string5</node>\n"
          "</root>\n");
  EXPECT_TRUE(CXMLUtils::GetAdditiveString(a.RootElement(), "node", ",", val));

  ref = "some string1,some string2,some string3,some string4,some string5";
  EXPECT_STREQ(ref.c_str(), val.c_str());

  val.clear();
  b.Parse("<root>\n"
          "  <node>some string1</node>\n"
          "  <node>some string2</node>\n"
          "  <node clear=\"true\">some string3</node>\n"
          "  <node>some string4</node>\n"
          "  <node>some string5</node>\n"
          "</root>\n");
  EXPECT_TRUE(CXMLUtils::GetAdditiveString(b.RootElement(), "node", ",", val));

  ref = "some string3,some string4,some string5";
  EXPECT_STREQ(ref.c_str(), val.c_str());
}

TEST(TestXMLUtils, GetStringArray)
{
  CXMLUtils a;
  std::vector<std::string> strarray;

  a.Parse("<root>\n"
          "  <node>some string1</node>\n"
          "  <node>some string2</node>\n"
          "  <node>some string3</node>\n"
          "  <node>some string4</node>\n"
          "  <node>some string5</node>\n"
          "</root>\n");
  EXPECT_TRUE(CXMLUtils::GetStringArray(a.RootElement(), "node", strarray));

  EXPECT_STREQ("some string1", strarray.at(0).c_str());
  EXPECT_STREQ("some string2", strarray.at(1).c_str());
  EXPECT_STREQ("some string3", strarray.at(2).c_str());
  EXPECT_STREQ("some string4", strarray.at(3).c_str());
  EXPECT_STREQ("some string5", strarray.at(4).c_str());
}

TEST(TestXMLUtils, GetPath)
{
  CXMLUtils a, b;
  std::string ref, val;

  a.Parse("<root><node urlencoded=\"yes\">special://xbmc/</node></root>");
  EXPECT_TRUE(CXMLUtils::GetPath(a.RootElement(), "node", val));

  ref = "special://xbmc/";
  EXPECT_STREQ(ref.c_str(), val.c_str());

  val.clear();
  b.Parse("<root><node>special://xbmcbin/</node></root>");
  EXPECT_TRUE(CXMLUtils::GetPath(b.RootElement(), "node", val));

  ref = "special://xbmcbin/";
  EXPECT_STREQ(ref.c_str(), val.c_str());
}

TEST(TestXMLUtils, GetDate)
{
  CXMLUtils a;
  CDateTime ref, val;

  a.Parse("<root><node>2012-07-08</node></root>");
  EXPECT_TRUE(CXMLUtils::GetDate(a.RootElement(), "node", val));
  ref.SetDate(2012, 7, 8);
  EXPECT_TRUE(ref == val);
}

TEST(TestXMLUtils, GetDateTime)
{
  CXMLUtils a;
  CDateTime ref, val;

  a.Parse("<root><node>2012-07-08 01:02:03</node></root>");
  EXPECT_TRUE(CXMLUtils::GetDateTime(a.RootElement(), "node", val));
  ref.SetDateTime(2012, 7, 8, 1, 2, 3);
  EXPECT_TRUE(ref == val);
}

TEST(TestXMLUtils, SetString)
{
  CXMLUtils a;
  std::string ref, val;

  a.Parse("<root></root>");
  CXMLUtils::SetString(a.RootElement(), "node", "some string");
  EXPECT_TRUE(CXMLUtils::GetString(a.RootElement(), "node", val));

  ref = "some string";
  EXPECT_STREQ(ref.c_str(), val.c_str());
}

TEST(TestXMLUtils, SetAdditiveString)
{
  CXMLUtils a;
  std::string ref, val;

  a.Parse("<root></root>");
  CXMLUtils::SetAdditiveString(a.RootElement(), "node", ",",
    "some string1,some string2,some string3,some string4,some string5");
  EXPECT_TRUE(CXMLUtils::GetAdditiveString(a.RootElement(), "node", ",", val));

  ref = "some string1,some string2,some string3,some string4,some string5";
  EXPECT_STREQ(ref.c_str(), val.c_str());
}

TEST(TestXMLUtils, SetStringArray)
{
  CXMLUtils a;
  std::vector<std::string> strarray;
  strarray.push_back("some string1");
  strarray.push_back("some string2");
  strarray.push_back("some string3");
  strarray.push_back("some string4");
  strarray.push_back("some string5");

  a.Parse("<root></root>");
  CXMLUtils::SetStringArray(a.RootElement(), "node", strarray);
  EXPECT_TRUE(CXMLUtils::GetStringArray(a.RootElement(), "node", strarray));

  EXPECT_STREQ("some string1", strarray.at(0).c_str());
  EXPECT_STREQ("some string2", strarray.at(1).c_str());
  EXPECT_STREQ("some string3", strarray.at(2).c_str());
  EXPECT_STREQ("some string4", strarray.at(3).c_str());
  EXPECT_STREQ("some string5", strarray.at(4).c_str());
}

TEST(TestXMLUtils, SetInt)
{
  CXMLUtils a;
  int ref, val;

  a.Parse("<root></root>");
  CXMLUtils::SetInt(a.RootElement(), "node", 1000);
  EXPECT_TRUE(CXMLUtils::GetInt(a.RootElement(), "node", val));

  ref = 1000;
  EXPECT_EQ(ref, val);
}

TEST(TestXMLUtils, SetFloat)
{
  CXMLUtils a;
  float ref, val;

  a.Parse("<root></root>");
  CXMLUtils::SetFloat(a.RootElement(), "node", 1000.1f);
  EXPECT_TRUE(CXMLUtils::GetFloat(a.RootElement(), "node", val));

  ref = 1000.1f;
  EXPECT_EQ(ref, val);
}

TEST(TestXMLUtils, SetBoolean)
{
  CXMLUtils a;
  bool ref, val;

  a.Parse("<root></root>");
  CXMLUtils::SetBoolean(a.RootElement(), "node", true);
  EXPECT_TRUE(CXMLUtils::GetBoolean(a.RootElement(), "node", val));

  ref = true;
  EXPECT_EQ(ref, val);
}

TEST(TestXMLUtils, SetHex)
{
  CXMLUtils a;
  uint32_t ref, val;

  a.Parse("<root></root>");
  CXMLUtils::SetHex(a.RootElement(), "node", 0xFF);
  EXPECT_TRUE(CXMLUtils::GetHex(a.RootElement(), "node", val));

  ref = 0xFF;
  EXPECT_EQ(ref, val);
}

TEST(TestXMLUtils, SetPath)
{
  CXMLUtils a;
  std::string ref, val;

  a.Parse("<root></root>");
  CXMLUtils::SetPath(a.RootElement(), "node", "special://xbmc/");
  EXPECT_TRUE(CXMLUtils::GetPath(a.RootElement(), "node", val));

  ref = "special://xbmc/";
  EXPECT_STREQ(ref.c_str(), val.c_str());
}

TEST(TestXMLUtils, SetLong)
{
  CXMLUtils a;
  long ref, val;

  a.Parse("<root></root>");
  CXMLUtils::SetLong(a.RootElement(), "node", 1000);
  EXPECT_TRUE(CXMLUtils::GetLong(a.RootElement(), "node", val));

  ref = 1000;
  EXPECT_EQ(ref, val);
}

TEST(TestXMLUtils, SetDate)
{
  CXMLUtils a;
  CDateTime ref, val;

  a.Parse("<root></root>");
  ref.SetDate(2012, 7, 8);
  CXMLUtils::SetDate(a.RootElement(), "node", ref);
  EXPECT_TRUE(CXMLUtils::GetDate(a.RootElement(), "node", val));
  EXPECT_TRUE(ref == val);
}

TEST(TestXMLUtils, SetDateTime)
{
  CXMLUtils a;
  CDateTime ref, val;

  a.Parse("<root></root>");
  ref.SetDateTime(2012, 7, 8, 1, 2, 3);
  CXMLUtils::SetDateTime(a.RootElement(), "node", ref);
  EXPECT_TRUE(CXMLUtils::GetDateTime(a.RootElement(), "node", val));
  EXPECT_TRUE(ref == val);
}

TEST(TestXMLUtils, ParseFromString)
{
  bool retval = false;
  // scraper results with unescaped &
  CXMLUtils doc;
  std::string data("<details><url function=\"ParseTMDBRating\" "
                  "cache=\"tmdb-en-12244.json\">"
                  "http://api.themoviedb.org/3/movie/12244"
                  "?api_key=57983e31fb435df4df77afb854740ea9"
                  "&language=en&#x3f;&#x003F;&#0063;</url></details>");
  doc.Parse(data.c_str());
  TiXmlNode *root = doc.RootElement();
  if (root && root->ValueStr() == "details")
  {
    TiXmlElement *url = root->FirstChildElement("url");
    if (url && url->FirstChild())
    {
      retval = (url->FirstChild()->ValueStr() == "http://api.themoviedb.org/3/movie/12244?api_key=57983e31fb435df4df77afb854740ea9&language=en???");
    }
  }
  EXPECT_TRUE(retval);
}

TEST(TestXMLUtils, ParseFromFileHandle)
{
  bool retval = false;
  // scraper results with unescaped &
  CXMLUtils doc;
  FILE *f = fopen(XBMC_REF_FILE_PATH("/xbmc/utils/test/XMLUtils-test.xml").c_str(), "r");
  ASSERT_NE(nullptr, f);
  doc.LoadFile(f);
  fclose(f);
  TiXmlNode *root = doc.RootElement();
  if (root && root->ValueStr() == "details")
  {
    TiXmlElement *url = root->FirstChildElement("url");
    if (url && url->FirstChild())
    {
      std::string str = url->FirstChild()->ValueStr();
      retval = (StringUtils::Trim(str) == "http://api.themoviedb.org/3/movie/12244?api_key=57983e31fb435df4df77afb854740ea9&language=en???");
    }
  }
  EXPECT_TRUE(retval);
}
