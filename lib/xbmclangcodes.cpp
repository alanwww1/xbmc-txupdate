/*
 *      Copyright (C) 2005-2014 Team Kodi
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
 *  along with Kodi; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <string>
#include <stdio.h>
#include "xbmclangcodes.h"
#include "Log.h"
#include "HTTPUtils.h"
#include "JSONHandler.h"
#include "TinyXML/tinyxml.h"

using namespace std;

CLCodeHandler g_LCodeHandler;

enum
{
  CUST_LANG_COUNT = 6,
};

CLangcodes CustomLangcodes [CUST_LANG_COUNT] =
{
  {"Chinese (Simple)", "zh", 0, ""},
  {"Chinese (Traditional)", "zh_TW", 0, ""},
  {"English (US)", "en_US", 0, ""},
  {"Hindi (Devanagiri)", "hi", 0, ""},
  {"Serbian (Cyrillic)", "sr_RS", 0, ""},
  {"Spanish", "es", 0, ""}
};

CLCodeHandler::CLCodeHandler()
{}

CLCodeHandler::~CLCodeHandler()
{}

void CLCodeHandler::Init(std::string strURL)
{
  g_HTTPHandler.Cleanup();
  g_HTTPHandler.ReInit();
  std::string strtemp = g_HTTPHandler.GetURLToSTR(strURL);
  if (strtemp.empty())
    CLog::Log(logERROR, "XBMCLangCode::Init: error getting available language list from transifex.net");

  m_mapLCodes = g_Json.ParseTransifexLanguageDatabase(strtemp);

  for (int i=0; i<CUST_LANG_COUNT ; i++)
  {
    if (m_mapLCodes.find(CustomLangcodes[i].Langcode) != m_mapLCodes.end())
      m_mapLCodes[CustomLangcodes[i].Langcode].Langname = CustomLangcodes[i].Langname;
  }

  CLog::Log(logINFO, "LCodeHandler: Succesfully fetched %i language codes from Transifex", m_mapLCodes.size());
}

int CLCodeHandler::GetnPlurals(std::string LangCode)
{
  if (m_mapLCodes.find(LangCode) != m_mapLCodes.end())
    return m_mapLCodes[LangCode].nplurals;
  CLog::Log(logERROR, "LangCodes: GetnPlurals: unable to find langcode: %s", LangCode.c_str());
  return 0;
}

std::string CLCodeHandler::GetPlurForm(std::string LangCode)
{
  if (m_mapLCodes.find(LangCode) != m_mapLCodes.end())
    return m_mapLCodes[LangCode].Pluralform;
  CLog::Log(logERROR, "LangCodes: GetPlurForm: unable to find langcode: %s", LangCode.c_str());
  return "(n != 1)";
}

std::string CLCodeHandler::FindLang(std::string LangCode)
{
  if (m_mapLCodes.find(LangCode) != m_mapLCodes.end())
    return m_mapLCodes[LangCode].Langname;
  CLog::Log(logERROR, "LangCodes: FindLang: unable to find language for langcode: %s", LangCode.c_str());
  return "UNKNOWN";
}

std::string CLCodeHandler::FindLangCode(std::string Lang)
{
  for (itmapLCodes = m_mapLCodes.begin(); itmapLCodes != m_mapLCodes.end() ; itmapLCodes++)
  {
    if (Lang == itmapLCodes->second.Langname)
      return itmapLCodes->first;
  }
  CLog::Log(logERROR, "LangCodes: FindLangCode: unable to find langcode for language: %s", Lang.c_str());
  return "UNKNOWN";
}

std::string CLCodeHandler::VerifyLangCode(std::string LangCode)
{
  std::string strOldCode = LangCode;

  // common mistakes, we correct them on the fly
  if (LangCode == "kr") LangCode = "ko";
  if (LangCode == "cr") LangCode = "hr";
  if (LangCode == "cz") LangCode = "cs";
 
  if (strOldCode != LangCode)
    CLog::Log(logWARNING, "LangCodes: problematic language code: %s was corrected to %s", strOldCode.c_str(), LangCode.c_str());

  if (m_mapLCodes.find(LangCode) != m_mapLCodes.end())
    return LangCode;
  CLog::Log(logERROR, "LangCodes::VerifyLangCode: unable to find language code: %s", LangCode.c_str());
  return "UNKNOWN";
}

void CLCodeHandler::ReadWhiteBlackLangList (std::string strPath)
{
  if (!g_File.FileExist(strPath))
    return;

  std::string strXMLBuffer = g_File.ReadFileToStr(strPath);
  if (strXMLBuffer.empty())
    CLog::Log(logERROR, "CLCodeHandler::ReadWhiteBlackLangList: file error reading XML file from path: %s", strPath.c_str());

  std::map<std::string, std::string>  * pList;

  TiXmlDocument XMLDoc;

  strXMLBuffer.push_back('\n');

  if (!XMLDoc.Parse(strXMLBuffer.c_str(), 0, TIXML_DEFAULT_ENCODING))
    CLog::Log(logERROR, "CLCodeHandler::ReadWhiteBlackLangList: xml file problem: %s %s\n", XMLDoc.ErrorDesc(), strPath.c_str());

  TiXmlElement* pRootElement = XMLDoc.RootElement();

  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueTStr()=="blacklist")
    pList = &m_BlackList;
  else if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueTStr()=="whitelist")
    pList = &m_WhiteList;
  else
    CLog::Log(logERROR, "CLCodeHandler::ReadWhiteBlackLangList: No root element or no child found in input XML file: %s\n", strPath.c_str());

  const TiXmlElement *pChildElement = pRootElement->FirstChildElement("lang");
  const char* pAttrId = NULL;
  const char* pValue = NULL;
  std::string valueString;
  std::string strLcode;

  while (pChildElement)
  {
    pAttrId=pChildElement->Attribute("lcode");
    if (pAttrId && !pChildElement->NoChildren())
    {
      strLcode = pAttrId;
      pValue = pChildElement->FirstChild()->Value();
      valueString = pValue;
      pList->operator[](strLcode) = valueString;
    }
    pChildElement = pChildElement->NextSiblingElement("lang");
  }
  // Free up the allocated memory for the XML file
  XMLDoc.Clear();
  return;
}

bool CLCodeHandler::CheckIfLangCodeBlacklisted (std::string strLcode)
{
  if (strLcode.find("_") != std::string::npos)
    return m_WhiteList.find(strLcode) == m_WhiteList.end();
  else
    return m_BlackList.find(strLcode) != m_BlackList.end();
}

bool CLCodeHandler::CheckIfLangBlacklisted (std::string strLang)
{
  std::string strLcode = FindLangCode(strLang);
  if (strLcode == "UNKNOWN")
    CLog::Log(logERROR, "CLCodeHandler::CheckIfLangBlacklisted: invalid language: %s", strLang.c_str());

  if (strLcode.find("_") != std::string::npos)
    return m_WhiteList.find(strLcode) == m_WhiteList.end();
  else
    return m_BlackList.find(strLcode) != m_BlackList.end();
}