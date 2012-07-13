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
#pragma once

#include <string>
#include <stdio.h>
#include <curl/curl.h>

const size_t CACHEEXPIRE = 21600; // 6 hours

class CHTTPHandler
{
public:
  CHTTPHandler();
  ~CHTTPHandler();
  void ReInit();
  void GetURLToFILE(std::string strFilename, std::string strURL, std::string strLogin = "", std::string strPasswd = "");
  std::string GetURLToSTR(std::string strURL, std::string strLogin = "", std::string strPasswd = "");
  void Cleanup();
  void SetCacheDir(std::string strCacheDir);
private:
  CURL *m_curlHandle;
  std::string m_strCacheDir;
  std::string CacheFileNameFromURL(std::string strURL);
  void curlURLToCache(std::string strCacheFile, std::string strURL, std::string strLogin, std::string strPasswd);
};

size_t Write_CurlData_File(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t Write_CurlData_String(char *data, size_t size, size_t nmemb, std::string *buffer);