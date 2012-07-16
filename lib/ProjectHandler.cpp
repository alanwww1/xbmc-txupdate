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
#include "UpdateXMLHandler.h"
#include "HTTPUtils.h"
#include "JSONHandler.h"
#include "Settings.h"

CProjectHandler::CProjectHandler()
{};

CProjectHandler::~CProjectHandler()
{};

bool CProjectHandler::LoadProject(std::string strProjRootDir)
{
//  CUpdateXMLHandler UpdateXMLHandler;
//  UpdateXMLHandler.LoadXMLToMem(strProjRootDir);

  GetResourcesFromDir(strProjRootDir);
  int loadCounter = 0;

  for (itmapResources = m_mapResources.begin(); itmapResources != m_mapResources.end(); itmapResources++)
  {
    CLog::Log(logLINEFEED, "");
    CLog::Log(logINFO, "ProjHandler: *** Resource: %s ***", itmapResources->first.c_str());
    if (itmapResources->second.LoadResource(strProjRootDir + itmapResources->first + DirSepChar, ""))
      loadCounter++;
  };

  CLog::Log(logLINEFEED, "");
  CLog::Log(logINFO, "ProjHandler: Loaded %i resources from root dir: %s", loadCounter, strProjRootDir.c_str());

//  UpdateXMLHandler.SaveMemToXML(strProjRootDir);

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
    m_mapResources[*itlistDirs] = ResourceHandler;
  }

  CLog::Log(logINFO, "ProjHandler: Found %i resources at root dir: %s", resCounter, strProjRootDir.c_str());

  return true;
};

bool CProjectHandler::FetchResourcesFromTransifex(std::string strProjRootDir)
{
  CUpdateXMLHandler UpdateXMLHandler;
  UpdateXMLHandler.LoadXMLToMem(strProjRootDir);

  std::string strtemp = g_HTTPHandler.GetURLToSTR("https://www.transifex.com/api/2/project/" + g_Settings.GetProjectname()
                                                  + "/resources/");
  printf("%s, strlength: %i", strtemp.c_str(), strtemp.size());

  char cstrtemp[strtemp.size()];
  strcpy(cstrtemp, strtemp.c_str());

  CJSONHandler JSONHandler;
  std::map<std::string, std::string> mapResourcesTX;
  mapResourcesTX = JSONHandler.ParseResources(strtemp);

  CResourceHandler ResourceHandler;

  for (std::map<std::string, std::string>::iterator it = mapResourcesTX.begin(); it != mapResourcesTX.end(); it++)
  {
    if (!DirExists(strProjRootDir + it->first))
    {
      CLog::Log(logERROR, "ProjHandler: Creating local directory for new resource on Transifex: %s", it->first.c_str());
      MakeDir(strProjRootDir + it->first);
    } 
    m_mapResources[it->first] = ResourceHandler;
    m_mapResources[it->first].FetchPOFilesTXToMem("https://www.transifex.com/api/2/project/" + g_Settings.GetProjectname()
                                              + "/resource/" + it->first + "/");
  }
  return true;

};

/*
if (!DirExists(strProjRootDir + it->first))
  MakeDir(strProjRootDir + it->first);
if (it->second == "skin" && !DirExists(strProjRootDir + it->first + DirSepChar + "language"))
  MakeDir(strProjRootDir + it->first + DirSepChar + "language");
if (it->second == "xbmc-core" && !DirExists(strProjRootDir + it->first + DirSepChar + "language"))
  MakeDir(strProjRootDir + it->first + DirSepChar + "language");
if (it->second == "addon" && !DirExists(strProjRootDir + it->first + DirSepChar + "resources"))
  MakeDir(strProjRootDir + it->first + DirSepChar + "resources");
if (it->second == "addon" && !DirExists(strProjRootDir + it->first + DirSepChar + "resources" + DirSepChar + "language"))
  MakeDir(strProjRootDir + it->first + DirSepChar + "resources" + DirSepChar + "language");
*/