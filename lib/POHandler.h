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

#include "POUtils/POUtils.h"
#include <map>

class CPOHandler
{
public:
  CPOHandler();
  ~CPOHandler();
  bool LoadPOFile(std::string strDir, std::string strLang, std::string strPOuffix);
//  bool SavePOFile(std::string strDir, std::string strLang);
  bool FetchPOTXToMem(std::string strURL, std::string strLang);
  bool WritePOFile(const std::string &strDir, const std::string &strLang, std::map<std::string,
                   CAddonXMLEntry> &mapAddonXMLData, const std::string &strResData, const std::string &strPOuffix);
  bool LookforClassicEntry (CPOEntry &EntryToFind);
  void AddClassicEntry (CPOEntry &EntryToAdd);
  bool ModifyClassicEntry (CPOEntry &EntryToFind, CPOEntry EntryNewValue);
  bool DeleteClassicEntry (CPOEntry &EntryToFind);

protected:
  void ClearCPOEntry (CPOEntry &entry);
  bool ProcessPOFile(CPODocument &PODoc, std::string strLang);

  std::string m_strHeader;

  std::map<uint32_t, CPOEntry> m_mapStrings;
  typedef std::map<uint32_t, CPOEntry>::iterator itStrings;

  std::map<std::string, CAddonXMLEntry> m_mapAddoXMLEntries;
  typedef std::map<std::string, CAddonXMLEntry>::iterator itAddonXMLEntry;

  std::vector<CPOEntry> m_vecClassicEntries;
  typedef std::vector<CPOEntry>::iterator itClassicEntries;

};

