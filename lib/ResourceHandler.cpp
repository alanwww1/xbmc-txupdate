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

void CResourceHandler::CreateMissingDirs(std::string strResRootDir, int resType)
{
  if (resType == SKIN || resType == CORE)
  {
    if (!DirExists(strResRootDir + "language"))
    {
      CLog::Log(logINFO, "ResHandler: Creating language directory for new resource on Transifex");
      MakeDir(strResRootDir + "language");
    }
    m_langDir = strResRootDir + "language" + DirSepChar;
  }
  else if (resType == ADDON)
  {
    if (!DirExists(strResRootDir + "resources"))
    {
      CLog::Log(logINFO, "ResHandler: Creating \"resources\" directory for new addon resource on Transifex");
      MakeDir(strResRootDir + "resources");
    }
    if (!DirExists(strResRootDir + "resources" + DirSepChar + "language"))
    {
      CLog::Log(logINFO, "ResHandler: Creating language directory for new addon resource on Transifex");
      MakeDir(strResRootDir + "resources" + DirSepChar + "language");
    }
    m_langDir = strResRootDir + "resources" + DirSepChar + "language" + DirSepChar;
  }
  else
    CLog::Log(logERROR, "ResHandler::CreateMissingDirs: Unknown resource type given.");

  return;
};

bool CResourceHandler::FetchPOFilesTXToMem(std::string strURL)
{
  CLog::Log(logINFO, "ResHandler: Starting to load resource from TX URL: %s into memory",strURL.c_str());

  std::string strtemp = g_HTTPHandler.GetURLToSTR(strURL + "stats/");
//  printf("%s, strlength: %i", strtemp.c_str(), strtemp.size());

  char cstrtemp[strtemp.size()];
  strcpy(cstrtemp, strtemp.c_str());

  CJSONHandler JSONHandler;
  std::list<std::string> listLangsTX = JSONHandler.ParseAvailLanguages(strtemp);

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

bool CResourceHandler::FetchPOFilesUpstreamToMem(CXMLResdata XMLResdata, std::list<std::string> listLangsAll)
{
  CLog::Log(logINFO, "ResHandler: Starting to load resource from Upsream URL: %s into memory",XMLResdata.strUptreamURL.c_str());

  std::list<std::string> listLangs;

  if (XMLResdata.strLangsFromUpstream == "all")
    listLangs = listLangsAll;
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

  std::string strLangdirPrefix;

  m_resType = XMLResdata.Restype;
  m_strResdir = XMLResdata.strResDirectory;
  switch (m_resType)
  {
    case ADDON :
      strLangdirPrefix = m_strResdir + "resources/language/";
      break;
    case ADDON_NOSTRINGS :
      strLangdirPrefix = m_strResdir;
      break;
    case SKIN :
      strLangdirPrefix = m_strResdir + "language/";
      break;
    case CORE :
      strLangdirPrefix = m_strResdir + "language/";
      break;
    default:
      CLog::Log(logINFO, "ResHandler: No resourcetype defined for upsream URL:",XMLResdata.strUptreamURL.c_str());
  }

  if (m_resType == CORE)
    m_AddonXMLHandler.FetchCoreVersionUpstr(XMLResdata.strUptreamURL + "xbmc/GUIInfoManager.h");
  else
    m_AddonXMLHandler.FetchAddonXMLFileUpstr(XMLResdata.strUptreamURL + "addon.xml");

  if (m_resType == ADDON_NOSTRINGS)
    listLangs.clear();

  for (std::list<std::string>::iterator it = listLangs.begin(); it != listLangs.end(); it++)
  {
    CPOHandler POHandler;
    if (POHandler.FetchPOURLToMem(XMLResdata.strUptreamURL + strLangdirPrefix + g_LCodeHandler.FindLang(*it) + DirSepChar + "strings.po",
                              true))
    {
      POHandler.SetIfIsEnglish(*it == "en");
      m_mapPOFiles[*it] = POHandler;
      std::string strLang = *it;
      strLang.resize(20, ' ');
      CLog::Log(logINFO, "POHandler: %s\t\t%i\t\t%i\t\t%i", strLang.c_str(), POHandler.GetNumEntriesCount(),
              POHandler.GetClassEntriesCount(), POHandler.GetCommntEntriesCount());
    }
  }
//  GetResTypeFromTX(strCategory);
  return true;
}



bool CResourceHandler::WritePOToFiles(std::string strResourceDir, std::string strPOsuffix, std::string strResname)
{
  CLog::Log(logINFO, "ResHandler: Starting to write resource from memory to directory: %s",strResourceDir.c_str());

  if (!m_mapPOFiles.empty())
    CreateMissingDirs(strResourceDir, m_resType);

  std::string strListNewDirs;

  for (T_itmapPOFiles itmapPOFiles = m_mapPOFiles.begin(); itmapPOFiles != m_mapPOFiles.end(); itmapPOFiles++)
  {
//    if (itmapPOFiles->first == "en")
//      continue;
    std::string strPODir = m_langDir + g_LCodeHandler.FindLang(itmapPOFiles->first);
    if (!DirExists(strPODir))
    {
      strListNewDirs += g_LCodeHandler.FindLang(itmapPOFiles->first) + " ";
      MakeDir(strPODir);
    }

    CPOHandler * pPOHandler = &m_mapPOFiles[itmapPOFiles->first];
    pPOHandler->WritePOFile(strPODir + DirSepChar + "strings.po" + strPOsuffix);
    std::string strLang = g_LCodeHandler.FindLang(itmapPOFiles->first);
    strLang.resize(20, ' ');
    CLog::Log(logINFO, "POHandler: %s\t\t%i\t\t%i\t\t%i", strLang.c_str(), pPOHandler->GetNumEntriesCount(),
              pPOHandler->GetClassEntriesCount(), pPOHandler->GetCommntEntriesCount());
  }

  // update local addon.xml file
  if (strResname != "xbmc-core")
    m_AddonXMLHandler.UpdateAddonXMLFile(strResourceDir + "addon.xml");

  if (!strListNewDirs.empty())
    CLog::Log(logINFO, "POHandler: New local directories already existing on Transifex: %s", strListNewDirs.c_str());
  return true;
}

T_itmapPOFiles CResourceHandler::IterateToMapIndex(T_itmapPOFiles it, size_t index)
{
  for (size_t i = 0; i != index; i++) it++;
  return it;
}
