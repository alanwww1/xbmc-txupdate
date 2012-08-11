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
#pragma once

#include "ResourceHandler.h"
#include "UpdateXMLHandler.h"
#include <list>

struct CresourceAvail
{
  bool bhasUpstream;
  bool bhasonTX;
  bool bhasLocal;
};

class CProjectHandler
{
public:
  CProjectHandler();
  ~CProjectHandler();
  bool LoadProject(std::string strProjRootDir);
  bool FetchResourcesFromTransifex(std::string strProjRootDir);
  bool FetchResourcesFromUpstream(std::string strProjRootDir);
  bool CreateMergedResources();
  bool WriteResourcesToFile(std::string strProjRootDir, std::string strPOSuffix);
  void InitUpdateXMLHandler(std::string strProjRootDir);
  void SaveUpdateXML(std::string strProjRootDir);

protected:
  bool GetResourcesFromDir(std::string strProjRootDir);
  const CPOEntry * SafeGetPOEntry(std::map<std::string, CResourceHandler> &mapResHandl, const std::string &strResname,
                            std::string &strLangCode, size_t numID);
  std::list<std::string> CreateLanguageList(std::string strResname);
  CResourceHandler * ChoosePrefResMap(std::string strResname);
  std::list<std::string> CreateResourceList();

  std::map<std::string, CResourceHandler> m_mapResourcesLocal;
  std::map<std::string, CResourceHandler> m_mapResourcesTX;
  std::map<std::string, CResourceHandler> m_mapResourcesUpstr;

  std::map<std::string, CResourceHandler> m_mapResMerged;
  std::map<std::string, CResourceHandler> m_mapResUpdateTX;
  typedef std::map<std::string, CResourceHandler>::iterator T_itmapRes;
  std::map<std::string, std::string> m_mapResourceNamesTX;
  int m_resCount;
  CUpdateXMLHandler m_UpdateXMLHandler;
};
