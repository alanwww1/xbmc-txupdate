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
#include <curl/curl.h>
// #include <curl/types.h>
#include <curl/easy.h>


void httpGet(std::string strFilename, std::string strURL)
{
  CURL *curl;
  CURLcode res;
  FILE *dloadfile;

  curl = curl_easy_init();
  if(curl) 
  {
    dloadfile = fopen(strFilename.c_str(),"wb");
    curl_easy_setopt(curl, CURLOPT_URL, "https://raw.github.com/xbmc/xbmc/master/language/English/strings.po");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Write_CURLdata);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, dloadfile);
    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(dloadfile);
  }
};

size_t Write_CURLdata(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  size_t written = fwrite(ptr, size, nmemb, stream);
  return written;
};