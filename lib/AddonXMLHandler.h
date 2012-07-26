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

#include "POHandler.h"
#include "TinyXML/tinyxml.h"

class CAddonXMLHandler
{
public:
  CAddonXMLHandler();
  ~CAddonXMLHandler();
  bool LoadAddonXMLFile (std::string AddonXMLFilename);
  bool FetchAddonXMLFileUpstr (std::string strURL);
  bool LoadCoreVersion(std::string filename);
  bool FetchCoreVersionUpstr(std::string strURL);

protected:
  bool ProcessAddonXMLFile (std::string AddonXMLFilename, TiXmlDocument &xmlAddonXML);
  bool ProcessCoreVersion(std::string filename, std::string &strBuffer);
  bool GetEncoding(const TiXmlDocument* pDoc, std::string& strEncoding);
  std::string EscapeLF(const char * StrToEscape);
  std::map<std::string, CAddonXMLEntry> m_mapAddonXMLData;
  std::map<std::string, CAddonXMLEntry>::iterator itAddonXMLData;
  std::string m_strResourceData;
};
