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

CUpdateXMLHandler::CUpdateXMLHandler()
{};

CUpdateXMLHandler::~CUpdateXMLHandler()
{};

bool CUpdateXMLHandler::Load (std::string UpdateXMLFilename)
{
  TiXmlDocument xmlUpdateXML;

  if (!xmlUpdateXML.LoadFile(UpdateXMLFilename.c_str()))
  {
    CLog::Log(logINFO, "UpdXMLHandler: No update.xml file exists, we will create one later\n");
    return false;
  }

  CXMLResdata currResData;

  TiXmlElement* pRootElement = xmlUpdateXML.RootElement();

  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueTStr()!="resources")
  {
    CLog::Log(logINFO, "UpdXMLHandler: No root element called \"resources\" in xml file. We will create it\n");
    return false;
  }

  const TiXmlElement *pChildResElement = pRootElement->FirstChildElement("resource");
  while (pChildResElement && pChildResElement->FirstChild())
  {
    std::string strResName = pChildResElement->Attribute("name");
    if (pChildResElement->FirstChild())
    {
      const TiXmlElement *pChildTypeElement = pChildResElement->FirstChildElement("type");
      currResData.strResType = pChildTypeElement->FirstChild()->Value();
      const TiXmlElement *pChildURLElement = pChildResElement->FirstChildElement("upstreamURL");
      currResData.strUptreamURL = pChildURLElement->FirstChild()->Value();
      const TiXmlElement *pChildUpstrLElement = pChildResElement->FirstChildElement("upstreamLangs");
      currResData.strLangsFromUpstream = pChildUpstrLElement->FirstChild()->Value();
      m_mapXMLResdata[strResName] = currResData;
    }
    pChildResElement = pChildResElement->NextSiblingElement("resource");
  }

  return true;
};