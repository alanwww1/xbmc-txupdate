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

#include "POUtils/POUtils.h"

enum
{
  SKIN = 0,
  ADDON = 1,
  CORE = 2,
  ADDON_NOSTRINGS = 3,
  UNKNOWN = 4
};

struct CAddonXMLEntry
{
  std::string strSummary;
  std::string strDescription;
  std::string strDisclaimer;
};

struct CResData
{
  std::string ResName;
  std::string ResVersion;
  std::string ResTextName;
  std::string ResProvider;
};

class CPOHandler
{
public:
  CPOHandler();
  ~CPOHandler();
  bool LoadPOFile(std::string strDir, std::string strLang);
  bool SavePOFile(std::string strDir, std::string strLang);

protected:
  void ClearCPOEntry (CPOEntry &entry);

  std::string m_strHeader;

  std::map<uint32_t, CPOEntry> m_mapStrings;
  typedef std::map<uint32_t, CPOEntry>::iterator itStrings;

  std::vector<CPOEntry> m_vecClassicEntries;
  typedef std::vector<CPOEntry>::iterator itClassicEntries;

};

