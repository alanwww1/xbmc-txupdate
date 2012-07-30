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
  bool LoadPOFile(std::string strPOFileName);
  bool FetchPOURLToMem(std::string strURL);
  bool WritePOFile(const std::string &strOutputPOFilename);
  bool LookforClassicEntry (CPOEntry &EntryToFind);
  void AddClassicEntry (CPOEntry &EntryToAdd);
  bool ModifyClassicEntry (CPOEntry &EntryToFind, CPOEntry EntryNewValue);
  bool DeleteClassicEntry (CPOEntry &EntryToFind);

  CPOEntry GetNumPOEntry(uint32_t numid) const {return m_mapStrings[numid];}

  void SetAddonMetaData (CAddonXMLEntry AddonXMLEntry, CAddonXMLEntry AddonXMLEntryEN);
  void SetHeader (std::string strPreText);
  size_t const GetNumEntriesCount() {return m_mapStrings.size();}
  size_t const GetClassEntriesCount() {return m_vecClassicEntries.size();}
  size_t const GetCommntEntriesCount() {return m_CommsCntr;}
  void SetIfIsEnglish(bool bIsENLang) {m_bPOIsEnglish = bIsENLang;}

protected:
  void ClearCPOEntry (CPOEntry &entry);
  bool ProcessPOFile(CPODocument &PODoc);

  std::string m_strHeader;

  std::map<uint32_t, CPOEntry> m_mapStrings;
  typedef std::map<uint32_t, CPOEntry>::iterator itStrings;

  std::vector<CPOEntry> m_vecClassicEntries;
  typedef std::vector<CPOEntry>::iterator itClassicEntries;

  size_t m_CommsCntr;
  bool m_bPOIsEnglish;
};

