/*
 *      Copyright (C) 2005-2008 Team XBMC
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

#include <string>
#include "CharsetUtils.h"
#include <errno.h>
#include "../Log.h"
#include <sstream>

CCharsetUtils g_CharsetUtils;

std::string CCharsetUtils::IntToStr(int number)
{
  std::stringstream ss;//create a stringstream
  ss << number;//add number to the stream
  return ss.str();//return a string with the contents of the stream
};

std::string CCharsetUtils::UnescapeCPPString(const std::string &strInput)
{
  std::string strOutput;
  if (strInput.empty())
    return strOutput;

  char oescchar;
  strOutput.reserve(strInput.size());
  std::string::const_iterator it = strInput.begin();
  while (it < strInput.end())
  {
    oescchar = *it++;
    if (oescchar == '\\')
    {
      if (it == strInput.end())
      {
        CLog::Log(logWARNING, "CharsetUtils: Unhandled escape character at line-end. Problematic string: %s", strInput.c_str());
        continue;
      }
      switch (*it++)
      {
        case 'a':  oescchar = '\a'; break;
        case 'b':  oescchar = '\b'; break;
        case 'v':  oescchar = '\v'; break;
        case 'n':  oescchar = '\n'; break;
        case 't':  oescchar = '\t'; break;
        case 'r':  oescchar = '\r'; break;
        case '"':  oescchar = '"' ; break;
        case '0':  oescchar = '\0'; break;
        case 'f':  oescchar = '\f'; break;
        case '?':  oescchar = '\?'; break;
        case '\'': oescchar = '\''; break;
        case '\\': oescchar = '\\'; break;

        default: 
        {
          CLog::Log(logWARNING, "CharsetUtils: Unhandled escape character. Problematic string: %s",strInput.c_str());
          continue;
        }
      }
    }
    strOutput.push_back(oescchar);
  }
  return strOutput;
};

std::string CCharsetUtils::EscapeStringCPP(const std::string &strInput)
{
  std::string strOutput;
  if (strInput.empty())
    return strOutput;

  strOutput.reserve(strInput.size());
  for (std::string::const_iterator it = strInput.begin(); it != strInput.end();it++)
  {
    switch (*it)
    {
      case '\n':  strOutput += "\\n"; break;
      case '\"':  strOutput += "\\\""; break;
      case '\r':  strOutput += "\\r"; break;
      case '\\':  strOutput += "\\\\"; break;
      default: strOutput.push_back(*it);
    }
  }
  return strOutput;
};

std::string CCharsetUtils::EscapeStringXML(const std::string &strInput)
{
  std::string strOutput;
  if (strInput.empty())
    return strOutput;

  strOutput.reserve(strInput.size());
  for (std::string::const_iterator it = strInput.begin(); it != strInput.end();it++)
  {
    switch (*it)
    {
      case '\n':  strOutput += "&#10;";  break;
      case '<':   strOutput += "&lt;";   break;
      case '>':   strOutput += "&gt;";   break;
      case '&':   strOutput += "&amp;";  break;
      default: strOutput.push_back(*it);
    }
  }
  return strOutput;
};

// remove trailing and leading whitespaces
std::string CCharsetUtils::UnWhitespace(std::string strInput)
{
  int offset_end = strInput.size();
  int offset_start = 0;

  while (strInput[offset_start] == ' ')
    offset_start++; // check first non-whitespace char
  while (strInput[offset_end-1] == ' ')
    offset_end--; // check last non whitespace char

  strInput = strInput.substr(offset_start, offset_end - offset_start);
  return strInput;
}

std::string CCharsetUtils::ToUTF8(std::string strEncoding, const std::string& str)
{
  if (strEncoding.empty() || strEncoding == "utf8" || strEncoding == "UTF8" || strEncoding == "utf-8" || strEncoding == "UTF-8")
  {
    if (IsValidUTF8(str))
      return str;

    CLog::Log(logWARNING, "CharsetUtils::ToUTF8: Invalid character sequence in UTF8 string, trying windows-1252 for string: %s", str.c_str());
    strEncoding = "windows-1252"; // The most common is that user thinks that he is typing utf8, but he uses cp-1252 codepage
  }

  std::string ret;
  stringCharsetToUtf8(strEncoding, str, ret);

  if (!IsValidUTF8(ret))
    CLog::Log(logERROR, "CharsetUtils::ToUTF8: Wrong character encoding given, not able to convert to UTF8. String is: %s", str.c_str());

  return ret;
}

void CCharsetUtils::stringCharsetToUtf8(const std::string& strSourceCharset, const std::string& strSource,
                         std::string& strDest)
{
  iconv_t iconvString;
  ICONV_PREPARE(iconvString);
  convert(iconvString,UTF8_DEST_MULTIPLIER,strSourceCharset,"UTF-8",strSource,strDest);
  iconv_close(iconvString);
}

void CCharsetUtils::convert(iconv_t& type, int multiplier, const std::string& strFromCharset,
                    const std::string& strToCharset, const std::string& strSource,
                    std::string& strDest)
{
  if(!convert_checked(type, multiplier, strFromCharset, strToCharset, strSource, strDest))
    strDest = strSource;
}

bool CCharsetUtils::convert_checked(iconv_t& type, int multiplier, const std::string& strFromCharset,
                            const std::string& strToCharset, const std::string& strSource,
                            std::string& strDest)
{
  if (type == (iconv_t)-1)
  {
    type = iconv_open(strToCharset.c_str(), strFromCharset.c_str());
    if (type == (iconv_t)-1) //iconv_open failed
    {
      printf("iconv_open() failed from %s to %s", strFromCharset.c_str(), strToCharset.c_str());
      return false;
    }
  }

  if (strSource.empty())
  {
    strDest.clear(); //empty strings are easy
    return true;
  }

  //input buffer for iconv() is the buffer from strSource
  size_t      inBufSize  = (strSource.length() + 1) * sizeof(strSource[0]);
  const char* inBuf      = (const char*)strSource.c_str();

  //allocate output buffer for iconv()
  size_t      outBufSize = (strSource.length() + 1) * multiplier;
  char*       outBuf     = (char*)malloc(outBufSize);

  size_t      inBytesAvail  = inBufSize;  //how many bytes iconv() can read
  size_t      outBytesAvail = outBufSize; //how many bytes iconv() can write
  const char* inBufStart    = inBuf;      //where in our input buffer iconv() should start reading
  char*       outBufStart   = outBuf;     //where in out output buffer iconv() should start writing

  while(1)
  {
    //iconv() will update inBufStart, inBytesAvail, outBufStart and outBytesAvail
    size_t returnV = iconv_const(type, &inBufStart, &inBytesAvail, &outBufStart, &outBytesAvail);

    if ((returnV == (size_t)-1) && (errno != EINVAL))
    {
      if (errno == E2BIG) //output buffer is not big enough
      {
        //save where iconv() ended converting, realloc might make outBufStart invalid
        size_t bytesConverted = outBufSize - outBytesAvail;

        //make buffer twice as big
        outBufSize   *= 2;
        char* newBuf  = (char*)realloc(outBuf, outBufSize);
        if (!newBuf)
        {
          printf("realloc failed with buffer=%p size=%zu errno=%d(%s)",
                  outBuf, outBufSize, errno, strerror(errno));
          free(outBuf);
          return false;
        }
        outBuf = newBuf;

        //update the buffer pointer and counter
        outBufStart   = outBuf + bytesConverted;
        outBytesAvail = outBufSize - bytesConverted;

        //continue in the loop and convert the rest
      }
      else if (errno == EILSEQ) //An invalid multibyte sequence has been encountered in the input
      {
        //skip invalid byte
        inBufStart++;
        inBytesAvail--;

        //continue in the loop and convert the rest
      }
      else //iconv() had some other error
      {
        printf("iconv() failed from %s to %s, errno=%d(%s)",
               strFromCharset.c_str(), strToCharset.c_str(), errno, strerror(errno));
        free(outBuf);
        return false;
      }
    }
    else
    {
      //complete the conversion, otherwise the current data will prefix the data on the next call
      returnV = iconv_const(type, NULL, NULL, &outBufStart, &outBytesAvail);
      if (returnV == (size_t)-1)
        printf("failed cleanup errno=%d(%s)", errno, strerror(errno));

      //we're done
        break;
    }
  }

  size_t bytesWritten = outBufSize - outBytesAvail;
  strDest.clear();
  strDest.resize(bytesWritten);
  char*  dest         = &strDest[0];

  //copy the output from iconv() into the CStdString
  memcpy(dest, outBuf, bytesWritten);

//  strDest.clear();

  free(outBuf);

  return true;
}

size_t CCharsetUtils::iconv_const (void* cd, const char** inbuf, size_t *inbytesleft,
                    char* * outbuf, size_t *outbytesleft)
{
  struct iconv_param_adapter
  {
    iconv_param_adapter(const char** p) : p(p) {}
    iconv_param_adapter(char**p) : p((const char**)p) {}
    operator char**() const
    {
      return(char**)p;
    }
    operator const char**() const
    {
      return(const char**)p;
    }
    const char** p;
  };

  return iconv((iconv_t)cd, iconv_param_adapter(inbuf), inbytesleft, outbuf, outbytesleft);
}

bool CCharsetUtils::IsValidUTF8(std::string const &strToCheck)
{
  int numContBExpected =0;
  for (std::string::const_iterator it = strToCheck.begin(); it != strToCheck.end(); it++)
  {
    unsigned char ch = (unsigned char) *it;

    if (ch <= 0x7f) //We are at the ASCII range
    {
      if (numContBExpected ==0)
        continue;
      else
        return false; // we were expecting a continuation byte, but we are not getting one
    }

    if (ch==0xc0 || ch==0xc1 || ch>=0xf5) //invalid characters in an utf8 string
      return false;

    if ((ch & 0xc0) == 0xc0 && (numContBExpected !=0)) // we have a code entry start, but
      return false; // we are still expecting a continuation byte not a new code start

    if ((ch & 0xc0) == 0x80)
    {
      if (numContBExpected == 0) // we have a continuation byte, but
        return false; // we are not expecting one
      numContBExpected--;
      continue;
    }

    if ((ch & 0xe0) == 0xc0) // we have a 2byte long code entry start
      numContBExpected = 1;
    else if ((ch & 0xf0) == 0xe0) // we have a 3byte long code entry start
      numContBExpected = 2;
    else if ((ch & 0xf8) == 0xf0) // we have a 4byte long code entry start
      numContBExpected = 3;
    else if ((ch & 0xfc) == 0xf8) // we have a 5byte long code entry start which is invalid by new standards
      return false;
    else if ((ch & 0xfe) == 0xfc) // we have a 6byte long code entry start which is invalid by new standards
      return false;
  }
  return true;
};
