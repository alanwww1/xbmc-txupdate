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

#ifndef SETTINGS_H
#define SETTINGS_H

#pragma once

#include <string>

const size_t DEFAULTCACHEEXPIRE = 21600; // 6 hours
const size_t DEFAULTMINCOMPLETION = 75; // %
const std::string DEFAULTPRPJNAME = "Unknown-XBMC-project";

class CSettings
{
public:
  CSettings();
  ~CSettings();
  void SetProjectname(std::string strName);
  std::string GetProjectname();
  void SetHTTPCacheExpire(size_t exptime);
  size_t GetHTTPCacheExpire();
  void SetMinCompletion(int complperc);
  int GetMinCompletion();
private:
  size_t m_CacheExpire;
  int m_minComplPercentage;
  std::string m_strProjectName;
};

extern CSettings g_Settings;
#endif