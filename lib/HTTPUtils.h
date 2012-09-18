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

#ifndef HTTPUTILS_H
#define HTTPUTILS_H

#pragma once

#include <string>
#include <stdio.h>
#include <curl/curl.h>
#include "TinyXML/tinyxml.h"

struct CLoginData
{
  std::string strLogin;
  std::string strPassword;
};

class CHTTPHandler
{
public:
  CHTTPHandler();
  ~CHTTPHandler();
  void ReInit();
  std::string GetURLToSTR(std::string strURL);
  void Cleanup();
  void SetCacheDir(std::string strCacheDir);
  bool LoadCredentials (std::string CredentialsFilename);
private:
  CURL *m_curlHandle;
  std::string m_strCacheDir;
  std::string CacheFileNameFromURL(std::string strURL);
  long curlURLToCache(std::string strCacheFile, std::string strURL);
  CLoginData GetCredentials (std::string strURL);
  std::string URLEncode (std::string strURL);
  std::map<std::string, CLoginData> m_mapLoginData;
  std::map<std::string, CLoginData>::iterator itMapLoginData;
};

size_t Write_CurlData_File(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t Write_CurlData_String(char *data, size_t size, size_t nmemb, std::string *buffer);

extern CHTTPHandler g_HTTPHandler;
#endif