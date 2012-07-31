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

#include "ProjectHandler.h"
#include <list>
#include "HTTPUtils.h"
#include "JSONHandler.h"
#include "Settings.h"

CProjectHandler::CProjectHandler()
{};

CProjectHandler::~CProjectHandler()
{};

bool CProjectHandler::LoadProject(std::string strProjRootDir)
{
  GetResourcesFromDir(strProjRootDir);
  int loadCounter = 0;

  for (T_itmapRes itmapResources = m_mapResourcesLocal.begin(); itmapResources != m_mapResourcesLocal.end(); itmapResources++)
  {
    CLog::Log(logLINEFEED, "");
    CLog::Log(logINFO, "ProjHandler: *** Resource: %s ***", itmapResources->first.c_str());
    if (itmapResources->second.LoadResource(strProjRootDir + itmapResources->first + DirSepChar, ""))
      loadCounter++;
  };

  CLog::Log(logLINEFEED, "");
  CLog::Log(logINFO, "ProjHandler: Loaded %i resources from root dir: %s", loadCounter, strProjRootDir.c_str());

  return true;
};

bool CProjectHandler::GetResourcesFromDir(std::string strProjRootDir)
{
  std::list<std::string> listDirs;
  std::list<std::string>::iterator itlistDirs;
  DIR* Dir;
  struct dirent *DirEntry;
  Dir = opendir(strProjRootDir.c_str());

  while((DirEntry=readdir(Dir)))
  {
    if (DirEntry->d_type == DT_DIR && DirEntry->d_name[0] != '.')
      listDirs.push_back(DirEntry->d_name);
  }
  listDirs.sort();

  int resCounter = 0;
  for (itlistDirs = listDirs.begin(); itlistDirs != listDirs.end(); itlistDirs++)
  {
    resCounter++;
    CResourceHandler ResourceHandler;
    m_mapResourcesLocal[*itlistDirs] = ResourceHandler;
  }

  CLog::Log(logINFO, "ProjHandler: Found %i resources at root dir: %s", resCounter, strProjRootDir.c_str());

  return true;
};

bool CProjectHandler::FetchResourcesFromTransifex(std::string strProjRootDir)
{

  std::string strtemp = g_HTTPHandler.GetURLToSTR("https://www.transifex.com/api/2/project/" + g_Settings.GetProjectname()
                                                  + "/resources/");

  char cstrtemp[strtemp.size()];
  strcpy(cstrtemp, strtemp.c_str());

  CJSONHandler JSONHandler;
  m_mapResourceNamesTX.clear();
  m_mapResourceNamesTX = JSONHandler.ParseResources(strtemp);

  CResourceHandler ResourceHandler;

  for (std::map<std::string, std::string>::iterator it = m_mapResourceNamesTX.begin(); it != m_mapResourceNamesTX.end(); it++)
  {
    CLog::Log(logLINEFEED, "");
    CLog::Log(logINFO, "ProjHandler: *** Fetch Resource: %s ***", it->first.c_str());
    if (!DirExists(strProjRootDir + it->first))
    {
      CLog::Log(logERROR, "ProjHandler: Creating local directory for new resource on Transifex: %s", it->first.c_str());
      MakeDir(strProjRootDir + it->first);
    }
    m_UpdateXMLHandler.AddResourceToXMLFile(it->first);
    m_mapResourcesTX[it->first] = ResourceHandler;
    m_mapResourcesTX[it->first].FetchPOFilesTXToMem("https://www.transifex.com/api/2/project/" + g_Settings.GetProjectname()
                                              + "/resource/" + it->first + "/", it->second);
  }
  return true;
};

bool CProjectHandler::FetchResourcesFromUpstream(std::string strProjRootDir)
{

  std::map<std::string, CXMLResdata> mapRes = m_UpdateXMLHandler.GetResMap();
  std::map<std::string, CXMLResdata> mapResWithUpstream;

  for (std::map<std::string, CXMLResdata>::iterator it = mapRes.begin(); it != mapRes.end(); it++)
  {
    if (!it->second.strUptreamURL.empty())
      mapResWithUpstream[it->first] = it->second;
  }

  CResourceHandler ResourceHandler;

  for (std::map<std::string, CXMLResdata>::iterator it = mapResWithUpstream.begin(); it != mapResWithUpstream.end(); it++)
  {
    CLog::Log(logLINEFEED, "");
    CLog::Log(logINFO, "ProjHandler: *** Fetch Resource from upstream: %s ***", it->first.c_str());
    if (!DirExists(strProjRootDir + it->first))
    {
      CLog::Log(logERROR, "ProjHandler: Creating local directory for new resource in update.xml: %s", it->first.c_str());
      MakeDir(strProjRootDir + it->first);
    }
    m_mapResourcesUpstr[it->first] = ResourceHandler;
    m_mapResourcesUpstr[it->first].FetchPOFilesUpstreamToMem(it->second, m_mapResourcesLocal[it->first].GetCurrResType());
  }
  return true;
};

bool CProjectHandler::WriteResourcesToFile(std::string strProjRootDir, std::string strPOSuffix)
{
  for (T_itmapRes itmapResources = m_mapResourcesTX.begin(); itmapResources != m_mapResourcesTX.end(); itmapResources++)
  {
    CLog::Log(logLINEFEED, "");
    CLog::Log(logINFO, "ProjHandler: *** Write Resource: %s ***", itmapResources->first.c_str());
    if (!DirExists(strProjRootDir + itmapResources->first))
    {
      CLog::Log(logERROR, "ProjHandler: Creating local directory for new resource on Transifex: %s", itmapResources->first.c_str());
      MakeDir(strProjRootDir + itmapResources->first + DirSepChar);
    }
    m_mapResourcesTX[itmapResources->first].WritePOToFiles (strProjRootDir + itmapResources->first + DirSepChar, strPOSuffix);
  }
  return true;
};

bool CProjectHandler::CreateMergedResources()
{

  std::map<std::string, CresourceAvail> mapResAvail;
  CresourceAvail currResAvail, emptyResAvail;

  emptyResAvail.bhasLocal = false;
  emptyResAvail.bhasonTX = false;
  emptyResAvail.bhasUpstream = false;

  for (T_itmapRes it = m_mapResourcesUpstr.begin(); it != m_mapResourcesUpstr.end(); it++)
  {
    if (mapResAvail.find(it->first) == mapResAvail.end())
      mapResAvail[it->first] = emptyResAvail;
    mapResAvail[it->first].bhasUpstream = true;
  }

  for (T_itmapRes it = m_mapResourcesTX.begin(); it != m_mapResourcesTX.end(); it++)
  {
    if (mapResAvail.find(it->first) == mapResAvail.end())
      mapResAvail[it->first] = emptyResAvail;
    mapResAvail[it->first].bhasonTX = true;
  }

  for (T_itmapRes it = m_mapResourcesLocal.begin(); it != m_mapResourcesLocal.end(); it++)
  {
    if (mapResAvail.find(it->first) == mapResAvail.end())
      mapResAvail[it->first] = emptyResAvail;
    mapResAvail[it->first].bhasLocal = true;
  }

  std::map<std::string, CResourceHandler> * pmapRes;

  for (std::map<std::string, CresourceAvail>::iterator itResAvail = mapResAvail.begin(); itResAvail != mapResAvail.end(); itResAvail++)
  {
    printf("*******************************\n");
    if (itResAvail->second.bhasUpstream)
      pmapRes = &m_mapResourcesUpstr;
    else if (itResAvail->second.bhasonTX)
      pmapRes = &m_mapResourcesTX;
    else
      pmapRes = &m_mapResourcesLocal;
    CResourceHandler currResHandler = (*pmapRes)[itResAvail->first];

    for (size_t LangsIdx = 0; LangsIdx != currResHandler.GetLangsCount(); LangsIdx++)
    {
      for (size_t POEntryIdx = 0; POEntryIdx != currResHandler.GetPOData("en").GetNumEntriesCount(); POEntryIdx++)
      {
        printf("po:%s\n", currResHandler.GetPOData("en").GetNumPOEntryByIdx(POEntryIdx).msgID.c_str());
      }
    }
  }

}


void CProjectHandler::InitUpdateXMLHandler(std::string strProjRootDir)
{
m_UpdateXMLHandler.LoadXMLToMem(strProjRootDir);
}

void CProjectHandler::SaveUpdateXML(std::string strProjRootDir)
{
m_UpdateXMLHandler.SaveMemToXML(strProjRootDir);
}
