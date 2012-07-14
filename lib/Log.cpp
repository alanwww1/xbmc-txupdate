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
#include "FileUtils/FileUtils.h"

static FILE * m_pLogFile;
static int m_numWarnings;
static std::map <std::string, std::string> m_mapIdents;

using namespace std;

//CLogIdent LogIdents [4] =

CLog::CLog()
{
  m_numWarnings = 0;
}

CLog::~CLog()
{}

bool CLog::Init(std::string logfile)
{
   m_pLogFile = fopen (logfile.c_str(),"wb");
  if (m_pLogFile == NULL)
  {
    fclose(m_pLogFile);
    printf("Error creating logfile: %s\n", logfile.c_str());
    return false;
  }
  fprintf(m_pLogFile, "XBMC-TXUPDATE v%s Logfile\n\n", VERSION.c_str());

  m_mapIdents.clear();
  m_mapIdents["ProjHandler"] = "";
  m_mapIdents["ResHandler"] = "  ";
  m_mapIdents["POHandler"] = "    ";
  m_mapIdents["POUtils"] = "      ";

  return true;
};

void CLog::Log(TLogLevel loglevel, const char *format, ... )
{
  if (!m_pLogFile)
    return;

  if (loglevel == logLINEFEED)
  {
    fprintf(m_pLogFile, "\n");
    return;
  }

  if (loglevel == logWARNING)
    m_numWarnings++;

  fprintf(m_pLogFile, GetCurrTime().c_str());
  std::string strLogType;
  fprintf(m_pLogFile, "\t%s\t", listLogTypes[loglevel]);

  va_list va;
  va_start(va, format);

  std::string strFormat = format;
  for (std::map<std::string, std::string>::iterator it = m_mapIdents.begin(); it != m_mapIdents.end(); it++)
  {
    if (it->first == strFormat.substr(0, it->first.length()))
      fprintf(m_pLogFile, it->second.c_str());
  };

  vfprintf(m_pLogFile, format, va);
  fprintf(m_pLogFile, "\n");
  va_end(va);

  if (loglevel == logERROR)
    throw 1;
  return;
};

void CLog::Close()
{
  if (m_pLogFile)
    fclose(m_pLogFile);
  return;
};

void CLog::ResetWarnCounter()
{
  m_numWarnings = 0;
};

int CLog::GetWarnCount()
{
  return m_numWarnings;
};