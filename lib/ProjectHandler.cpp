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

#include "ProjectHandler.h"
#include <list>
#include "HTTPUtils.h"
#include "JSONHandler.h"
#include "Settings.h"
#include <algorithm>

CProjectHandler::CProjectHandler()
{};

CProjectHandler::~CProjectHandler()
{};

bool CProjectHandler::FetchResourcesFromTransifex()
{
  g_HTTPHandler.Cleanup();
  g_HTTPHandler.ReInit();
  std::string strtemp = g_HTTPHandler.GetURLToSTR("https://www.transifex.com/api/2/project/" + g_Settings.GetProjectname()
                                                  + "/resources/");
  if (strtemp.empty())
    CLog::Log(logERROR, "ProjectHandler::FetchResourcesFromTransifex: error getting resources from transifex.net");

  char cstrtemp[strtemp.size()];
  strcpy(cstrtemp, strtemp.c_str());

  CJSONHandler JSONHandler;
  std::list<std::string> listResourceNamesTX = JSONHandler.ParseResources(strtemp);

  CResourceHandler ResourceHandler;

  for (std::list<std::string>::iterator it = listResourceNamesTX.begin(); it != listResourceNamesTX.end(); it++)
  {
    printf("Downloading resource from TX: %s\n", it->c_str());
    CLog::Log(logLINEFEED, "");
    CLog::Log(logINFO, "ProjHandler: *** Fetch Resource: %s ***", it->c_str());

    std::string strResname = m_UpdateXMLHandler.GetResNameFromTXResName(*it);
    if (strResname.empty())
    {
      CLog::Log(logWARNING, "ProjHandler: found resource on Transifex which is not in xbmc-txupdate.xml: %s", it->c_str());
      continue;
    }

    m_mapResourcesTX[strResname]=ResourceHandler;
    m_mapResourcesTX[strResname].FetchPOFilesTXToMem("https://www.transifex.com/api/2/project/" + g_Settings.GetProjectname() +
                                              "/resource/" + *it + "/");
  }
  return true;
};

bool CProjectHandler::FetchResourcesFromUpstream()
{

  std::map<std::string, CXMLResdata> mapRes = m_UpdateXMLHandler.GetResMap();

  CResourceHandler ResourceHandler;

  for (std::map<std::string, CXMLResdata>::iterator it = mapRes.begin(); it != mapRes.end(); it++)
  {
    printf("Downloading resource from Upstream: %s\n", it->first.c_str());
    CLog::Log(logLINEFEED, "");
    CLog::Log(logINFO, "ProjHandler: *** Fetch Resource from upstream: %s ***", it->first.c_str());

    m_mapResourcesUpstr[it->first] = ResourceHandler;
    m_mapResourcesUpstr[it->first].FetchPOFilesUpstreamToMem(it->second, CreateMergedLanguageList(it->first, true));
  }
  return true;
};

bool CProjectHandler::WriteResourcesToFile(std::string strProjRootDir)
{
  for (T_itmapRes itmapResources = m_mapResMerged.begin(); itmapResources != m_mapResMerged.end(); itmapResources++)
  {
    printf("Writing resource to HDD: %s\n", itmapResources->first.c_str());
    CLog::Log(logLINEFEED, "");
    CLog::Log(logINFO, "ProjHandler: *** Write Resource: %s ***", itmapResources->first.c_str());
    CXMLResdata XMLResdata = m_UpdateXMLHandler.GetResData(itmapResources->first);

    m_mapResMerged[itmapResources->first].WritePOToFiles (strProjRootDir, "", itmapResources->first, XMLResdata);
  }

  for (T_itmapRes itmapResources = m_mapResUpdateTX.begin(); itmapResources != m_mapResUpdateTX.end(); itmapResources++)
  {
    printf("Writing update TX resource to HDD: %s\n", itmapResources->first.c_str());
    CLog::Log(logLINEFEED, "");
    CLog::Log(logINFO, "ProjHandler: *** Write UpdTX Resource: %s ***", itmapResources->first.c_str());
    CXMLResdata XMLResdata = m_UpdateXMLHandler.GetResData(itmapResources->first);

    std::string strPrefixDir = "temp_txupdate";
    strPrefixDir += DirSepChar;
    m_mapResUpdateTX[itmapResources->first].WritePOToFiles (strProjRootDir, strPrefixDir, itmapResources->first, XMLResdata);
  }

  return true;
};

bool CProjectHandler::CreateMergedResources()
{
  CLog::Log(logINFO, "CreateMergedResources started");

  std::list<std::string> listMergedResource = CreateResourceList();

  m_mapResMerged.clear();
  m_mapResUpdateTX.clear();

  for (std::list<std::string>::iterator itResAvail = listMergedResource.begin(); itResAvail != listMergedResource.end(); itResAvail++)
  {
    printf("Merging resource: %s\n", itResAvail->c_str());
    CLog::Log(logINFO, "CreateMergedResources: Merging resource:%s", itResAvail->c_str());

    CResourceHandler mergedResHandler, updTXResHandler;

    // Get available pretext for Resource Header. First use the upstream one, if not avail. the TX one
    std::string strResPreHeader;
    if (m_mapResourcesUpstr.find(*itResAvail) != m_mapResourcesUpstr.end())
      strResPreHeader = m_mapResourcesUpstr[*itResAvail].GetXMLHandler()->GetResHeaderPretext();
    else if (m_mapResourcesTX.find(*itResAvail) != m_mapResourcesTX.end())
      strResPreHeader = m_mapResourcesTX[*itResAvail].GetXMLHandler()->GetResHeaderPretext();

    CLog::Log(logINFO, "MergedPOHandl:\tLanguage\t\t\tID entries\tnon-ID entries\tInterline-comments");

    std::list<std::string> listMergedLangs = CreateMergedLanguageList(*itResAvail, false);

    CAddonXMLEntry * pENAddonXMLEntry;

    if ((pENAddonXMLEntry = GetAddonDataFromXML(&m_mapResourcesUpstr, *itResAvail, "en")) != NULL)
    {
      CLog::Log(logINFO, "CreateMergedResources: Using Upstream AddonXML file as source for merging");
      mergedResHandler.GetXMLHandler()->SetStrAddonXMLFile(m_mapResourcesUpstr[*itResAvail].GetXMLHandler()->GetStrAddonXMLFile());
      updTXResHandler.GetXMLHandler()->SetStrAddonXMLFile(m_mapResourcesUpstr[*itResAvail].GetXMLHandler()->GetStrAddonXMLFile());
    }
    else if (*itResAvail != "xbmc.core")
      CLog::Log(logERROR, "CreateMergedResources: No Upstream AddonXML file found as source for merging");

    CPOHandler * pcurrPOHandlerEN = m_mapResourcesUpstr[*itResAvail].GetPOData("en");

    for (std::list<std::string>::iterator itlang = listMergedLangs.begin(); itlang != listMergedLangs.end(); itlang++)
    {
      std::string strLangCode = *itlang;
      CPOHandler mergedPOHandler, updTXPOHandler;
      const CPOEntry* pPOEntryTX;
      const CPOEntry* pPOEntryUpstr;

      mergedPOHandler.SetIfIsEnglish(strLangCode == "en");
      updTXPOHandler.SetIfIsEnglish(strLangCode == "en");

      CAddonXMLEntry MergedAddonXMLEntry, MergedAddonXMLEntryTX;
      CAddonXMLEntry * pAddonXMLEntry;
      if (m_mapResourcesTX.find(*itResAvail) != m_mapResourcesTX.end() && m_mapResourcesTX[*itResAvail].GetPOData(*itlang))
      {
        CAddonXMLEntry AddonXMLEntryInPO, AddonENXMLEntryInPO;
        m_mapResourcesTX[*itResAvail].GetPOData(*itlang)->GetAddonMetaData(AddonXMLEntryInPO, AddonENXMLEntryInPO);
        MergeAddonXMLEntry(AddonXMLEntryInPO, MergedAddonXMLEntry, *pENAddonXMLEntry, AddonENXMLEntryInPO);
      }
      MergedAddonXMLEntryTX = MergedAddonXMLEntry;
      if ((pAddonXMLEntry = GetAddonDataFromXML(&m_mapResourcesUpstr, *itResAvail, *itlang)) != NULL)
        MergeAddonXMLEntry(*pAddonXMLEntry, MergedAddonXMLEntry, *pENAddonXMLEntry,
                           *GetAddonDataFromXML(&m_mapResourcesUpstr, *itResAvail, "en"));

      if (*itResAvail != "xbmc.core")
      {
        mergedResHandler.GetXMLHandler()->GetMapAddonXMLData()->operator[](*itlang) = MergedAddonXMLEntry;
        updTXResHandler.GetXMLHandler()->GetMapAddonXMLData()->operator[](*itlang) = MergedAddonXMLEntry;
        updTXPOHandler.SetAddonMetaData(MergedAddonXMLEntry, MergedAddonXMLEntryTX, *pENAddonXMLEntry, *itlang); // add addonxml data as PO  classic entries
      }

      for (size_t POEntryIdx = 0; pcurrPOHandlerEN && POEntryIdx != pcurrPOHandlerEN->GetNumEntriesCount(); POEntryIdx++)
      {
        size_t numID = pcurrPOHandlerEN->GetNumPOEntryByIdx(POEntryIdx)->numID;
        const CPOEntry* pcurrPOEntryEN = pcurrPOHandlerEN->GetNumPOEntryByIdx(POEntryIdx);

        pPOEntryTX = SafeGetPOEntry(m_mapResourcesTX, *itResAvail, strLangCode, numID);
        pPOEntryUpstr = SafeGetPOEntry(m_mapResourcesUpstr, *itResAvail, strLangCode, numID);

        if (strLangCode == "en")
        {
          mergedPOHandler.AddNumPOEntryByID(numID, *pcurrPOEntryEN);
          updTXPOHandler.AddNumPOEntryByID(numID, *pcurrPOEntryEN);
        }

        if (strLangCode != "en" && pPOEntryTX && pPOEntryTX->msgID == pcurrPOEntryEN->msgID && !pPOEntryTX->msgStr.empty())
          mergedPOHandler.AddNumPOEntryByID(numID, *pPOEntryTX);
        else if (strLangCode != "en" && pPOEntryUpstr && (pPOEntryUpstr->msgID == pcurrPOEntryEN->msgID) && !pPOEntryUpstr->msgStr.empty())
        {
          mergedPOHandler.AddNumPOEntryByID(numID, *pPOEntryUpstr);
          updTXPOHandler.AddNumPOEntryByID(numID, *pPOEntryUpstr);
        }
        else if (strLangCode != "en" && pPOEntryUpstr && pPOEntryUpstr->msgID.empty() && !pPOEntryUpstr->msgStr.empty())
        {
          mergedPOHandler.AddNumPOEntryByID(numID, *pPOEntryUpstr, pcurrPOEntryEN->msgID); // we got this entry from a strings.xml file
          updTXPOHandler.AddNumPOEntryByID(numID, *pPOEntryUpstr, pcurrPOEntryEN->msgID);
        }
        else if (strLangCode != "en")
          mergedPOHandler.AddNumPOEntryByID(numID, *pcurrPOEntryEN);
      }

      if (mergedPOHandler.GetNumEntriesCount() !=0 || mergedPOHandler.GetClassEntriesCount() !=0)
      {
        CPOHandler * pPOHandlerTX, * pPOHandlerUpst;
        pPOHandlerTX = SafeGetPOHandler(m_mapResourcesTX, *itResAvail, strLangCode);
        pPOHandlerUpst = SafeGetPOHandler(m_mapResourcesUpstr, *itResAvail, strLangCode);
        if (pPOHandlerTX && strLangCode != "en")
        {
          mergedPOHandler.SetHeader(pPOHandlerTX->GetHeader());
          updTXPOHandler.SetHeader(pPOHandlerTX->GetHeader());
        }
        else if (pPOHandlerUpst && !pcurrPOHandlerEN->GetIfSourceIsXML())
        {
          mergedPOHandler.SetHeader(pPOHandlerUpst->GetHeader());
          updTXPOHandler.SetHeader(pPOHandlerUpst->GetHeader());
        }
        else if (pcurrPOHandlerEN->GetIfSourceIsXML())
        {
          mergedPOHandler.SetHeaderXML(*itlang);
          updTXPOHandler.SetHeaderXML(*itlang);
        }

//        if (strLangCode != "en" || pcurrPOHandlerEN->GetIfSourceIsXML())
//        {
          mergedPOHandler.SetPreHeader(strResPreHeader);
          updTXPOHandler.SetPreHeader(strResPreHeader);
//        }

        if (mergedPOHandler.GetClassEntriesCount() != 0 || mergedPOHandler.GetNumEntriesCount() != 0)
          mergedResHandler.AddPOData(mergedPOHandler, strLangCode);
        if (updTXPOHandler.GetClassEntriesCount() != 0 || updTXPOHandler.GetNumEntriesCount() != 0)
          updTXResHandler.AddPOData(updTXPOHandler, strLangCode);
        CLog::Log(logINFO, "MergedPOHandler: %s\t\t%i\t\t%i\t\t%i", strLangCode.c_str(), mergedPOHandler.GetNumEntriesCount(),
                  mergedPOHandler.GetClassEntriesCount(), mergedPOHandler.GetCommntEntriesCount());
      }
    }
    if (mergedResHandler.GetLangsCount() != 0 || !mergedResHandler.GetXMLHandler()->GetMapAddonXMLData()->empty())
      m_mapResMerged[*itResAvail] = mergedResHandler;
    if (updTXResHandler.GetLangsCount() != 0 || !updTXResHandler.GetXMLHandler()->GetMapAddonXMLData()->empty())
      m_mapResUpdateTX[*itResAvail] = updTXResHandler;
  }
  return true;
}

std::list<std::string> CProjectHandler::CreateMergedLanguageList(std::string strResname, bool bOnlyTX)
{
  std::list<std::string> listMergedLangs;

  if (m_mapResourcesTX.find(strResname) != m_mapResourcesTX.end())
  {
    for (size_t i =0; i != m_mapResourcesTX[strResname].GetLangsCount(); i++)
    {
      std::string strMLCode = m_mapResourcesTX[strResname].GetLangCodeFromPos(i);
      if (std::find(listMergedLangs.begin(), listMergedLangs.end(), strMLCode) == listMergedLangs.end())
        listMergedLangs.push_back(strMLCode);
    }
  }
  if (bOnlyTX)
    return listMergedLangs;

  if (m_mapResourcesUpstr.find(strResname) != m_mapResourcesUpstr.end())
  {
    for (size_t i =0; i != m_mapResourcesUpstr[strResname].GetLangsCount(); i++)
    {
      std::string strMLCode = m_mapResourcesUpstr[strResname].GetLangCodeFromPos(i);
      if (std::find(listMergedLangs.begin(), listMergedLangs.end(), strMLCode) == listMergedLangs.end())
        listMergedLangs.push_back(strMLCode);
    }
  }

  return listMergedLangs;
}

std::list<std::string> CProjectHandler::CreateResourceList()
{
  std::list<std::string> listMergedResources;
  for (T_itmapRes it = m_mapResourcesUpstr.begin(); it != m_mapResourcesUpstr.end(); it++)
  {
    if (std::find(listMergedResources.begin(), listMergedResources.end(), it->first) == listMergedResources.end())
      listMergedResources.push_back(it->first);
  }

  for (T_itmapRes it = m_mapResourcesTX.begin(); it != m_mapResourcesTX.end(); it++)
  {
    if (std::find(listMergedResources.begin(), listMergedResources.end(), it->first) == listMergedResources.end())
      listMergedResources.push_back(it->first);
  }

  return listMergedResources;
}

CAddonXMLEntry * const CProjectHandler::GetAddonDataFromXML(std::map<std::string, CResourceHandler> * pmapRes,
                                                            const std::string &strResname, const std::string &strLangCode) const
{
  if (pmapRes->find(strResname) == pmapRes->end())
    return NULL;

  CResourceHandler * pRes = &(pmapRes->operator[](strResname));
  if (pRes->GetXMLHandler()->GetMapAddonXMLData()->find(strLangCode) == pRes->GetXMLHandler()->GetMapAddonXMLData()->end())
    return NULL;

  return &(pRes->GetXMLHandler()->GetMapAddonXMLData()->operator[](strLangCode));
}

const CPOEntry * CProjectHandler::SafeGetPOEntry(std::map<std::string, CResourceHandler> &mapResHandl, const std::string &strResname,
                          std::string &strLangCode, size_t numID)
{
  if (mapResHandl.find(strResname) == mapResHandl.end())
    return NULL;
  if (!mapResHandl[strResname].GetPOData(strLangCode))
    return NULL;
  return mapResHandl[strResname].GetPOData(strLangCode)->GetNumPOEntryByID(numID);
}

CPOHandler * CProjectHandler::SafeGetPOHandler(std::map<std::string, CResourceHandler> &mapResHandl, const std::string &strResname,
                                                 std::string &strLangCode)
{
  if (mapResHandl.find(strResname) == mapResHandl.end())
    return NULL;
  return mapResHandl[strResname].GetPOData(strLangCode);
}

void CProjectHandler::MergeAddonXMLEntry(CAddonXMLEntry const &EntryToMerge, CAddonXMLEntry &MergedAddonXMLEntry,
                                         CAddonXMLEntry const &SourceENEntry, CAddonXMLEntry const &CurrENEntry)
{
  if (!EntryToMerge.strDescription.empty() && MergedAddonXMLEntry.strDescription.empty() &&
      CurrENEntry.strDescription == SourceENEntry.strDescription)
    MergedAddonXMLEntry.strDescription = EntryToMerge.strDescription;

  if (!EntryToMerge.strDisclaimer.empty() && MergedAddonXMLEntry.strDisclaimer.empty() &&
    CurrENEntry.strDisclaimer == SourceENEntry.strDisclaimer)
    MergedAddonXMLEntry.strDisclaimer = EntryToMerge.strDisclaimer;

  if (!EntryToMerge.strSummary.empty() && MergedAddonXMLEntry.strSummary.empty() &&
    CurrENEntry.strSummary == SourceENEntry.strSummary)
    MergedAddonXMLEntry.strSummary = EntryToMerge.strSummary;
}

void CProjectHandler::InitUpdateXMLHandler(std::string strProjRootDir)
{
m_UpdateXMLHandler.LoadXMLToMem(strProjRootDir);
}

