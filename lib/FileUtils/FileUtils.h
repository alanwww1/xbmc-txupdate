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

#pragma once

#include <string>

#ifdef WINDOWS
  #include <direct.h>
  #define GetCurrentDir _getcwd
#else
  #include <unistd.h>
  #define GetCurrentDir getcwd
#endif

#ifdef _MSC_VER
static const char DirSepChar = '\\';
#include "dirent.h"
#else
static const char DirSepChar = '/';
#include <dirent.h>
#endif

bool MakeDir(std::string Path);
bool MakeOneDir(std::string Path);
std::string GetPath(std::string const &strFilename);
bool DirExists(std::string Path);
bool FileExist(std::string filename);
void DeleteFile(std::string filename);
void CopyFile(std::string strSourceFileName, std::string strDestFileName);
size_t GetFileAge(std::string strFileName);
std::string ReadFileToStr(std::string strFileName);
bool WriteFileFromStr(const std::string &pofilename, std::string const &strToWrite);
void ConvertStrLineEnds(std::string &strToConvert);
std::string AddSlash(std::string strIn);
std::string GetCurrTime();