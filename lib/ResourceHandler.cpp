/*
 *      Copyright (C) 2012 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "ResourceHandler.h"
#include <list>
#include <algorithm>
#include "JSONHandler.h"
#include "HTTPUtils.h"
#include "xbmclangcodes.h"

using namespace std;

CResourceHandler::CResourceHandler()
{};

CResourceHandler::~CResourceHandler()
{};

CPOHandler* CResourceHandler::GetPOData(std::string strLang)
{
  if (m_mapPOFiles.find(strLang) != m_mapPOFiles.end())
    return &m_mapPOFiles[strLang];
  return NULL;
}

// Download from Transifex related functions

bool CResourceHandler::FetchPOFilesTXToMem(std::string strURL)
{
  g_HTTPHandler.Cleanup();
  g_HTTPHandler.ReInit();
  CLog::Log(logINFO, "ResHandler: Starting to load resource from TX URL: %s into memory",strURL.c_str());

  std::string strtemp = g_HTTPHandler.GetURLToSTR(strURL + "stats/");
  if (strtemp.empty())
    CLog::Log(logERROR, "ResHandler::FetchPOFilesTXToMem: error getting po file list from transifex.net");

  char cstrtemp[strtemp.size()];
  strcpy(cstrtemp, strtemp.c_str());

  CJSONHandler JSONHandler;
  std::list<std::string> listLangsTX = JSONHandler.ParseAvailLanguagesTX(strtemp);

  CPOHandler POHandler;

  for (std::list<std::string>::iterator it = listLangsTX.begin(); it != listLangsTX.end(); it++)
  {
    m_mapPOFiles[*it] = POHandler;
    CPOHandler * pPOHandler = &m_mapPOFiles[*it];
    pPOHandler->FetchPOURLToMem(strURL + "translation/" + *it + "/?file", false);
    pPOHandler->SetIfIsEnglish(*it == "en");
    std::string strLang = *it;
    strLang.resize(20, ' ');
    CLog::Log(logINFO, "POHandler: %s\t\t%i\t\t%i\t\t%i", strLang.c_str(), pPOHandler->GetNumEntriesCount(),
              pPOHandler->GetClassEntriesCount(), pPOHandler->GetCommntEntriesCount());
  }
  return true;
}

bool CResourceHandler::FetchPOFilesUpstreamToMem(CXMLResdata XMLResdata, std::list<std::string> listLangsTX)
{
  g_HTTPHandler.Cleanup();
  g_HTTPHandler.ReInit();
  CLog::Log(logINFO, "ResHandler: Starting to load resource from Upsream URL: %s into memory",XMLResdata.strUptreamURL.c_str());

  std::string strLangdirPrefix;

  if (XMLResdata.Restype == CORE)
  {
    m_AddonXMLHandler.FetchCoreVersionUpstr(XMLResdata.strUptreamURL + "xbmc/GUIInfoManager.h");
    strLangdirPrefix = "language/";
  }
  else
  {
    m_AddonXMLHandler.FetchAddonXMLFileUpstr(XMLResdata.strUptreamURL + "addon.xml");
    if (XMLResdata.Restype == SKIN)
      strLangdirPrefix = "language/";
    else if (XMLResdata.Restype == ADDON)
      strLangdirPrefix = "resources/language/";
    else if (XMLResdata.Restype == ADDON_NOSTRINGS)
      return true;
  }

  std::list<std::string> listLangs;

  if (XMLResdata.strLangsFromUpstream == "tx_all")
  {
    CLog::Log(logINFO, "ResHandler: using language list previously got from Transifex");
    listLangs = listLangsTX;
  }
  else if (XMLResdata.strLangsFromUpstream == "github_all")
  {
    CLog::Log(logINFO, "ResHandler: using language list dwonloaded with github API");
    size_t pos1, pos2, pos3;
    std::string strGitHubURL, strGitBranch;
    if (XMLResdata.strUptreamURL.find("raw.github.com/") == std::string::npos)
      CLog::Log(logERROR, "ResHandler: Wrong Github URL format given");
    pos1 = XMLResdata.strUptreamURL.find("raw.github.com/")+15;
    pos2 = XMLResdata.strUptreamURL.find("/", pos1+1);
    pos2 = XMLResdata.strUptreamURL.find("/", pos2+1);
    pos3 = XMLResdata.strUptreamURL.find("/", pos2+1);
    strGitHubURL = "https://api.github.com/repos/" + XMLResdata.strUptreamURL.substr(pos1, pos2-pos1);
    strGitHubURL += "/contents";
    strGitHubURL += XMLResdata.strUptreamURL.substr(pos3, XMLResdata.strUptreamURL.size() - pos3 - 1);
    strGitBranch = XMLResdata.strUptreamURL.substr(pos2+1, pos3-pos2-1);
    if (XMLResdata.Restype == SKIN || XMLResdata.Restype == CORE)
      strGitHubURL += "/language";
    else if (XMLResdata.Restype == ADDON)
      strGitHubURL += "/resources/language";
    strGitHubURL += "?ref=" + strGitBranch;

    printf ("%s", strGitHubURL.c_str());
    std::string strtemp = g_HTTPHandler.GetURLToSTR(strGitHubURL);
    if (strtemp.empty())
      CLog::Log(logERROR, "ResHandler::FetchPOFilesTXToMem: error getting po file list from transifex.net");

    char cstrtemp[strtemp.size()];
    strcpy(cstrtemp, strtemp.c_str());

    CJSONHandler JSONHandler;
    listLangs = JSONHandler.ParseAvailLanguagesGITHUB(strtemp);
  }
  else
  {
    std::string strLangs;
    if (XMLResdata.strLangsFromUpstream.empty())
      strLangs = "en";
    else
      strLangs = "en " + XMLResdata.strLangsFromUpstream;
    size_t posEnd;

    do
    {
      posEnd = strLangs.find(" ");
    if (posEnd != std::string::npos)
    {
      listLangs.push_back(strLangs.substr(0,posEnd));
      strLangs = strLangs.substr(posEnd+1);
    }
    else
      listLangs.push_back(strLangs);
    }
    while (posEnd != std::string::npos);
  }

  for (std::list<std::string>::iterator it = listLangs.begin(); it != listLangs.end(); it++)
  {
    CPOHandler POHandler;
    if (POHandler.FetchPOURLToMem(XMLResdata.strUptreamURL + strLangdirPrefix + g_LCodeHandler.FindLang(*it) + DirSepChar + "strings.po",true))
    {
      POHandler.SetIfIsEnglish(*it == "en");
      m_mapPOFiles[*it] = POHandler;
      std::string strLang = *it;
      strLang.resize(20, ' ');
      CLog::Log(logINFO, "POHandler: %s\t\t%i\t\t%i\t\t%i", strLang.c_str(), POHandler.GetNumEntriesCount(),
              POHandler.GetClassEntriesCount(), POHandler.GetCommntEntriesCount());
    }
  }
  return true;
}

bool CResourceHandler::WritePOToFiles(std::string strProjRootDir, std::string strPOsuffix, std::string strResname, CXMLResdata XMLResdata)
{
  std::string strResourceDir, strLangDir;
  switch (XMLResdata.Restype)
  {
    case ADDON: case ADDON_NOSTRINGS:
      strResourceDir = strProjRootDir + XMLResdata.strResDirectory + DirSepChar + strResname +DirSepChar;
      strLangDir = strResourceDir + "resources" + DirSepChar + "language" + DirSepChar;
      break;
    case SKIN:
      strResourceDir = strProjRootDir + XMLResdata.strResDirectory + DirSepChar + strResname +DirSepChar;
      strLangDir = strResourceDir + "language" + DirSepChar;
      break;
    case CORE:
      strResourceDir = strProjRootDir + XMLResdata.strResDirectory + DirSepChar;
      strLangDir = strResourceDir + "language" + DirSepChar;
      break;
    default:
      CLog::Log(logERROR, "ResHandler: No resourcetype defined for resource: %s",strResname.c_str());
  }

  for (T_itmapPOFiles itmapPOFiles = m_mapPOFiles.begin(); itmapPOFiles != m_mapPOFiles.end(); itmapPOFiles++)
  {
    std::string strLang = g_LCodeHandler.FindLang(itmapPOFiles->first);
    std::string strPODir = strLangDir + strLang;

    CPOHandler * pPOHandler = &m_mapPOFiles[itmapPOFiles->first];
    pPOHandler->WritePOFile(strPODir + DirSepChar + "strings.po" + strPOsuffix);

    strLang.resize(20, ' ');
    CLog::Log(logINFO, "POHandler: %s\t\t%i\t\t%i\t\t%i", strLang.c_str(), pPOHandler->GetNumEntriesCount(),
              pPOHandler->GetClassEntriesCount(), pPOHandler->GetCommntEntriesCount());
  }

  // update local addon.xml file
  if (strResname != "xbmc.core")
    m_AddonXMLHandler.UpdateAddonXMLFile(strResourceDir + "addon.xml");

  return true;
}

T_itmapPOFiles CResourceHandler::IterateToMapIndex(T_itmapPOFiles it, size_t index)
{
  for (size_t i = 0; i != index; i++) it++;
  return it;
}
