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

#include "TinyXML/tinyxml.h"
#include "POUtils/POUtils.h"
#include <string>

struct CXMLResdata
{
  std::string strUptreamURL;
  std::string strLangsFromUpstream;
};

class CUpdateXMLHandler
{
public:
  CUpdateXMLHandler();
  ~CUpdateXMLHandler();
  bool LoadXMLToMem(std::string updateXMLFilename);
  void GetResourcesFromDir(std::string strProjRootDir);
  void SaveMemToXML(std::string UpdateXMLFilename);
private:
  int GetResType(std::string ResRootDir);
  std::string m_ProjDir;
  std::map<std::string, CXMLResdata> m_mapXMLResdata;
  std::map<std::string, CXMLResdata>::iterator itXMLResdata;
};