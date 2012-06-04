/*
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://xbmc.org
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

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>

#if defined( WIN32 ) && defined( TUNE )
  #include <crtdbg.h>
  _CrtMemState startMemState;
  _CrtMemState endMemState;
#endif

#ifdef WINDOWS
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif


#include <stdio.h>
#include "tinyxml.h"
#include <string>
#include <map>
#include <list>
#include "ctime"
#include <algorithm>
#include "POUtils.h"
#include <sys/stat.h>
#include "xbmclangcodes.h"
#include "CharsetUtils.h"

const std::string VERSION = "0.7";

#ifdef _MSC_VER
char DirSepChar = '\\';
  #include "dirent.h"
#else
char DirSepChar = '/';
  #include <dirent.h>
#endif

char* pSourceDirectory = NULL;
bool bCheckSourceLang;

std::string strHeader;
FILE * pPOTFile;
FILE * pLogFile;

std::map<uint32_t, CPOEntry> mapStrings;
typedef std::map<uint32_t, CPOEntry>::iterator itStrings;

std::vector<CPOEntry> vecClassicEntries;
typedef std::vector<CPOEntry>::iterator itClassicEntries;

struct CAddonXMLEntry
{
  std::string strSummary;
  std::string strDescription;
  std::string strDisclaimer;
};

enum
{
  SKIN = 0,
  ADDON = 1,
  CORE = 2,
  ADDON_NOSTRINGS = 3,
  UNKNOWN = 4
};

std::string addonXMLEncoding;
std::map<std::string, CAddonXMLEntry> mapAddonXMLData;
std::map<std::string, CAddonXMLEntry>::iterator itAddonXMLData;
bool bhasLFWritten;
std::string WorkingDir;
std::string ProjRootDir;
std::string ProjName;
std::string ProjVersion;
std::string ProjTextName;
std::string ProjProvider;
int projType;

void PrintUsage()
{
  printf
  (
  "Usage: xbmc-xml2po -s <sourcedirectoryname> (-p <projectname>) (-v <version>)\n"
  "parameter -s <name> for source root language directory, which contains the language dirs\n"
  "parameter -p <name> for project name, eg. xbmc.skin.confluence\n"
  "parameter -v <name> for project version, eg. FRODO_GIT251373f9c3\n\n"
  );
#ifdef _MSC_VER
  printf
  (
  "Note for Windows users: In case you have whitespace or any special character\n"
  "in any of the arguments, please use apostrophe around them. For example:\n"
  "xbmc-xml2po.exe -s \"C:\\xbmc dir\\language\" -p xbmc.core -v Frodo_GIT\n"
  );
#endif
return;
};

bool MakeDir(std::string Path)
{
  #ifdef _MSC_VER
  return CreateDirectory (Path.c_str(), NULL) != 0;
  #else
  return mkdir(Path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) ==0;
  #endif
};

bool DirExists(std::string Path)
{
  #ifdef _MSC_VER
  return !(INVALID_FILE_ATTRIBUTES == GetFileAttributes(Path.c_str()) && GetLastError()==ERROR_FILE_NOT_FOUND);
  #else
  struct stat st;
  return (stat(Path.c_str(), &st) == 0);
  #endif
};

bool FileExist(std::string filename) 
{
  FILE* pfileToTest = fopen (filename.c_str(),"rb");
  if (pfileToTest == NULL)
    return false;
  fclose(pfileToTest);
  return true;
}

std::string AddSlash(std::string strIn)
{
  if (strIn[strIn.size()-1] == DirSepChar)
    return strIn;
  return strIn + DirSepChar;
}

bool LoadCoreVersion(std::string filename)
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
    printf("non valid length found for GUIInfoManager file");
    return false;
  }

  strBuffer.resize(fileLength+1);

  unsigned int readBytes =  fread(&strBuffer[1], 1, fileLength, file);
  fclose(file);

  if (readBytes != fileLength)
  {
    printf("actual read data differs from file size, for GUIInfoManager file");
    return false;
  }
  size_t startpos = strBuffer.find("#define VERSION_MAJOR ") + 22;
  size_t endpos = strBuffer.find_first_of(" \n\r", startpos);
  ProjVersion = strBuffer.substr(startpos, endpos-startpos);
  ProjVersion += ".";

  startpos = strBuffer.find("#define VERSION_MINOR ") + 22;
  endpos = strBuffer.find_first_of(" \n\r", startpos);
  ProjVersion += strBuffer.substr(startpos, endpos-startpos);

  startpos = strBuffer.find("#define VERSION_TAG \"") + 21;
  endpos = strBuffer.find_first_of(" \n\r\"", startpos);
  ProjVersion += strBuffer.substr(startpos, endpos-startpos);
  return true;
}

void WriteLF(FILE* pfile)
{
  if (!bhasLFWritten)
  {
    bhasLFWritten = true;
    fprintf (pfile, "\n");
  }
};

// we write str lines into the file
void WriteStrLine(std::string prefix, std::string linkedString, std::string encoding, FILE* pFile)
{
  WriteLF(pFile);
  linkedString = ToUTF8(encoding, linkedString);
  fprintf (pFile, "%s", prefix.c_str());
  fprintf (pFile, "\"%s\"\n", linkedString.c_str());
  return;
}

std::string EscapeLF(const char * StrToEscape)
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

bool GetEncoding(const TiXmlDocument* pDoc, std::string& strEncoding)
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
}

bool loadAddonXMLFile (std::string AddonXMLFilename)
{
  TiXmlDocument xmlAddonXML;

  if (!xmlAddonXML.LoadFile(AddonXMLFilename.c_str()))
  {
    printf ("%s %s\n", xmlAddonXML.ErrorDesc(), (AddonXMLFilename + "addon.xml").c_str());
    return false;
  }

  GetEncoding(&xmlAddonXML, addonXMLEncoding);

  TiXmlElement* pRootElement = xmlAddonXML.RootElement();

  if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueTStr()!="addon")
  {
    printf ("error: No root element called: \"addon\" or no child found in AddonXML file: %s\n",
            AddonXMLFilename.c_str());
    return false;
  }
  const char* pMainAttrId = NULL;

  pMainAttrId=pRootElement->Attribute("id");
  if (!pMainAttrId)
  {
    printf ("warning: No addon name was available in addon.xml file: %s\n", AddonXMLFilename.c_str());
    ProjName = "xbmc-unnamed";
  }
  else
    ProjName = EscapeLF(pMainAttrId);

  pMainAttrId=pRootElement->Attribute("version");
  if (!pMainAttrId)
  {
    printf ("warning: No version name was available in addon.xml file: %s\n", AddonXMLFilename.c_str());
    ProjVersion = "rev_unknown";
  }
  else
    ProjVersion = EscapeLF(pMainAttrId);

  pMainAttrId=pRootElement->Attribute("name");
  if (!pMainAttrId)
  {
    printf ("warning: No addon name was available in addon.xml file: %s\n", AddonXMLFilename.c_str());
    ProjTextName = "unknown";
  }
  else
    ProjTextName = EscapeLF(pMainAttrId);

  pMainAttrId=pRootElement->Attribute("provider-name");
  if (!pMainAttrId)
  {
    printf ("warning: No addon provider was available in addon.xml file: %s\n", AddonXMLFilename.c_str());
    ProjProvider = "unknown";
  }
  else
    ProjProvider = EscapeLF(pMainAttrId);

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
      mapAddonXMLData[strLang].strSummary = strValue;
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
      mapAddonXMLData[strLang].strDescription = strValue;
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
      mapAddonXMLData[strLang].strDisclaimer = strValue;
    }
    pChildDisclElement = pChildDisclElement->NextSiblingElement("disclaimer");
  }

  return true;
}

void WriteMultilineComment(std::vector<std::string> vecCommnts, std::string prefix)
{
  if (!vecCommnts.empty())
  {
    for (size_t i=0; i < vecCommnts.size(); i++)
    {
      WriteLF(pPOTFile);
      fprintf(pPOTFile, "%s%s\n", prefix.c_str(), vecCommnts[i].c_str());
    }
  }
  return;
}

bool WritePOFile(std::string strDir, std::string strLang)
{
  int writtenEntry = 0;
  std::string OutputPOFilename;
  size_t startpos = strHeader.find("Language: ")+10;
  size_t endpos = strHeader.find_first_of("\\ \n", startpos);
  std::string LCode = strHeader.substr(startpos, endpos-startpos);
  bool bIsForeignLang = strLang != "English";
  bhasLFWritten = false;

  OutputPOFilename = strDir + DirSepChar + strLang + DirSepChar + "strings.po.temp";

  // Initalize the output po document
  pPOTFile = fopen (OutputPOFilename.c_str(),"wb");
  if (pPOTFile == NULL)
  {
    printf("Error opening output file: %s\n", OutputPOFilename.c_str());
    return false;
  }
  printf("%s\t\t", LCode.c_str());

  size_t startPos;
  startPos = strHeader.find("msgid \"\"");
  if ((startPos = strHeader.find("# Translators")) != std::string::npos)
    strHeader = strHeader.substr(startPos);
  else if ((startPos = strHeader.find("msgid \"\"")) != std::string::npos)
    strHeader = strHeader.substr(startPos);

  fprintf(pPOTFile, "# XBMC Media Center language file\n");
  if (projType != CORE && projType != UNKNOWN)
  {
    fprintf(pPOTFile, "# Addon Name: %s\n", ProjTextName.c_str());
    fprintf(pPOTFile, "# Addon id: %s\n", ProjName.c_str());
    fprintf(pPOTFile, "# Addon version: %s\n", ProjVersion.c_str());
    fprintf(pPOTFile, "# Addon Provider: %s\n", ProjProvider.c_str());
  }

  fprintf(pPOTFile, "%s", strHeader.c_str());

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
}


void ClearCPOEntry (CPOEntry &entry)
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

bool CheckPOFile(std::string strDir, std::string strLang)
{
  CPODocument PODoc;
  if (!PODoc.LoadFile(strDir + DirSepChar + strLang + DirSepChar + "strings.po"))
    return false;
  if (PODoc.GetEntryType() != HEADER_FOUND)
    printf ("POParser: No valid header found for this language");

  strHeader = PODoc.GetEntryData().Content.substr(1);

  mapStrings.clear();
  vecClassicEntries.clear();

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
        mapStrings[currEntry.numID] = currEntry;
      else
        vecClassicEntries.push_back(currEntry);

      ClearCPOEntry(currEntry);
    }
  }

  WritePOFile(strDir, strLang);

  return true;
};

int main(int argc, char* argv[])
{
  // Basic syntax checking for "-x name" format
  while ((argc > 2) && (argv[1][0] == '-') && (argv[1][1] != '\x00') &&
    (argv[1][2] == '\x00'))
  {
    switch (argv[1][1])
    {
      case 's':
	if ((argv[2][0] == '\x00') || (argv[2][0] == '-'))
	  break;
        --argc; ++argv;
        pSourceDirectory = argv[1];
        break;
      case 'E':
        --argc; ++argv;
	bCheckSourceLang = true;
        break;
    }
    ++argv; --argc;
  }

  if (pSourceDirectory == NULL)
  {
    printf("\nWrong Arguments given !\n");
    return 1;
  }

  printf("\nXBMC-CHECKPO v%s by Team XBMC\n", VERSION.c_str());
  printf("\nResults:\n\n");
  printf("Langcode\tString match\tAuto contexts\tOutput file\n");
  printf("--------------------------------------------------------------\n");

  WorkingDir = pSourceDirectory;
  if (WorkingDir[WorkingDir.length()-1] != DirSepChar)
    WorkingDir.append(&DirSepChar);

  pLogFile = fopen ((WorkingDir + "xbmc-checkpo.log").c_str(),"wb");
  if (pLogFile == NULL)
  {
    fclose(pLogFile);
    printf("Error opening logfile: %s\n", (WorkingDir + "xbmc-checkpo.log").c_str());
    return false;
  }
  fprintf(pLogFile, "XBMC.CHECKPO v%s started", VERSION.c_str());

  ProjRootDir = pSourceDirectory;
  ProjRootDir = AddSlash(ProjRootDir);
  std::string strprojType;

  if ((FileExist(ProjRootDir + "addon.xml")) && (FileExist(ProjRootDir + "resources" + DirSepChar + "language" +
    DirSepChar + "English" + DirSepChar + "strings.po")))
  {
    projType = ADDON;
    strprojType = "Addon with translatable strings";
    WorkingDir = ProjRootDir + DirSepChar + "resources" + DirSepChar + "language"+ DirSepChar;
  }
  else if ((FileExist(ProjRootDir + "addon.xml")) && (FileExist(ProjRootDir + "language" + DirSepChar + "English" +
    DirSepChar + "strings.po")))
  {
    projType = SKIN;
    strprojType = "Skin addon";
    WorkingDir = ProjRootDir + DirSepChar + "language"+ DirSepChar;
  }
  else if (FileExist(ProjRootDir + "addon.xml"))
  {
    projType = ADDON_NOSTRINGS;
    strprojType = "Addon without any translatable strings";
    WorkingDir = ProjRootDir + DirSepChar + "resources" + DirSepChar + "language"+ DirSepChar;
  }
  else if (FileExist(ProjRootDir + "xbmc" + DirSepChar + "GUIInfoManager.h"))
  {
    projType = CORE;
    strprojType = "XBMC core";
    WorkingDir = ProjRootDir + DirSepChar + "language" + DirSepChar;
  }
  else
  {
    projType = UNKNOWN;
    strprojType = "Unknown";
    WorkingDir = ProjRootDir;
  }

  if (projType == ADDON || projType == ADDON_NOSTRINGS || projType == SKIN)
    loadAddonXMLFile(ProjRootDir + "addon.xml");
  else if (projType == CORE)
  {
    ProjTextName = "XBMC Core";
    ProjProvider = "Team XBMC";
    ProjName = "xbmc.core";
    LoadCoreVersion(ProjRootDir + "xbmc" + DirSepChar + "GUIInfoManager.h");
  }

  printf ("Project type detected:\t%s\n", strprojType.c_str());
  printf ("\nProject name:\t\t%s\n", ProjTextName.c_str());
  printf ("Project id:\t\t%s\n", ProjName.c_str());
  printf ("Project version:\t%s\n", ProjVersion.c_str());
  printf ("Project provider:\t%s\n", ProjProvider.c_str());

  if (projType == ADDON_NOSTRINGS)
  {
    if (!DirExists(ProjRootDir + "resources") && (!MakeDir(ProjRootDir + "resources")))
    {
      printf ("fatal error: not able to create resources directory at dir: %s", ProjRootDir.c_str());
      return 1;
    }
    if (!DirExists(ProjRootDir + "resources" + DirSepChar + "language") &&
      (!MakeDir(ProjRootDir + "resources"+ DirSepChar + "language")))
    {
      printf ("fatal error: not able to create language directory at dir: %s", (ProjRootDir + "resources").c_str());
      return 1;
    }
    WorkingDir = ProjRootDir + "resources"+ DirSepChar + "language" + DirSepChar;
    for (itAddonXMLData = mapAddonXMLData.begin(); itAddonXMLData != mapAddonXMLData.end(); itAddonXMLData++)
    {
      if (!DirExists(WorkingDir + FindLang(itAddonXMLData->first)) && (!MakeDir(WorkingDir +
        FindLang(itAddonXMLData->first))))
      {
        printf ("fatal error: not able to create %s language directory at dir: %s", itAddonXMLData->first.c_str(),
                WorkingDir.c_str());
        return 1;
      }
    }
  }

  DIR* Dir;
  struct dirent *DirEntry;
  Dir = opendir(WorkingDir.c_str());
  int langcounter =0;
  std::list<std::string> listDirs;
  std::list<std::string>::iterator itlistDirs;

  while((DirEntry=readdir(Dir)))
  {
    if (DirEntry->d_type == DT_DIR && DirEntry->d_name[0] != '.')
      listDirs.push_back(DirEntry->d_name);
  }
  listDirs.sort();

  for (itlistDirs = listDirs.begin(); itlistDirs != listDirs.end(); itlistDirs++)
  {
    if (CheckPOFile(WorkingDir, *itlistDirs))
    {
      std::string pofilename = WorkingDir + DirSepChar + *itlistDirs + DirSepChar + "strings.po";
      std::string bakpofilename = WorkingDir + DirSepChar + *itlistDirs + DirSepChar + "strings.po.bak";
      std::string temppofilename = WorkingDir + DirSepChar + *itlistDirs + DirSepChar + "strings.po.temp";

      rename(pofilename.c_str(), bakpofilename.c_str());
      rename(temppofilename.c_str(),pofilename.c_str());
      langcounter++;
    }
  }

  printf("\nReady. Processed %i languages.\n", langcounter+1);

//  if (bUnknownLangFound)
//    printf("\nWarning: At least one language found with unpaired language code !\n"
//      "Please edit the .po file manually and correct the language code, plurals !\n"
//      "Also please report this to alanwww1@xbmc.org if possible !\n\n");
  return 0;
}
