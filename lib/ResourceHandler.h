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
#include "AddonXMLHandler.h"
#include "UpdateXMLHandler.h"

typedef std::map<std::string, CPOHandler>::iterator T_itmapPOFiles;

class CResourceHandler
{
public:
  CResourceHandler();
  ~CResourceHandler();
  bool LoadResource(std::string strResRootDir, std::string strPOsuffix);
  bool FetchPOFilesTXToMem(std::string strURL, std::string strCategory);
  bool FetchPOFilesUpstreamToMem(CXMLResdata XMLResdata, int resType);
  bool WritePOToFiles(std::string strResourceDir, std::string strPOsuffix);
  int GetCurrResType() const {return m_resType;}
  size_t GetLangsCount() const {return m_mapPOFiles.size();}
  std::string GetLangCodeFromPos(size_t pos) {T_itmapPOFiles it = IterateToMapIndex (m_mapPOFiles.begin(), pos); return it->first;}
  CPOHandler* GetPOData(std::string strLang);
  void AddPOData(CPOHandler &POHandler, std::string strLang) {m_mapPOFiles[strLang] = POHandler;}
  std::string GetLangDir () const {return m_langDir;}
  void SetLangDir (std::string langDir) {m_langDir = langDir;}
  CAddonXMLHandler * GetXMLHandler () {return &m_AddonXMLHandler;}
  void SetXMLHandler (CAddonXMLHandler XMLHandler) {m_AddonXMLHandler = XMLHandler;}
  int GetResType () const {return m_resType;}
  void SetResType (int resType) {m_resType = resType;}

protected:
  bool GetLangsFromDir(std::string strLangDir);
  void CreateMissingDirs (std::string strRootDir);
  void GetResTypeFromDir(std::string ResRootDir);
  void GetResTypeFromTX(std::string category);
  T_itmapPOFiles IterateToMapIndex(T_itmapPOFiles it, size_t index);

  std::map<std::string, CPOHandler> m_mapPOFiles;
  std::string m_langDir;
  CAddonXMLHandler m_AddonXMLHandler;
  int m_resType;
//  std::string m_strTXCategory;
};
