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

#include "UpdateXMLHandler.h"
#include "Log.h"
#include "Settings.h"
#include <stdlib.h>
#include <sstream>


using namespace std;

CXMLResdata::CXMLResdata()
{
  Restype = UNKNOWN;
}

CXMLResdata::~CXMLResdata()
{}

CUpdateXMLHandler::CUpdateXMLHandler()
{};

CUpdateXMLHandler::~CUpdateXMLHandler()
{};

bool CUpdateXMLHandler::LoadXMLToMem (std::string rootDir)
{
  std::string UpdateXMLFilename = rootDir  + DirSepChar + "xbmc-txupdate.xml";
  TiXmlDocument xmlUpdateXML;

  if (!xmlUpdateXML.LoadFile(UpdateXMLFilename.c_str()))
  {
    CLog::Log(logINFO, "UpdXMLHandler: No update.xml file exists, we will create one later");
    return false;
  }

  CLog::Log(logINFO, "UpdXMLHandler: Succesfuly found the update.xml file");

  TiXmlElement* pRootElement = xmlUpdateXML.RootElement();
  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueTStr()!="resources")
  {
    CLog::Log(logINFO, "UpdXMLHandler: No root element called \"resources\" in xml file. We will create it");
    return false;
  }

  std::string strProjName = pRootElement->Attribute("projectname");
  if (strProjName == "" || strProjName == DEFAULTPRPJNAME)
    CLog::Log(logERROR, "UpdXMLHandler: No projectname found in xbmc-txupdate.xml file. Please specify the Transifex "
    "projectname in the xml file");
  else
  {
    CLog::Log(logINFO, "UpdXMLHandler: Found projectname in xbmc-txupdate.xml file: %s",strProjName.c_str());
    g_Settings.SetProjectname(strProjName);
  }

  std::string strHTTPCacheExp = pRootElement->Attribute("http_cache_expire");
  if (strHTTPCacheExp == "")
    CLog::Log(logINFO, "UpdXMLHandler: No http cache expire time found in xbmc-txupdate.xml file. Please specify it!");
  else
  {
    CLog::Log(logINFO, "UpdXMLHandler: Found http cache expire time in xbmc-txupdate.xml file: %s", strHTTPCacheExp.c_str());
    g_Settings.SetHTTPCacheExpire(strtol(&strHTTPCacheExp[0], NULL, 10));
  }

  std::string strMinCompletion = pRootElement->Attribute("min_completion");
  if (strMinCompletion == "")
    CLog::Log(logINFO, "UpdXMLHandler: No min completion percentage found in xbmc-txupdate.xml file. Please specify it!");
  else
  {
    CLog::Log(logINFO, "UpdXMLHandler: Found min completion percentage in xbmc-txupdate.xml file: %s", strMinCompletion.c_str());
    g_Settings.SetMinCompletion(strtol(&strMinCompletion[0], NULL, 10));
  }

  const TiXmlElement *pChildResElement = pRootElement->FirstChildElement("resource");
  std::string strType;
  while (pChildResElement && pChildResElement->FirstChild())
  {
    CXMLResdata currResData;
    std::string strResName = pChildResElement->Attribute("name");
    if (pChildResElement->FirstChild())
    {
      const TiXmlElement *pChildURLElement = pChildResElement->FirstChildElement("upstreamURL");
      if (pChildURLElement && pChildURLElement->FirstChild())
        currResData.strUptreamURL = pChildURLElement->FirstChild()->Value();
      if (currResData.strUptreamURL.empty())
        CLog::Log(logERROR, "UpdXMLHandler: UpstreamURL entry is empty for resource %s", strResName.c_str());
      if (pChildURLElement->Attribute("filetype"))
        currResData.strLangFileType = pChildURLElement->Attribute("filetype");

      const TiXmlElement *pChildUpstrLElement = pChildResElement->FirstChildElement("upstreamLangs");
      if (pChildUpstrLElement && pChildUpstrLElement->FirstChild())
        currResData.strLangsFromUpstream = pChildUpstrLElement->FirstChild()->Value();

      const TiXmlElement *pChildResTypeElement = pChildResElement->FirstChildElement("resourceType");
      if (pChildResTypeElement && pChildResTypeElement->FirstChild())
      {
        strType = pChildResTypeElement->FirstChild()->Value();
        if (strType == "addon")
         currResData.Restype = ADDON;
        else if (strType == "addon_nostrings")
          currResData.Restype = ADDON_NOSTRINGS;
        else if (strType == "skin")
          currResData.Restype = SKIN;
        else if (strType == "xbmc_core")
          currResData.Restype = CORE;
      }
      if (currResData.Restype == UNKNOWN)
        CLog::Log(logERROR, "UpdXMLHandler: Unknown type found for resource %s", strResName.c_str());

      const TiXmlElement *pChildResDirElement = pChildResElement->FirstChildElement("resourceSubdir");
      if (pChildResDirElement && pChildResDirElement->FirstChild())
        currResData.strResDirectory = pChildResDirElement->FirstChild()->Value();

      const TiXmlElement *pChildTXNameElement = pChildResElement->FirstChildElement("TXname");
      if (pChildTXNameElement && pChildTXNameElement->FirstChild())
        currResData.strTXResName = pChildTXNameElement->FirstChild()->Value();
      if (currResData.strTXResName.empty())
        CLog::Log(logERROR, "UpdXMLHandler: Transifex resource name is empty for resource %s", strResName.c_str());

      m_mapXMLResdata[strResName] = currResData;
      CLog::Log(logINFO, "UpdXMLHandler: found resource in update.xml file: %s, Type: %s, SubDir: %s",
                strResName.c_str(), strType.c_str(), currResData.strResDirectory.c_str());
    }
    pChildResElement = pChildResElement->NextSiblingElement("resource");
  }

  return true;
};

std::string CUpdateXMLHandler::GetResNameFromTXResName(std::string const &strTXResName)
{
  for (itXMLResdata = m_mapXMLResdata.begin(); itXMLResdata != m_mapXMLResdata.end(); itXMLResdata++)
  {
    if (itXMLResdata->second.strTXResName == strTXResName)
      return itXMLResdata->first;
  }
  return "";
}

std::string CUpdateXMLHandler::IntToStr(int number)
{
  std::stringstream ss;//create a stringstream
  ss << number;//add number to the stream
  return ss.str();//return a string with the contents of the stream
};

CXMLResdata CUpdateXMLHandler::GetResData(string strResName)
{
  CXMLResdata EmptyXMLResdata;
  if (m_mapXMLResdata.find(strResName) != m_mapXMLResdata.end())
    return m_mapXMLResdata[strResName];
  CLog::Log(logINFO, "UpdXMLHandler::GetResData: unknown resource to find: %s", strResName.c_str());
  return EmptyXMLResdata;
}
