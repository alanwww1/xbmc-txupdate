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

#include "POHandler.h"

CPOHandler::CPOHandler()
{};

CPOHandler::~CPOHandler()
{};

bool CPOHandler::LoadPOFile(std::string strPOFileName)
{
  CPODocument PODoc;
  if (!PODoc.LoadFile(strPOFileName))
    return false;
  return ProcessPOFile(PODoc);
};

bool CPOHandler::FetchPOTXToMem (std::string strURL)
{
  CPODocument PODoc;
  if (!PODoc.FetchTXToMem(strURL))
    return false;
  return ProcessPOFile(PODoc);
};

bool CPOHandler::ProcessPOFile(CPODocument &PODoc)
{
  if (PODoc.GetEntryType() != HEADER_FOUND)
    CLog::Log(logERROR, "POHandler: No valid header found for this language");

  m_strHeader = PODoc.GetEntryData().Content.substr(1);

  m_mapStrings.clear();
  m_vecClassicEntries.clear();
  int ilCommsCntr = 0;

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
      ilCommsCntr++;
      continue;
    }

    if (currType == ID_FOUND || currType == MSGID_FOUND || currType == MSGID_PLURAL_FOUND)
    {
      if (bMultipleComment)
        CLog::Log(logWARNING, "POHandler: multiple comment entries found. Using only the last one "
        "before the real entry. Entry after comments: %s", currEntry.Content.c_str());
      if (!currEntry.interlineComm.empty())
        CLog::Log(logWARNING, "POParser: interline comments (eg. #comment) is not alowed inside "
        "a real po entry. Cleaned it. Problematic entry: %s", currEntry.Content.c_str());
      currEntry.interlineComm = vecILCommnts;
      bMultipleComment = false;
      vecILCommnts.clear();

      if (currType == ID_FOUND)
        m_mapStrings[currEntry.numID] = currEntry;
      else
      {
        m_vecClassicEntries.push_back(currEntry);
      }
      ClearCPOEntry(currEntry);
    }
  }
  m_CommsCntr = ilCommsCntr;

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
  entry.Content.clear();
};


bool CPOHandler::WritePOFile(const std::string &strOutputPOFilename)
{
  CPODocument PODoc;

  PODoc.SetIfIsEnglish(m_bPOIsEnglish);

  PODoc.WriteHeader(m_strHeader);

  CPOEntry POEntry;
  POEntry.msgCtxt = "Addon Summary";
  if (LookforClassicEntry(POEntry))
    PODoc.WritePOEntry(POEntry);

  ClearCPOEntry(POEntry);
  POEntry.msgCtxt = "Addon Description";
  if (LookforClassicEntry(POEntry))
    PODoc.WritePOEntry(POEntry);

  ClearCPOEntry(POEntry);
  POEntry.msgCtxt = "Addon Disclaimer";
  if (LookforClassicEntry(POEntry))
    PODoc.WritePOEntry(POEntry);

  for (itStrings it = m_mapStrings.begin(); it != m_mapStrings.end(); it++)
  {
//    int id = it->first;
    CPOEntry currEntry = it->second;
    PODoc.WritePOEntry(currEntry);
  }

  PODoc.SaveFile(strOutputPOFilename);

  return true;
};

// Data manipulation functions

bool CPOHandler::LookforClassicEntry (CPOEntry &EntryToFind)
{
  for (itClassicEntries it = m_vecClassicEntries.begin(); it != m_vecClassicEntries.end(); it++)
  {
    if (*it == EntryToFind)
    {
      EntryToFind = *it;
      return true;
    }
  }
  return false;
}

void CPOHandler::AddClassicEntry (CPOEntry &EntryToAdd)
{
  m_vecClassicEntries.push_back(EntryToAdd);
};

bool CPOHandler::ModifyClassicEntry (CPOEntry &EntryToFind, CPOEntry EntryNewValue)
{
  for (itClassicEntries it = m_vecClassicEntries.begin(); it != m_vecClassicEntries.end(); it++)
  {
    if (*it == EntryToFind)
    {
      *it = EntryNewValue;
      return true;
    }
  }
  m_vecClassicEntries.push_back(EntryNewValue);
  return false;
}

bool CPOHandler::DeleteClassicEntry (CPOEntry &EntryToFind)
{
  for (itClassicEntries it = m_vecClassicEntries.begin(); it != m_vecClassicEntries.end(); it++)
  {
    if (*it == EntryToFind)
    {
      m_vecClassicEntries.erase(it);
      return true;
    }
  }
  return false;
}

void CPOHandler::SetAddonMetaData (CAddonXMLEntry AddonXMLEntry, CAddonXMLEntry AddonXMLEntryEN)
{
  CPOEntry POEntryDesc, POEntryDiscl, POEntrySumm;
  POEntryDesc.Type = MSGID_FOUND;
  POEntryDiscl.Type = MSGID_FOUND;
  POEntrySumm.Type = MSGID_FOUND;
  POEntryDesc.msgCtxt = "Addon Description";
  POEntryDiscl.msgCtxt = "Addon Disclaimer";
  POEntrySumm.msgCtxt = "Addon Summary";

  CPOEntry newPOEntryDesc = POEntryDesc;
  CPOEntry newPOEntryDisc = POEntryDiscl;
  CPOEntry newPOEntrySumm = POEntrySumm;

  newPOEntryDesc.msgID = AddonXMLEntryEN.strDescription;
  newPOEntryDisc.msgID = AddonXMLEntryEN.strDisclaimer;
  newPOEntrySumm.msgID = AddonXMLEntryEN.strSummary;

  if (!AddonXMLEntry.strDescription.empty() || !AddonXMLEntry.strDisclaimer.empty() || !AddonXMLEntry.strSummary.empty())
  {
    newPOEntryDesc.msgStr = AddonXMLEntry.strDescription;
    newPOEntryDisc.msgStr = AddonXMLEntry.strDisclaimer;
    newPOEntrySumm.msgStr = AddonXMLEntry.strSummary;
  }

  ModifyClassicEntry(POEntryDesc, newPOEntryDesc);
  ModifyClassicEntry(POEntryDiscl, newPOEntryDisc);
  ModifyClassicEntry(POEntrySumm, newPOEntrySumm);
  return;
}

void CPOHandler::SetHeader (std::string strPreText)
{
  if (strPreText.empty())
    return;

  std::string strOutHeader;
  size_t startPos;

  if ((startPos = m_strHeader.find("# Translators")) != std::string::npos)
    m_strHeader = m_strHeader.substr(startPos);
  else if ((startPos = m_strHeader.find("msgid \"\"")) != std::string::npos)
    m_strHeader = m_strHeader.substr(startPos);  startPos = m_strHeader.find("msgid \"\"");

  strOutHeader += "# XBMC Media Center language file\n";

  strOutHeader += strPreText + m_strHeader;
  m_strHeader = strOutHeader;
}
