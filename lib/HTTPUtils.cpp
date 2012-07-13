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

#include "Log.h"
#include "HTTPUtils.h"
// #include <curl/types.h>
#include <curl/easy.h>
#include "FileUtils/FileUtils.h"
#include <cctype>

using namespace std;

CHTTPHandler::CHTTPHandler()
{
  m_curlHandle = curl_easy_init();
};

CHTTPHandler::~CHTTPHandler()
{
  Cleanup();
};

void CHTTPHandler::GetURLToFILE(std::string strFilename, std::string strURL, std::string strLogin, std::string strPasswd)
{
  std::string strCacheFile = CacheFileNameFromURL(strURL);
  bool bIsTooLongUrl = strCacheFile == "cache_for_long_URL_download";
  strCacheFile = m_strCacheDir + strCacheFile;

  if (!FileExist(strCacheFile) || GetFileAge(strCacheFile) > CACHEEXPIRE)
  {
    curlURLToCache(strCacheFile, strURL, strLogin, strPasswd);
  }
  else
    CLog::Log(logINFO, "HTTPUtils: GetURLToFILE used a cached local file for URL %s to localdir %s, file was cached %d minutes ago",
              strURL.c_str(), strFilename.c_str(), GetFileAge(strCacheFile)/60);

  CopyFile(strCacheFile, strFilename);
  if (bIsTooLongUrl)
    DeleteFile(strCacheFile);
};

std::string CHTTPHandler::GetURLToSTR(std::string strURL, std::string strLogin, std::string strPasswd)
{
  std::string strBuffer;
  std::string strCacheFile = CacheFileNameFromURL(strURL);
  bool bIsTooLongUrl = strCacheFile == "cache_for_long_URL_download";
  strCacheFile = m_strCacheDir + strCacheFile;

  if (!FileExist(strCacheFile) || GetFileAge(strCacheFile) > CACHEEXPIRE)
  {
    curlURLToCache(strCacheFile, strURL, strLogin, strPasswd);
  }
  else
  {
    CLog::Log(logINFO, "HTTPUtils: GetURLToNem used a cached local file for URL %s, file was cached %d minutes ago",
              strURL.c_str(), GetFileAge(strCacheFile)/60);
  }
  strBuffer = ReadFileToStr(strCacheFile);

  if (bIsTooLongUrl)
    DeleteFile(strCacheFile);
  return strBuffer;
};

void CHTTPHandler::curlURLToCache(std::string strCacheFile, std::string strURL, std::string strLogin, std::string strPasswd)
{
  CURLcode curlResult;
  FILE *dloadfile;

    if(m_curlHandle) 
    {
      dloadfile = fopen(strCacheFile.c_str(),"wb");
      curl_easy_setopt(m_curlHandle, CURLOPT_URL, strURL.c_str());
      curl_easy_setopt(m_curlHandle, CURLOPT_WRITEFUNCTION, Write_CurlData_File);
      curl_easy_setopt(m_curlHandle, CURLOPT_USERNAME, strLogin.c_str());
      curl_easy_setopt(m_curlHandle, CURLOPT_PASSWORD, strPasswd.c_str());
      curl_easy_setopt(m_curlHandle, CURLOPT_WRITEDATA, dloadfile);
      curl_easy_setopt(m_curlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
      curl_easy_setopt(m_curlHandle, CURLOPT_SSL_VERIFYPEER, 0);

      curlResult = curl_easy_perform(m_curlHandle);

      fseek (dloadfile, 0, SEEK_END);
      size_t filesize=ftell (dloadfile);
      fclose(dloadfile);

      if (curlResult == 0)
        CLog::Log(logINFO, "HTTPUtils: curlURLToCache finished with success from URL %s to cachefile %s, read filesize: %ibytes",
                  strURL.c_str(), strCacheFile.c_str(), filesize);
      else
        CLog::Log(logERROR, "HTTPUtils: curlURLToCache finished with error: %s from URL %s to localdir %s",
                  curl_easy_strerror(curlResult), strURL.c_str(), strCacheFile.c_str());
    }
    else
      CLog::Log(logERROR, "HTTPUtils: curlURLToCache failed because Curl was not initalized");
};

void CHTTPHandler::ReInit()
{
  if (!m_curlHandle)
    m_curlHandle = curl_easy_init();
  else
    CLog::Log(logWARNING, "HTTPUtils: Trying to reinitalize an already existing Curl handle");
};

void CHTTPHandler::Cleanup()
{
  if (m_curlHandle)
  {
    curl_easy_cleanup(m_curlHandle);
    m_curlHandle = NULL;
  }
};

size_t Write_CurlData_File(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
};

size_t Write_CurlData_String(char *data, size_t size, size_t nmemb, string *buffer)
{
  size_t written = 0;
  if(buffer != NULL)
  {
    buffer -> append(data, size * nmemb);
    written = size * nmemb;
  }
  return written;
}

void CHTTPHandler::SetCacheDir(std::string strCacheDir)
{
  if (!DirExists(strCacheDir))
    MakeDir(strCacheDir);
  m_strCacheDir = strCacheDir + DirSepChar;
  CLog::Log(logINFO, "HTTPUtils: Cache directory set to: %s", strCacheDir.c_str());
};

std::string CHTTPHandler::CacheFileNameFromURL(std::string strURL)
{
  if (strURL.size() > 200)
  {
    CLog::Log(logWARNING, "HTTPUtils: Can't make HTTP cache for too long URL: %s", strURL.c_str());
    return "cache_for_long_URL_download";
  }
  std::string strResult;
  std::string strCharsToKeep = "/.=?";
  std::string strReplaceChars = "_-=?";

  std::string hexChars = "01234567890abcdef"; 

  for (std::string::iterator it = strURL.begin(); it != strURL.end(); it++)
  {
    if (isalnum(*it))
      strResult += *it;
    else
    {
      if (size_t pos = strCharsToKeep.find(*it) != std::string::npos)
        strResult += strReplaceChars[pos];
      else
      {
        strResult += "%";
        strResult += hexChars[(*it >> 4) & 0x0F];
        strResult += hexChars[*it & 0x0F];
      }
    }
  }

  return strResult;
};

