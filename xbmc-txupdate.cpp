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

#include <string>
#include <stdio.h>
#include "lib/ProjectHandler.h"
#include "lib/HTTPUtils.h"
#include "lib/xbmclangcodes.h"

using namespace std;

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
  char* pSourceDirectory = NULL;

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
//      case 't':
//        --argc; ++argv;
//        bFetchMissingLanguagesFromTX = true;
//        break;
    }
    ++argv; --argc;
  }

  if (pSourceDirectory == NULL)
  {
    printf("\nWrong Arguments given !\n");
    return 1;
  }

  printf("\nXBMC-TXUPDATE v%s by Team XBMC\n\n", VERSION.c_str());
//  printf("\nResults:\n\n");
//  printf("Langcode\tString match\tAuto contexts\tOutput file\n");
//  printf("--------------------------------------------------------------\n");

  std::string WorkingDir = pSourceDirectory;
  if (WorkingDir[WorkingDir.length()-1] != DirSepChar)
    WorkingDir.append(&DirSepChar);

  CLog::Init(WorkingDir + "xbmc-txupdate.log");
  CLog::Log(logINFO, "Root Directory: %s", WorkingDir.c_str());

  g_HTTPHandler.LoadCredentials(WorkingDir + ".passwords.xml");
  g_HTTPHandler.SetCacheDir(WorkingDir + ".httpcache");

  g_LCodeHandler.Init("https://raw.github.com/transifex/transifex/master/transifex/languages/fixtures/all_languages.json");

  CProjectHandler TXProject;
  TXProject.InitUpdateXMLHandler(WorkingDir);
  TXProject.FetchResourcesFromTransifex(WorkingDir);
  TXProject.WriteResourcesToFile(WorkingDir, ".dtx");
  TXProject.SaveUpdateXML(WorkingDir);

  printf("Warnings: %i\n", CLog::GetWarnCount());
//  if (bUnknownLangFound)
//    printf("\nWarning: At least one language found with unpaired language code !\n"
//      "Please edit the .po file manually and correct the language code, plurals !\n"
//      "Also please report this to alanwww1@xbmc.org if possible !\n\n");
  return 0;
}
