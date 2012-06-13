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

#include "FileUtils/FileUtils.h"
#include "POHandler.h"
#include "CharsetUtils/CharsetUtils.h"

bool CPOHandler::LoadPOFile(std::string strDir, std::string strLang)
{
  CPODocument PODoc;
  if (!PODoc.LoadFile(strDir + DirSepChar + strLang + DirSepChar + "strings.po"))
    return false;
  if (PODoc.GetEntryType() != HEADER_FOUND)
    printf ("POParser: No valid header found for this language");

  m_strHeader = PODoc.GetEntryData().Content.substr(1);

  m_mapStrings.clear();
  m_vecClassicEntries.clear();

  bool bMultipleComment = false;
  std::vector<std::string> vecILCommnts;
  CPOEntry currEntry;
  int currType = UNKNOWN_FOUND;

  while ((PODoc.GetNextEntry()))
  {
    PODoc.ParseEntry();
    currEntry = PODoc.GetEntryData();
    currType = PODoc.GetEntryType();

    if (currType == COMMENT_ENTRY_FOUND)
    {
      if (!vecILCommnts.empty())
        bMultipleComment = true;
      vecILCommnts = currEntry.interlineComm;
      continue;
    }

    if (currType == ID_FOUND || currType == MSGID_FOUND || currType == MSGID_PLURAL_FOUND)
    {
      if (bMultipleComment)
        printf("POParser: multiple comment entries found. Using only the last one "
        "before the real entry. Entry after comments: %s", currEntry.Content.c_str());
      if (!currEntry.interlineComm.empty())
        printf("POParser: interline comments (eg. #comment) is not alowed inside "
        "a real po entry. Cleaned it. Problematic entry: %s", currEntry.Content.c_str());
      currEntry.interlineComm = vecILCommnts;
      bMultipleComment = false;
      vecILCommnts.clear();
      if (currType == ID_FOUND)
        m_mapStrings[currEntry.numID] = currEntry;
      else
        m_vecClassicEntries.push_back(currEntry);

      ClearCPOEntry(currEntry);
    }
  }

  return true;
};

void CPOHandler::ClearCPOEntry (CPOEntry &entry)
{
  entry.msgStrPlural.clear();
  entry.referenceComm.clear();
  entry.extractedComm.clear();
  entry.translatorComm.clear();
  entry.interlineComm.clear();
  entry.numID = 0;
  entry.msgID.clear();
  entry.msgStr.clear();
  entry.msgIDPlur.clear();
  entry.msgCtxt.clear();
  entry.Type = UNKNOWN_FOUND;
};


bool CPOHandler::WritePOFile(const std::string &strDir, const std::string &strLang, const int resType,
                             std::map<std::string, CAddonXMLEntry> &mapAddonXMLData,
                             const std::string &strResData)
{
  std::string OutputPOFilename = strDir + DirSepChar + strLang + DirSepChar + "strings.po.temp";

  CPODocument PODoc;
  PODoc.WriteHeader(strResData, m_strHeader);

  std::string LCode = PODoc.GetLangCode();
  bool bIsSource = LCode == "en";

  if (!(mapAddonXMLData["en"].strSummary).empty())
  {
    CPOEntry POEntry;
    POEntry.Type = MSGID_FOUND;
    POEntry.msgCtxt = "Addon Summary";
    POEntry.msgID   = mapAddonXMLData["en"].strSummary;
    POEntry.msgStr  = bIsSource ? "": mapAddonXMLData[LCode].strSummary;
    PODoc.WritePOEntry(POEntry);
  }

  if (!mapAddonXMLData["en"].strDescription.empty())
  {
    CPOEntry POEntry;
    POEntry.Type = MSGID_FOUND;
    POEntry.msgCtxt = "Addon Description";
    POEntry.msgID   = mapAddonXMLData["en"].strDescription;
    POEntry.msgStr  = bIsSource ? "": mapAddonXMLData[LCode].strDescription;
    PODoc.WritePOEntry(POEntry);
  }

  if (!mapAddonXMLData["en"].strDisclaimer.empty())
  {
    CPOEntry POEntry;
    POEntry.Type = MSGID_FOUND;
    POEntry.msgCtxt = "Addon Disclaimer";
    POEntry.msgID   = mapAddonXMLData["en"].strDisclaimer;
    POEntry.msgStr  = bIsSource ? "": mapAddonXMLData[LCode].strDisclaimer;
    PODoc.WritePOEntry(POEntry);
  }

  for ( itStrings it = m_mapStrings.begin() ; it != m_mapStrings.end() ; it++)
  {
    int id = it->first;
    CPOEntry currEntry = it->second;
    PODoc.WritePOEntry(currEntry);
  }

  PODoc.SaveFile(OutputPOFilename);

  return true;
};
