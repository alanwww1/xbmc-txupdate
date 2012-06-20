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

#include "ResourceHandler.h"
#include <list>
#include <algorithm>

CResourceHandler::CResourceHandler()
{};

CResourceHandler::~CResourceHandler()
{};

bool CResourceHandler::LoadResource(std::string strResRootDir, std::string strPOsuffix)
{
  CheckResType(strResRootDir);
  GetLangsFromDir(m_langDir);

  if (m_resType == ADDON || m_resType == ADDON_NOSTRINGS || m_resType == SKIN)
    loadAddonXMLFile(strResRootDir + "addon.xml");
  else if (m_resType == CORE)
  {
    LoadCoreVersion(strResRootDir + "xbmc" + DirSepChar + "GUIInfoManager.h");
  }

  for (itmapPOFiles = m_mapPOFiles.begin(); itmapPOFiles != m_mapPOFiles.end(); itmapPOFiles++)
  {
    itmapPOFiles->second.LoadPOFile(m_langDir, itmapPOFiles->first, strPOsuffix);
  };
  return true;
};

bool CResourceHandler::GetLangsFromDir(std::string strLangDir)
{
  std::list<std::string> listDirs;
  std::list<std::string>::iterator itlistDirs;
  DIR* Dir;
  struct dirent *DirEntry;
  Dir = opendir(strLangDir.c_str());

  while((DirEntry=readdir(Dir)))
  {
    if (DirEntry->d_type == DT_DIR && DirEntry->d_name[0] != '.')
      listDirs.push_back(DirEntry->d_name);
  }
  listDirs.sort();

  for (itlistDirs = listDirs.begin(); itlistDirs != listDirs.end(); itlistDirs++)
  {
    std::string strLcode;
    CPOHandler POHandler;
    if (FindLangCode((strLcode = *itlistDirs)) != "UNKNOWN")
      m_mapPOFiles[strLcode] = POHandler;
    else
      CLog::Log(logERROR, "ResHandler: No langcode found for dirname: %s" , itlistDirs->c_str());
  }
  return true;
};

bool CResourceHandler::CreateMissingDirs (std::string strRootDir)
{
  if (!DirExists(strRootDir + "resources") && (!MakeDir(strRootDir + "resources")))
  {
    CLog::Log(logERROR, "ResHandler: Not able to create resources directory at dir: %s", strRootDir.c_str());
    return 1;
  }

  if (!DirExists(strRootDir + "resources" + DirSepChar + "language") &&
    (!MakeDir(strRootDir + "resources"+ DirSepChar + "language")))
  {
    CLog::Log(logERROR, "ResHandler: Not able to create language directory at dir: %s", (strRootDir + "resources").c_str());
    return 1;
  }

  std::string WorkingDir = strRootDir + "resources"+ DirSepChar + "language" + DirSepChar;
  for (itAddonXMLData = m_mapAddonXMLData.begin(); itAddonXMLData != m_mapAddonXMLData.end(); itAddonXMLData++)
  {
    if (!DirExists(WorkingDir + FindLang(itAddonXMLData->first)) && (!MakeDir(WorkingDir +
      FindLang(itAddonXMLData->first))))
    {
      CLog::Log(logERROR, "ResHandler: Not able to create %s language directory at dir: %s", itAddonXMLData->first.c_str(),
              WorkingDir.c_str());
      return 1;
    }
  }
  return true;
};

bool CResourceHandler::loadAddonXMLFile (std::string AddonXMLFilename)
{
  TiXmlDocument xmlAddonXML;
  std::string addonXMLEncoding;
  m_strResourceData.clear();

  if (!xmlAddonXML.LoadFile(AddonXMLFilename.c_str()))
  {
    CLog::Log(logERROR, "ResHandler: AddonXML file problem: %s %s\n", xmlAddonXML.ErrorDesc(), (AddonXMLFilename + "addon.xml").c_str());
    return false;
  }

  GetEncoding(&xmlAddonXML, addonXMLEncoding);

  TiXmlElement* pRootElement = xmlAddonXML.RootElement();

  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueTStr()!="addon")
  {
    CLog::Log(logERROR, "ResHandler: No root element called: \"addon\" or no child found in AddonXML file: %s\n",
            AddonXMLFilename.c_str());
    return false;
  }
  const char* pMainAttrId = NULL;

  pMainAttrId=pRootElement->Attribute("id");
  m_strResourceData += "# Addon Name: ";
  if (!pMainAttrId)
  {
    CLog::Log(logWARNING, "ResHandler: No addon name was available in addon.xml file: %s\n", AddonXMLFilename.c_str());
    m_strResourceData += "\"xbmc-unnamed\"\n";
  }
  else
    m_strResourceData += "\"" + ToUTF8(addonXMLEncoding, EscapeLF(pMainAttrId)) + "\"\n";

  pMainAttrId=pRootElement->Attribute("name");
  m_strResourceData = "# Addon id: ";
  if (!pMainAttrId)
  {
    CLog::Log(logWARNING, "ResHandler: No addon name was available in addon.xml file: %s\n", AddonXMLFilename.c_str());
    m_strResourceData +=  "\"unknown\"\n";
  }
  else
    m_strResourceData += "\"" + ToUTF8(addonXMLEncoding, EscapeLF(pMainAttrId)) + "\"\n";

  pMainAttrId=pRootElement->Attribute("version");
  m_strResourceData += "# Addon version: ";
  if (!pMainAttrId)
  {
    CLog::Log(logWARNING, "ResHandler: No version name was available in addon.xml file: %s\n", AddonXMLFilename.c_str());
    m_strResourceData += "\"rev_unknown\"\n";
  }
  else
    m_strResourceData += "\"" + ToUTF8(addonXMLEncoding, EscapeLF(pMainAttrId)) + "\"\n";

  pMainAttrId=pRootElement->Attribute("provider-name");
  m_strResourceData += "# Addon Provider: ";
  if (!pMainAttrId)
  {
    CLog::Log(logWARNING, "ResHandler: Warning: No addon provider was available in addon.xml file: %s\n", AddonXMLFilename.c_str());
    m_strResourceData += "\"unknown\"\n";
  }
  else
    m_strResourceData += "\"" + ToUTF8(addonXMLEncoding, EscapeLF(pMainAttrId)) + "\"\n";

  std::string strAttrToSearch = "xbmc.addon.metadata";

  const TiXmlElement *pChildElement = pRootElement->FirstChildElement("extension");
  while (pChildElement && strcmp(pChildElement->Attribute("point"), "xbmc.addon.metadata") != 0)
    pChildElement = pChildElement->NextSiblingElement("extension");

  const TiXmlElement *pChildSummElement = pChildElement->FirstChildElement("summary");
  while (pChildSummElement && pChildSummElement->FirstChild())
  {
    std::string strLang = pChildSummElement->Attribute("lang");
    if (pChildSummElement->FirstChild())
    {
      std::string strValue = EscapeLF(pChildSummElement->FirstChild()->Value());
      m_mapAddonXMLData[strLang].strSummary = ToUTF8(addonXMLEncoding, strValue);
    }
    pChildSummElement = pChildSummElement->NextSiblingElement("summary");
  }

  const TiXmlElement *pChildDescElement = pChildElement->FirstChildElement("description");
  while (pChildDescElement && pChildDescElement->FirstChild())
  {
    std::string strLang = pChildDescElement->Attribute("lang");
    if (pChildDescElement->FirstChild())
    {
      std::string strValue = EscapeLF(pChildDescElement->FirstChild()->Value());
      m_mapAddonXMLData[strLang].strDescription = ToUTF8(addonXMLEncoding, strValue);
    }
    pChildDescElement = pChildDescElement->NextSiblingElement("description");
  }

  const TiXmlElement *pChildDisclElement = pChildElement->FirstChildElement("disclaimer");
  while (pChildDisclElement && pChildDisclElement->FirstChild())
  {
    std::string strLang = pChildDisclElement->Attribute("lang");
    if (pChildDisclElement->FirstChild())
    {
      std::string strValue = EscapeLF(pChildDisclElement->FirstChild()->Value());
      m_mapAddonXMLData[strLang].strDisclaimer = ToUTF8(addonXMLEncoding, strValue);
    }
    pChildDisclElement = pChildDisclElement->NextSiblingElement("disclaimer");
  }

  return true;
};

bool CResourceHandler::GetEncoding(const TiXmlDocument* pDoc, std::string& strEncoding)
{
  const TiXmlNode* pNode=NULL;
  while ((pNode=pDoc->IterateChildren(pNode)) && pNode->Type()!=TiXmlNode::TINYXML_DECLARATION) {}
  if (!pNode) return false;
  const TiXmlDeclaration* pDecl=pNode->ToDeclaration();
  if (!pDecl) return false;
  strEncoding=pDecl->Encoding();
  if (strEncoding.compare("UTF-8") ==0 || strEncoding.compare("UTF8") == 0 ||
    strEncoding.compare("utf-8") ==0 || strEncoding.compare("utf8") == 0)
    strEncoding.clear();
  std::transform(strEncoding.begin(), strEncoding.end(), strEncoding.begin(), ::toupper);
  return !strEncoding.empty(); // Other encoding then UTF8?
};

std::string CResourceHandler::EscapeLF(const char * StrToEscape)
{
  std::string strIN(StrToEscape);
  std::string strOut;
  std::string::iterator it;
  for (it = strIN.begin(); it != strIN.end(); it++)
  {
    if (*it == '\n')
    {
      strOut.append("\\n");
      continue;
    }
    if (*it == '\r')
      continue;
    strOut += *it;
  }
  return strOut;
}

bool CResourceHandler::LoadCoreVersion(std::string filename)
{
  std::string strBuffer;
  FILE * file;

  file = fopen(filename.c_str(), "rb");
  if (!file)
    return false;

  fseek(file, 0, SEEK_END);
  int64_t fileLength = ftell(file);
  fseek(file, 0, SEEK_SET);

  if (fileLength < 10)
  {
    fclose(file);
    CLog::Log(logERROR, "ResHandler: Non valid length found for GUIInfoManager file");
    return false;
  }

  strBuffer.resize(fileLength+1);

  unsigned int readBytes =  fread(&strBuffer[1], 1, fileLength, file);
  fclose(file);

  if (readBytes != fileLength)
  {
    CLog::Log(logERROR, "ResHandler: Actual read data differs from file size, for GUIInfoManager file");
    return false;
  }

  m_strResourceData.clear();
  size_t startpos = strBuffer.find("#define VERSION_MAJOR ") + 22;
  size_t endpos = strBuffer.find_first_of(" \n\r", startpos);
  m_strResourceData += strBuffer.substr(startpos, endpos-startpos);
  m_strResourceData += ".";

  startpos = strBuffer.find("#define VERSION_MINOR ") + 22;
  endpos = strBuffer.find_first_of(" \n\r", startpos);
  m_strResourceData += strBuffer.substr(startpos, endpos-startpos);

  startpos = strBuffer.find("#define VERSION_TAG \"") + 21;
  endpos = strBuffer.find_first_of(" \n\r\"", startpos);
  m_strResourceData += strBuffer.substr(startpos, endpos-startpos);

  return true;
}

void CResourceHandler::CheckResType(std::string ResRootDir)
{
  if ((FileExist(ResRootDir + "addon.xml")) && (FileExist(ResRootDir + "resources" + DirSepChar + "language" +
    DirSepChar + "English" + DirSepChar + "strings.po")))
  {
    m_resType = ADDON;
    m_langDir = ResRootDir + DirSepChar + "resources" + DirSepChar + "language"+ DirSepChar;
  }
  else if ((FileExist(ResRootDir + "addon.xml")) && (FileExist(ResRootDir + "language" + DirSepChar + "English" +
    DirSepChar + "strings.po")))
  {
    m_resType = SKIN;
    m_langDir = ResRootDir + DirSepChar + "language"+ DirSepChar;
  }
  else if (FileExist(ResRootDir + "addon.xml"))
  {
    m_resType = ADDON_NOSTRINGS;
    m_langDir = ResRootDir + DirSepChar + "resources" + DirSepChar + "language"+ DirSepChar;
  }
  else if (FileExist(ResRootDir + "xbmc" + DirSepChar + "GUIInfoManager.h"))
  {
    m_resType = CORE;
    m_langDir = ResRootDir + DirSepChar + "language" + DirSepChar;
  }
  else
  {
    m_resType = UNKNOWN;
    m_langDir = ResRootDir;
  }
};
