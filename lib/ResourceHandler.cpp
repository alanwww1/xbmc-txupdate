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

  for (itmapPOFiles = m_mapPOFiles.begin(); itmapPOFiles != m_mapPOFiles.end(); itmapPOFiles++)
  {
    itmapPOFiles->second.LoadPOFile(m_langDir + g_LCodeHandler.FindLang(itmapPOFiles->first) + DirSepChar + "strings.po"
                                    + strPOsuffix);
    itmapPOFiles->second.SetIfIsEnglish(itmapPOFiles->first == "en");
//    CLog::Log(logDEBUG, "ResHandler: Loaaded language file for: %s", itmapPOFiles->first.c_str());
  };

  CLog::Log(logINFO, "ResHandler: Found and loaded %i languages.", m_mapPOFiles.size());
  return true;
};

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
 /*
bool CResourceHandler::CreateMissingDirs (std::string strRootDir)
{
  if (!DirExists(strRootDir + "resources"))
  {
    if ((!MakeDir(strRootDir + "resources")))
      CLog::Log(logINFO, "ResHandler: Created missing resources directory at dir: %s", strRootDir.c_str());
    else
    {
      CLog::Log(logERROR, "ResHandler: Not able to create resources directory at dir: %s", strRootDir.c_str());
      return 1;
    }
  }

  if (!DirExists(strRootDir + "resources" + DirSepChar + "language"))
  {
    if (!MakeDir(strRootDir + "resources"+ DirSepChar + "language"))
      CLog::Log(logINFO, "ResHandler: Created missing language directory in dir: %s", (strRootDir + "resources").c_str());
    else
    {
      CLog::Log(logERROR, "ResHandler: Not able to create language directory in dir: %s", (strRootDir + "resources").c_str());
      return 1;
    }
  }

  std::string WorkingDir = strRootDir + "resources"+ DirSepChar + "language" + DirSepChar;
  for (itAddonXMLData = m_mapAddonXMLData.begin(); itAddonXMLData != m_mapAddonXMLData.end(); itAddonXMLData++)
  {
    if (!DirExists(WorkingDir + FindLang(itAddonXMLData->first)) && (!MakeDir(WorkingDir +
      FindLang(itAddonXMLData->first))))
    {
      CLog::Log(logERROR, "ResHandler: Not able to create %s language directory at dir: %s", itAddonXMLData->first.c_str(),
              WorkingDir.c_str());
      return 1;
    }
  }
  return true;
};

*/

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
  CLog::Log(logINFO, "ResHandler: Starting to load resource from URL: %s into memory",strURL.c_str());

  std::string strtemp = g_HTTPHandler.GetURLToSTR(strURL + "stats/");
//  printf("%s, strlength: %i", strtemp.c_str(), strtemp.size());

  char cstrtemp[strtemp.size()];
  strcpy(cstrtemp, strtemp.c_str());

  CJSONHandler JSONHandler;
  std::list<std::string> listLangsTX = JSONHandler.ParseAvailLanguages(strtemp);

  CPOHandler POHandler;

  for (std::list<std::string>::iterator it = listLangsTX.begin(); it != listLangsTX.end(); it++)
  {
//    std::string strLangdir = GetResTypeFromTX(strResRootDir, category);

    m_mapPOFiles[*it] = POHandler;
    CPOHandler * pPOHandler = &m_mapPOFiles[*it];
    pPOHandler->FetchPOTXToMem(strURL + "translation/" + *it + "/?file");
    pPOHandler->SetIfIsEnglish(*it == "en");
    std::string strLang = *it;
    strLang.resize(20, ' ');
    CLog::Log(logINFO, "POHandler: %s\t\t%i\t\t%i\t\t%i", strLang.c_str(), pPOHandler->GetSizeNumEntries(),
              pPOHandler->GetSizeClassEntries(), pPOHandler->GetSizeCommEntries());
  }
  GetResTypeFromTX(strCategory);
  return true;
}

bool CResourceHandler::WritePOToFiles(std::string strResourceDir, std::string strPOsuffix)
{
  CLog::Log(logINFO, "ResHandler: Starting to write resource from memory to directory: %s",strResourceDir.c_str());

  CreateMissingDirs(strResourceDir);
  std::string strListNewDirs;

  for (itmapPOFiles = m_mapPOFiles.begin(); itmapPOFiles != m_mapPOFiles.end(); itmapPOFiles++)
  {
    if (itmapPOFiles->first == "en")
      continue;
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
    CLog::Log(logINFO, "POHandler: %s\t\t%i\t\t%i\t\t%i", strLang.c_str(), pPOHandler->GetSizeNumEntries(),
              pPOHandler->GetSizeClassEntries(), pPOHandler->GetSizeCommEntries());
  }
  if (!strListNewDirs.empty())
    CLog::Log(logINFO, "POHandler: New local directories already existing on Transifex: %s", strListNewDirs.c_str());
  return true;
}
