/*
 *      Copyright (C) 2014 Team Kodi
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
 *  along with Kodi; see the file COPYING.  If not, write to
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
#include "Settings.h"

using namespace std;

CResourceHandler::CResourceHandler()
{};

CResourceHandler::~CResourceHandler()
{};

CPOHandler* CResourceHandler::GetPOData(std::string strLang)
{
  if (m_mapPOFiles.empty())
    return NULL;
  if (m_mapPOFiles.find(strLang) != m_mapPOFiles.end())
    return &m_mapPOFiles[strLang];
  return NULL;
}

// Download from Transifex related functions

bool CResourceHandler::FetchPOFilesTXToMem(std::string strURL, bool bIsXBMCCore)
{
  g_HTTPHandler.Cleanup();
  g_HTTPHandler.ReInit();
  CLog::Log(logINFO, "ResHandler: Starting to load resource from TX URL: %s into memory",strURL.c_str());
  printf(" Langlist");

  std::string strtemp = g_HTTPHandler.GetURLToSTR(strURL + "stats/");
  if (strtemp.empty())
    CLog::Log(logERROR, "ResHandler::FetchPOFilesTXToMem: error getting po file list from transifex.net");

  char cstrtemp[strtemp.size()];
  strcpy(cstrtemp, strtemp.c_str());

  std::list<std::string> listLangsTX = g_Json.ParseAvailLanguagesTX(strtemp, bIsXBMCCore, strURL);

  CPOHandler POHandler;

  for (std::list<std::string>::iterator it = listLangsTX.begin(); it != listLangsTX.end(); it++)
  {
    printf (" %s", it->c_str());
    m_mapPOFiles[*it] = POHandler;
    CPOHandler * pPOHandler = &m_mapPOFiles[*it];
    pPOHandler->FetchPOURLToMem(strURL + "translation/" + *it + "/?file", false);
    pPOHandler->SetIfIsEnglish(*it == "en");
    std::string strLang = *it;
    CLog::LogTable(logINFO, "txfetch", "\t\t\t%s\t\t%i\t\t%i", strLang.c_str(), pPOHandler->GetNumEntriesCount(),
                            pPOHandler->GetClassEntriesCount());
  }
  CLog::LogTable(logADDTABLEHEADER, "txfetch", "--------------------------------------------------------------\n");
  CLog::LogTable(logADDTABLEHEADER, "txfetch", "FetchPOFilesTX:\tLang\t\tIDEntry\t\tClassEntry\n");
  CLog::LogTable(logADDTABLEHEADER, "txfetch", "--------------------------------------------------------------\n");
  CLog::LogTable(logCLOSETABLE, "txfetch", "");
  return true;
}

bool CResourceHandler::FetchPOFilesUpstreamToMem(CXMLResdata XMLResdata, std::list<std::string> listLangsTX)
{
  g_HTTPHandler.Cleanup();
  g_HTTPHandler.ReInit();
  CLog::Log(logINFO, "ResHandler: Starting to load resource from Upsream URL: %s into memory",XMLResdata.strUpstreamURL.c_str());

  std::string strLangdirPrefix, strGitHubURL, strtemp;

  if (XMLResdata.Restype == CORE)
  {
    m_AddonXMLHandler.FetchCoreVersionUpstr(XMLResdata.strUpstreamURL + "xbmc/GUIInfoManager.h");
    strLangdirPrefix = "language/";
  }
  else
  {
    // We get the version of the addon.xml and changelog.txt files here
    if (XMLResdata.strUpstreamURL.find(".github") != std::string::npos)  //if URL is github, we download a directory tree to get SHA versions
    {
      strGitHubURL = g_HTTPHandler.GetGitHUBAPIURL(XMLResdata.strUpstreamURL, "");
      printf(" Dir");
      strtemp = g_HTTPHandler.GetURLToSTR(strGitHubURL);
      if (strtemp.empty())
        CLog::Log(logERROR, "ResHandler::FetchPOFilesUpstreamToMem: error getting addon.xml file version from github.com");

      g_Json.ParseAddonXMLVersionGITHUB(strtemp, XMLResdata.strUpstreamURL);
    }

    printf(" Addxml");
    m_AddonXMLHandler.FetchAddonXMLFileUpstr(XMLResdata.strUpstreamURL + "addon.xml" + XMLResdata.strAddonXMLSuffix + XMLResdata.strURLSuffix);
    if (XMLResdata.bHasChangelog)
    {
      printf(" Chlog");
      m_AddonXMLHandler.FetchAddonChangelogFile(XMLResdata.strUpstreamURL + XMLResdata.strLogFilename + XMLResdata.strURLSuffix);
    }
    if (XMLResdata.Restype == SKIN)
      strLangdirPrefix = "language/";
    else if (XMLResdata.Restype == ADDON)
      strLangdirPrefix = "resources/language/";
    else if (XMLResdata.Restype == ADDON_NOSTRINGS)
      return true;
  }

  std::list<std::string> listLangs, listGithubLangs;

  if (XMLResdata.strUpstreamURL.find(".github") != std::string::npos)  //if URL is github, we download a directory tree to get SHA versions
  {
    printf(" Langlist");
    strtemp.clear();
    strGitHubURL.clear();
    strGitHubURL = g_HTTPHandler.GetGitHUBAPIURL(XMLResdata.strUpstreamURL, (XMLResdata.Restype == SKIN || XMLResdata.Restype == CORE)?
                                                             "/language":"/resources/language");

    strtemp = g_HTTPHandler.GetURLToSTR(strGitHubURL);
    if (strtemp.empty())
      CLog::Log(logERROR, "ResHandler::FetchPOFilesUpstreamToMem: error getting po file list from github.com");

    listGithubLangs = g_Json.ParseAvailLanguagesGITHUB(strtemp, XMLResdata.strUpstreamURL + strLangdirPrefix, XMLResdata.strLangFileType != "xml");
  }


  if (XMLResdata.strLangsFromUpstream == "tx_all")
  {
    CLog::Log(logINFO, "ResHandler: using language list previously got from Transifex");
    listLangs = listLangsTX;
  }
  else if (XMLResdata.strLangsFromUpstream == "github_all" || XMLResdata.strUpstreamURL.find("github") != std::string::npos)
  {
    CLog::Log(logINFO, "ResHandler: using language list dwonloaded with github API");
    listLangs=listGithubLangs;
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

  bool bResult;

  for (std::list<std::string>::iterator it = listLangs.begin(); it != listLangs.end(); it++)
  {
    CPOHandler POHandler;
    POHandler.SetIfIsEnglish(*it == "en");
    printf (" %s", it->c_str());

    if (XMLResdata.strLangFileType == "xml")
      bResult = POHandler.FetchXMLURLToMem(XMLResdata.strUpstreamURL + strLangdirPrefix + g_LCodeHandler.FindLang(*it) + DirSepChar + "strings.xml" + XMLResdata.strURLSuffix);
    else
      bResult = POHandler.FetchPOURLToMem(XMLResdata.strUpstreamURL + strLangdirPrefix + g_LCodeHandler.FindLang(*it) + DirSepChar + "strings.po" + XMLResdata.strURLSuffix,false);
    if (bResult)
    {
      m_mapPOFiles[*it] = POHandler;
      std::string strLang = *it;
      CLog::LogTable(logINFO, "upstrFetch", "\t\t\t%s\t\t%i\t\t%i\t\t%i", strLang.c_str(), POHandler.GetNumEntriesCount(),
              POHandler.GetClassEntriesCount(), POHandler.GetCommntEntriesCount());
    }
  }
  CLog::LogTable(logADDTABLEHEADER, "upstrFetch", "-----------------------------------------------------------------------------\n");
  CLog::LogTable(logADDTABLEHEADER, "upstrFetch", "FetchPOFilesUpstr:\tLang\t\tIDEntry\t\tClassEntry\tCommEntry\n");
  CLog::LogTable(logADDTABLEHEADER, "upstrFetch", "-----------------------------------------------------------------------------\n");
  CLog::LogTable(logCLOSETABLE, "upstrFetch", "");
  return true;
}

bool CResourceHandler::WritePOToFiles(std::string strProjRootDir, std::string strPrefixDir, std::string strResname, CXMLResdata XMLResdata, bool bTXUpdFile)
{
  std::string strResourceDir, strLangDir;
  switch (XMLResdata.Restype)
  {
    case ADDON: case ADDON_NOSTRINGS:
      strResourceDir = strProjRootDir + strPrefixDir + DirSepChar + XMLResdata.strResDirectory + DirSepChar + strResname + DirSepChar + XMLResdata.strDIRprefix + DirSepChar;
      strLangDir = strResourceDir + "resources" + DirSepChar + "language" + DirSepChar;
      break;
    case SKIN:
      strResourceDir = strProjRootDir + strPrefixDir + DirSepChar  + XMLResdata.strResDirectory + DirSepChar + strResname + DirSepChar+ XMLResdata.strDIRprefix + DirSepChar;
      strLangDir = strResourceDir + "language" + DirSepChar;
      break;
    case CORE:
      strResourceDir = strProjRootDir + strPrefixDir + DirSepChar + XMLResdata.strResDirectory + DirSepChar + XMLResdata.strDIRprefix + DirSepChar;
      strLangDir = strResourceDir + "language" + DirSepChar;
      break;
    default:
      CLog::Log(logERROR, "ResHandler: No resourcetype defined for resource: %s",strResname.c_str());
  }

  if (bTXUpdFile && !m_mapPOFiles.empty())
    printf("Languages to update from upstream to upload to Transifex:");
  size_t counter = 0;

  for (T_itmapPOFiles itmapPOFiles = m_mapPOFiles.begin(); itmapPOFiles != m_mapPOFiles.end(); itmapPOFiles++)
  {
    std::string strLang = g_LCodeHandler.FindLang(itmapPOFiles->first);
    std::string strPODir = strLangDir + strLang;

    if (bTXUpdFile && counter < 20)
      printf (" %s", itmapPOFiles->first.c_str());
    if ((bTXUpdFile && counter == 19) && m_mapPOFiles.size() != 20)
      printf ("+%i Langs", m_mapPOFiles.size()-19);

    CPOHandler * pPOHandler = &m_mapPOFiles[itmapPOFiles->first];
    if (XMLResdata.bWritePO || bTXUpdFile)
      pPOHandler->WritePOFile(strPODir + DirSepChar + "strings.po");

    if (XMLResdata.bWriteXML && !bTXUpdFile)
      pPOHandler->WriteXMLFile(strPODir + DirSepChar + "strings.xml");

    CLog::LogTable(logINFO, "writepo", "\t\t\t%s\t\t%i\t\t%i", itmapPOFiles->first.c_str(), pPOHandler->GetNumEntriesCount(),
              pPOHandler->GetClassEntriesCount());
  }
  if (bTXUpdFile && !m_mapPOFiles.empty())
    printf("\n");

  CLog::LogTable(logADDTABLEHEADER, "writepo", "--------------------------------------------------------------\n");
  CLog::LogTable(logADDTABLEHEADER, "writepo", "WritePOFiles:\tLang\t\tIDEntry\t\tClassEntry\n");
  CLog::LogTable(logADDTABLEHEADER, "writepo", "--------------------------------------------------------------\n");
  CLog::LogTable(logCLOSETABLE, "writepo", "");

  // update local addon.xml file
  if (strResname != "xbmc.core" && strPrefixDir == g_Settings.GetMergedLangfilesDir())
  {
    bool bResChangedFromUpstream = !m_lChangedLangsFromUpstream.empty() || !m_lChangedLangsInAddXMLFromUpstream.empty();
    m_AddonXMLHandler.UpdateAddonXMLFile(strResourceDir + "addon.xml" + XMLResdata.strAddonXMLSuffix, bResChangedFromUpstream);
    if (XMLResdata.bHasChangelog)
      m_AddonXMLHandler.UpdateAddonChangelogFile(strResourceDir + XMLResdata.strLogFilename, XMLResdata.strLogFormat, bResChangedFromUpstream);
  }

  return true;
}

T_itmapPOFiles CResourceHandler::IterateToMapIndex(T_itmapPOFiles it, size_t index)
{
  for (size_t i = 0; i != index; i++) it++;
  return it;
}
