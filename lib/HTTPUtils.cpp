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
  CURLcode curlResult;
  FILE *dloadfile;

  if(m_curlHandle) 
  {
    dloadfile = fopen(strFilename.c_str(),"wb");
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

    if (curlResult == 0)
      CLog::Log(logINFO, "HTTPUtils: GetURLToFILE finished with success from URL %s to localdir %s, read filesize: %ibytes",
                strURL.c_str(), strFilename.c_str(), filesize);
    else
      CLog::Log(logERROR, "HTTPUtils: GetURLToFILE finished with error: %s from URL %s to localdir %s",
                curl_easy_strerror(curlResult), strURL.c_str(), strFilename.c_str());

    fclose(dloadfile);
  }
  else
    CLog::Log(logERROR, "HTTPUtils: GetURLToFILE failed because Curl was not initalized");
};

std::string CHTTPHandler::GetURLToSTR(std::string strURL, std::string strLogin, std::string strPasswd)
{
  CURLcode curlResult;
  std::string strBuffer;

  if(m_curlHandle) 
  {
    curl_easy_setopt(m_curlHandle, CURLOPT_URL, strURL.c_str());
    curl_easy_setopt(m_curlHandle, CURLOPT_WRITEFUNCTION, Write_CurlData_String);
    curl_easy_setopt(m_curlHandle, CURLOPT_USERNAME, strLogin.c_str());
    curl_easy_setopt(m_curlHandle, CURLOPT_PASSWORD, strPasswd.c_str());
    curl_easy_setopt(m_curlHandle, CURLOPT_WRITEDATA, &strBuffer);
    curl_easy_setopt(m_curlHandle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(m_curlHandle, CURLOPT_SSL_VERIFYPEER, 0);

    curlResult = curl_easy_perform(m_curlHandle);

    if (curlResult == 0)
      CLog::Log(logINFO, "HTTPUtils: GetURLToMem finished with success from URL %s, read size: %ibytes", strURL.c_str(),
                strBuffer.size());
      else
        CLog::Log(logERROR, "HTTPUtils: GetURLToMem finished with error: %s from URL %s",
                  curl_easy_strerror(curlResult), strURL.c_str());
  }
  else
    CLog::Log(logERROR, "HTTPUtils: GetURLToMem failed because Curl was not initalized");

  return strBuffer;
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