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

bool CResourceHandler::LoadResource(std::string strResRootDir, std::string strPOsuffix)
{
  CLog::Log(logINFO, "ResHandler: Starting to load resource from dir: %s, with PO suffix: %s",
            strResRootDir.c_str(), strPOsuffix.c_str());

  GetResTypeFromDir(strResRootDir);
  CreateMissingDirs(strResRootDir);
  GetLangsFromDir(m_langDir);

  if (m_resType == ADDON || m_resType == ADDON_NOSTRINGS || m_resType == SKIN)
  {
    CLog::Log(logINFO, "ResHandler: Addon or skin resource detected.");
    m_AddonXMLHandler.LoadAddonXMLFile(strResRootDir + "addon.xml");
  }
  else if (m_resType == CORE)
  {
    CLog::Log(logINFO, "ResHandler: XBMC core detected.");
    m_AddonXMLHandler.LoadCoreVersion(strResRootDir + "xbmc" + DirSepChar + "GUIInfoManager.h");
  }

  CLog::Log(logINFO, "POHandler: Language\t\t\tID entries\tnon-ID entries\tInterline-comments");

  for (T_itmapPOFiles itmapPOFiles = m_mapPOFiles.begin(); itmapPOFiles != m_mapPOFiles.end(); itmapPOFiles++)
  {
    itmapPOFiles->second.LoadPOFile(m_langDir + g_LCodeHandler.FindLang(itmapPOFiles->first) + DirSepChar + "strings.po"
                                    + strPOsuffix);
    itmapPOFiles->second.SetIfIsEnglish(itmapPOFiles->first == "en");
//    CLog::Log(logDEBUG, "ResHandler: Loaaded language file for: %s", itmapPOFiles->first.c_str());
  };

  CLog::Log(logINFO, "ResHandler: Found and loaded %i languages.", m_mapPOFiles.size());
  return true;
};

CPOHandler* CResourceHandler::GetPOData(std::string strLang)
{
  if (m_mapPOFiles.find(strLang) != m_mapPOFiles.end())
    return &m_mapPOFiles[strLang];
  return NULL;
}


bool CResourceHandler::GetLangsFromDir(std::string strLangDir)
{
  std::list<std::string> listDirs;
  std::list<std::string>::iterator itlistDirs;
  DIR* Dir;
  struct dirent *DirEntry;
  Dir = opendir(strLangDir.c_str());

  while((DirEntry=readdir(Dir)))
  {
    if (DirEntry->d_type == DT_DIR && DirEntry->d_name[0] != '.')
      listDirs.push_back(DirEntry->d_name);
  }
  listDirs.sort();

  for (itlistDirs = listDirs.begin(); itlistDirs != listDirs.end(); itlistDirs++)
  {
    std::string strLcode;
    CPOHandler POHandler;
    strLcode = g_LCodeHandler.FindLangCode(*itlistDirs);
    m_mapPOFiles[strLcode] = POHandler;
  }
  return true;
};

void CResourceHandler::GetResTypeFromDir(std::string ResRootDir)
{
  if ((FileExist(ResRootDir + "addon.xml")) && (FileExist(ResRootDir + "resources" + DirSepChar + "language" +
    DirSepChar + "English" + DirSepChar + "strings.po")))
  {
    m_resType = ADDON;
  }
  else if ((FileExist(ResRootDir + "addon.xml")) && (FileExist(ResRootDir + "language" + DirSepChar + "English" +
    DirSepChar + "strings.po")))
  {
    m_resType = SKIN;
  }
  else if (FileExist(ResRootDir + "addon.xml"))
  {
    m_resType = ADDON_NOSTRINGS;
  }
  else if (FileExist(ResRootDir + "xbmc" + DirSepChar + "GUIInfoManager.h"))
  {
    m_resType = CORE;
  }
  else
  {
    m_resType = UNKNOWN;
    m_langDir = "";
    CLog::Log(logERROR, "ResHandler: GetResTypeFromDir: Type of resource can not be determined at rootdir: %s",
              ResRootDir.c_str());
  }
};


// Download from Transifex related functions

void CResourceHandler::CreateMissingDirs(std::string strResRootDir)
{
  if (m_resType == SKIN || m_resType == CORE)
  {
    if (!DirExists(strResRootDir + "language"))
    {
      CLog::Log(logINFO, "ResHandler: Creating language directory for new resource on Transifex");
      MakeDir(strResRootDir + "language");
    }
    m_langDir = strResRootDir + "language" + DirSepChar;
  }
  if (m_resType == ADDON || m_resType == ADDON_NOSTRINGS)
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
  if (m_resType == UNKNOWN)
    CLog::Log(logERROR, "ResHandler: Impossible to determine resource type on Transifex (addon, skin, xbmc-core)");

  return;
};

void CResourceHandler::GetResTypeFromTX(std::string category)
{
  m_resType = UNKNOWN;
  if (category == "xbmc-core")
    m_resType = CORE;
  else if (category == "skin")
    m_resType = SKIN;
  else if (category == "addon")
    m_resType = ADDON;
  else if (category == "addon_nostrings")
    m_resType = ADDON_NOSTRINGS;
  else
    CLog::Log(logERROR, "ResHandler: Impossible to determine resource type on Transifex (addon, skin, xbmc-core)");

  return;
};

bool CResourceHandler::FetchPOFilesTXToMem(std::string strURL, std::string strCategory)
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
    pPOHandler->FetchPOURLToMem(strURL + "translation/" + *it + "/?file");
    pPOHandler->SetIfIsEnglish(*it == "en");
    std::string strLang = *it;
    strLang.resize(20, ' ');
    CLog::Log(logINFO, "POHandler: %s\t\t%i\t\t%i\t\t%i", strLang.c_str(), pPOHandler->GetNumEntriesCount(),
              pPOHandler->GetClassEntriesCount(), pPOHandler->GetCommntEntriesCount());
  }
  GetResTypeFromTX(strCategory);
  return true;
}

bool CResourceHandler::FetchPOFilesUpstreamToMem(CXMLResdata XMLResdata, int resType)
{
  CLog::Log(logINFO, "ResHandler: Starting to load resource from Upsream URL: %s into memory",XMLResdata.strUptreamURL.c_str());

  std::list<std::string> listLangs;
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

  std::string strLangdirPrefix;

  m_resType = resType;
  switch (resType)
  {
    case ADDON :
      strLangdirPrefix = "resources/language/";
      break;
    case ADDON_NOSTRINGS :
      strLangdirPrefix = "";
      break;
    case SKIN :
      strLangdirPrefix = "language/";
      break;
    case CORE :
      strLangdirPrefix = "language/";
      break;
    default:
      CLog::Log(logINFO, "ResHandler: No resourcetype defined for upsream URL:",XMLResdata.strUptreamURL.c_str());
  }

  if (resType == CORE)
    m_AddonXMLHandler.FetchCoreVersionUpstr(XMLResdata.strUptreamURL + "xbmc/GUIInfoManager.h");
  else
    m_AddonXMLHandler.FetchAddonXMLFileUpstr(XMLResdata.strUptreamURL + "addon.xml");

  CPOHandler POHandler;

  for (std::list<std::string>::iterator it = listLangs.begin(); it != listLangs.end(); it++)
  {
    m_mapPOFiles[*it] = POHandler;
    CPOHandler * pPOHandler = &m_mapPOFiles[*it];
    pPOHandler->FetchPOURLToMem(XMLResdata.strUptreamURL + strLangdirPrefix + g_LCodeHandler.FindLang(*it) + DirSepChar + "strings.po");
    pPOHandler->SetIfIsEnglish(*it == "en");
    std::string strLang = *it;
    strLang.resize(20, ' ');
    CLog::Log(logINFO, "POHandler: %s\t\t%i\t\t%i\t\t%i", strLang.c_str(), pPOHandler->GetNumEntriesCount(),
              pPOHandler->GetClassEntriesCount(), pPOHandler->GetCommntEntriesCount());
  }
//  GetResTypeFromTX(strCategory);
  return true;
}



bool CResourceHandler::WritePOToFiles(std::string strResourceDir, std::string strPOsuffix, std::string strResname)
{
  CLog::Log(logINFO, "ResHandler: Starting to write resource from memory to directory: %s",strResourceDir.c_str());

  CreateMissingDirs(strResourceDir);
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
