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

#include <string>
#include <vector>
#include <stdint.h>

enum
{
  ID_FOUND = 0, // We have an entry with a numeric (previously XML) identification number.
  MSGID_FOUND = 1, // We have a classic gettext entry with textual msgid. No numeric ID.
  MSGID_PLURAL_FOUND = 2, // We have a classic gettext entry with textual msgid in plural form.
  COMMENT_ENTRY_FOUND = 3, // We have a separate comment entry
  HEADER_FOUND = 4, // We have a header entry
  UNKNOWN_FOUND = 5 // Unknown entrytype found
};

enum Boolean
{
  ISSOURCELANG=true
};


// Struct to collect all important data of the current processed entry.
struct CPOEntry
{
  int Type;
  uint32_t numID;
  std::string msgCtxt;
  std::string msgID;
  std::string msgIDPlur;
  std::string msgStr;
  std::vector<std::string> msgStrPlural;
  std::vector<std::string> extractedComm;   // #. extracted comment
  std::vector<std::string> referenceComm;   // #: reference
  std::vector<std::string> translatorComm;  // # translator comment
  std::vector<std::string> interlineComm;   // #comment between lines
  std::string Content;
};

class CPODocument
{
public:
  CPODocument();
  ~CPODocument();

  bool LoadFile(const std::string &pofilename);
  bool GetNextEntry();
  int GetEntryType() const {return m_Entry.Type;}
  void ParseEntry();
  CPOEntry GetEntryData() const {return m_Entry;}

protected:
  std::string UnescapeString(const std::string &strInput);
  bool FindLineStart(const std::string &strToFind);
  bool ParseNumID(const std::string &strLineToCheck, size_t xIDPos);
  void ConvertLineEnds(const std::string &filename);
  bool ReadStringLine(const std::string &line, std::string * pStrToAppend, int skip);
  const bool HasPrefix(const std::string &strLine, const std::string &strPrefix);
  std::string m_strBuffer;
  size_t m_POfilelength;
  size_t m_CursorPos;
  size_t m_nextEntryPos;
  CPOEntry m_Entry;
};
