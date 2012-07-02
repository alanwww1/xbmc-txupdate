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
  m_curlHandle//  HTTPHandler.GetFile(rootDir + "test.po","https://raw.github.com/xbmc/xbmc/master/language/English/strings.po");
  = curl_easy_init();
};

CHTTPHandler::~CHTTPHandler()
{
  Cleanup();
};

void CHTTPHandler::GetFile(std::string strFilename, std::string strURL, std::string strLogin, std::string strPasswd)
{
  CURLcode curlResult;
  FILE *dloadfile;

  if(m_curlHandle) 
  {
    dloadfile = fopen(strFilename.c_str(),"wb");
    curl_easy_setopt(m_curlHandle, CURLOPT_URL, strURL.c_str());
    curl_easy_setopt(m_curlHandle, CURLOPT_WRITEFUNCTION, Write_CURLdata);
    curl_easy_setopt(m_curlHandle, CURLOPT_USERNAME, strLogin.c_str());
    curl_easy_setopt(m_curlHandle, CURLOPT_PASSWORD, strPasswd.c_str());
    curl_easy_setopt(m_curlHandle, CURLOPT_WRITEDATA, dloadfile);
    curl_easy_setopt(m_curlHandle, CURLOPT_SSL_VERIFYPEER, 0);

    curlResult = curl_easy_perform(m_curlHandle);

    if (curlResult == 0)
      CLog::Log(logINFO, "HTTPUtils: GetFile finished with success from URL %s to localdir %s", strURL.c_str(),
                strFilename.c_str());
    else
      CLog::Log(logERROR, "HTTPUtils: GetFile finished with error: %s from URL %s to localdir %s",
                curl_easy_strerror(curlResult), strURL.c_str(), strFilename.c_str());

    fclose(dloadfile);
  }
  else
    CLog::Log(logERROR, "HTTPUtils: GetFile failed because Curl was not initalized");
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

size_t Write_CURLdata(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
};