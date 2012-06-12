/*
 *      Copyright (C) 2012 Team XBMC
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

#if defined( WIN32 ) && defined( TUNE )
  #include <crtdbg.h>
  _CrtMemState startMemState;
  _CrtMemState endMemState;
#endif

#include "lib/FileUtils/FileUtils.h"
#include <string>
#include <map>
#include <list>
#include "ctime"
#include <algorithm>
#include "lib/xbmclangcodes.h"
#include "lib/CharsetUtils/CharsetUtils.h"
#include <stdio.h>

const std::string VERSION = "0.7";

char* pSourceDirectory = NULL;
bool bCheckSourceLang;

FILE * pLogFile;


std::string WorkingDir;
std::string ProjRootDir;

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
