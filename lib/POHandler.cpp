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
#include "FileUtils/FileUtils.h"
#include "POHandler.h"

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

  WritePOFile(strDir, strLang);

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
                             const std::map<std::string, CAddonXMLEntry> &mapAddonXMLData, const CResData &ResData)
{
  int writtenEntry = 0;
  std::string OutputPOFilename;
  size_t startpos = m_strHeader.find("Language: ")+10;
  size_t endpos = m_strHeader.find_first_of("\\ \n", startpos);
  std::string LCode = m_strHeader.substr(startpos, endpos-startpos);
  bool bIsForeignLang = strLang != "English";
  bhasLFWritten = false;

  OutputPOFilename = strDir + DirSepChar + strLang + DirSepChar + "strings.po.temp";

  // Initalize the output po document

  FILE * pPOTFile = fopen (OutputPOFilename.c_str(),"wb");
  if (pPOTFile == NULL)
  {
    printf("Error opening output file: %s\n", OutputPOFilename.c_str());
    return false;
  }
  printf("%s\t\t", LCode.c_str());

  size_t startPos;
  startPos = m_strHeader.find("msgid \"\"");
  if ((startPos = m_strHeader.find("# Translators")) != std::string::npos)
    m_strHeader = m_strHeader.substr(startPos);
  else if ((startPos = m_strHeader.find("msgid \"\"")) != std::string::npos)
    m_strHeader = m_strHeader.substr(startPos);

  fprintf(pPOTFile, "# XBMC Media Center language file\n");
  if (resType != CORE && resType != UNKNOWN)
  {
    fprintf(pPOTFile, "# Addon Name: %s\n", ResData.ResTextName.c_str());
    fprintf(pPOTFile, "# Addon id: %s\n", ResData.ResName.c_str());
    fprintf(pPOTFile, "# Addon version: %s\n", ResData.ResVersion.c_str());
    fprintf(pPOTFile, "# Addon Provider: %s\n", ResData.ResProvider.c_str());
  }

  fprintf(pPOTFile, "%s", m_strHeader.c_str());
  
  if (!mapAddonXMLData["en"].strSummary.empty())
  {
    WriteStrLine("msgctxt ", "Addon Summary", addonXMLEncoding, pPOTFile);
    WriteStrLine("msgid ", mapAddonXMLData["en"].strSummary.c_str(), addonXMLEncoding, pPOTFile);
    WriteStrLine("msgstr ", LCode == "en" ? "": mapAddonXMLData[LCode].strSummary.c_str(), addonXMLEncoding, pPOTFile);
    bhasLFWritten =false;
    writtenEntry++;
  }
  
  if (!mapAddonXMLData["en"].strDescription.empty())
  {
    WriteStrLine("msgctxt ", "Addon Description", addonXMLEncoding, pPOTFile);
    WriteStrLine("msgid ", mapAddonXMLData["en"].strDescription.c_str(), addonXMLEncoding, pPOTFile);
    WriteStrLine("msgstr ", LCode == "en" ? "": mapAddonXMLData[LCode].strDescription.c_str(), addonXMLEncoding, pPOTFile);
    bhasLFWritten =false;
    writtenEntry++;
  }
  
  if (!mapAddonXMLData["en"].strDisclaimer.empty())
  {
    WriteStrLine("msgctxt ", "Addon Disclaimer", addonXMLEncoding, pPOTFile);
    WriteStrLine("msgid ", mapAddonXMLData["en"].strDisclaimer.c_str(), addonXMLEncoding, pPOTFile);
    WriteStrLine("msgstr ", LCode == "en" ? "": mapAddonXMLData[LCode].strDisclaimer.c_str(), addonXMLEncoding, pPOTFile);
    bhasLFWritten =false;
    writtenEntry++;
  }
  
  int previd = -1;
  
  for ( itStrings it = mapStrings.begin() ; it != mapStrings.end() ; it++)
  {
    bhasLFWritten =false;
    int id = it->first;
    CPOEntry currEntry = it->second;
    
    if (!bIsForeignLang)
    {
      WriteMultilineComment(currEntry.interlineComm, "#");
      if (id-previd >= 2 && previd > -1)
      {
        WriteLF(pPOTFile);
        if (id-previd == 2)
          fprintf(pPOTFile,"#empty string with id %i\n", id-1);
        if (id-previd > 2)
          fprintf(pPOTFile,"#empty strings from id %i to %i\n", previd+1, id-1);
      }
    }
    bhasLFWritten = false;
    
    if (!bIsForeignLang)
    {
      WriteMultilineComment(currEntry.translatorComm, "# ");
      WriteMultilineComment(currEntry.extractedComm, "#.");
      WriteMultilineComment(currEntry.referenceComm, "#:");
    }
    
    WriteLF(pPOTFile);
    fprintf(pPOTFile,"msgctxt \"#%i\"\n", id);
    
    WriteLF(pPOTFile);
    fprintf(pPOTFile,"msgid \"%s\"\n", currEntry.msgID.c_str());
    fprintf(pPOTFile,"msgstr \"%s\"\n", currEntry.msgStr.c_str());
    
    writtenEntry++;
    previd =id;
  }
  fclose(pPOTFile);
  
  printf("%i\t\t", writtenEntry);
  printf("%i\t\t", 0);
  printf("%s\n", OutputPOFilename.erase(0,strDir.length()).c_str());
  
  return true;
};
