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

#include "FileUtils.h"
#include <stdio.h>
#include <sys/stat.h>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include "../Log.h"

using namespace std;

bool MakeDir(std::string Path)
{
  #ifdef _MSC_VER
  return CreateDirectory (Path.c_str(), NULL) != 0;
  #else
  return mkdir(Path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
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
};

void DeleteFile(std::string filename)
{
  if (remove(filename.c_str()) != 0)
    CLog::Log(logERROR, "FileUtils: DeleteFile: unable to delete file: %s", filename.c_str());
};

std::string AddSlash(std::string strIn)
{
  if (strIn[strIn.size()-1] == DirSepChar)
    return strIn;
  return strIn + DirSepChar;
};

std::string GetCurrTime()
{
  std::string strTime(64, '\0');
  time_t now = std::time(0);
  struct std::tm* gmtm = std::gmtime(&now);

  if (gmtm != NULL)
  {
    sprintf(&strTime[0], "%04i-%02i-%02i %02i:%02i+0000", gmtm->tm_year + 1900, gmtm->tm_mon + 1,
            gmtm->tm_mday, gmtm->tm_hour, gmtm->tm_min);
  }
  return strTime;
};

void CopyFile(std::string strSourceFileName, std::string strDestFileName)
{
  ifstream source(strSourceFileName.c_str(), std::ios::binary);
  ofstream dest(strDestFileName.c_str(), std::ios::binary);

  dest << source.rdbuf();

  source.close();
  dest.close();
};

size_t GetFileAge(std::string strFileName)
{
  struct stat b;
  if (!stat(strFileName.c_str(), &b)) 
  {
    time_t now = std::time(0);
    return now-b.st_mtime;
  }
  else
  {
    CLog::Log(logWARNING, "FileUtils: Unable to determine the last modify date for file: %s", strFileName.c_str());
    return 0;
  }
};

std::string ReadFileToStr(std::string strFileName)
{
  FILE * file;
  std::string strRead;
  file = fopen(strFileName.c_str(), "rb");
  if (!file)
    CLog::Log(logERROR, "FileUtils: ReadFileToStr: unable to read file: %s", strFileName.c_str());

  fseek(file, 0, SEEK_END);
  int64_t fileLength = ftell(file);
  fseek(file, 0, SEEK_SET);

  strRead.resize(static_cast<size_t> (fileLength));

  unsigned int readBytes =  fread(&strRead[0], 1, fileLength, file);
  fclose(file);

  if (readBytes != fileLength)
  {
    CLog::Log(logERROR, "POParser: actual read data differs from file size, for string file: %s",strFileName.c_str());
  }
  return strRead;
};