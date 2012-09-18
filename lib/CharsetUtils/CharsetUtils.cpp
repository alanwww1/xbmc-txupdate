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

std::string EscapeLF(const char * StrToEscape)
{
  std::string strIN(StrToEscape);
  std::string strOut;
  std::string::iterator it;
  for (it = strIN.begin(); it != strIN.end(); it++)
  {
    if (*it == '\n')
    {
      strOut.append("\\n");
      continue;
    }
    if (*it == '\r')
      continue;
    strOut += *it;
  }
  return strOut;
}

// remove trailing and leading whitespaces
std::string UnWhitespace(std::string strInput)
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

std::string ToUTF8(const std::string& strEncoding, const std::string& str)
{
  if (strEncoding.empty())
    return str;

  std::string ret;
  stringCharsetToUtf8(strEncoding, str, ret);
  return ret;
}

void stringCharsetToUtf8(const std::string& strSourceCharset, const std::string& strSource,
                         std::string& strDest)
{
  {
    iconv_t iconvString;
    ICONV_PREPARE(iconvString);
    convert(iconvString,UTF8_DEST_MULTIPLIER,strSourceCharset,"UTF-8",strSource,strDest);
    iconv_close(iconvString);
  }
}

void convert(iconv_t& type, int multiplier, const std::string& strFromCharset,
                    const std::string& strToCharset, const std::string& strSource,
                    std::string& strDest)
{
  if(!convert_checked(type, multiplier, strFromCharset, strToCharset, strSource, strDest))
    strDest = strSource;
}

bool convert_checked(iconv_t& type, int multiplier, const std::string& strFromCharset,
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

size_t iconv_const (void* cd, const char** inbuf, size_t *inbytesleft,
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