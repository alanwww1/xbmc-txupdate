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

#include "JSONHandler.h"
#include "Log.h"
#include <list>
#include <stdlib.h>
#include "Settings.h"

using namespace std;

CJSONHandler::CJSONHandler()
{};

CJSONHandler::~CJSONHandler()
{};

std::map<std::string, std::string> CJSONHandler::ParseResources(std::string strJSON)
{
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;
  std::string resName, resType;
  std::map<std::string, std::string> mapResources;

  bool parsingSuccessful = reader.parse(strJSON, root );
  if ( !parsingSuccessful )
  {
    CLog::Log(logERROR, "JSONHandler: Parse resource: no valid JSON data");
    return mapResources;
  }

  for(Json::ValueIterator itr = root.begin() ; itr != root.end() ; itr++)
  {
    printf("key:%i\n", itr.key().asUInt());
    Json::Value valu = *itr;
    resName = valu.get("slug", "unknown").asString();
    resType = valu.get("category", "unknown").asString();
    if (resType == "")
    {
      if (resName.substr(0,4) == "skin")
      {
        CLog::Log(logINFO, "JSONHandler: no category was filled for resource %s on Transifex server, but from resourcename"
                  " this resource was declared as skin", resName.c_str());
        resType = "skin";
      }
      else if (resName == "xbmc-core")
      {
        CLog::Log(logINFO, "JSONHandler: no category was filled for resource %s on Transifex server, but from resourcename"
                  " this resource was declared as xbmc-core", resName.c_str());
        resType = "xbmc-core";
      }
      else
      {
        CLog::Log(logINFO, "JSONHandler: no category was filled for resource %s on Transifex server, but from resourcename"
        " this resource was declared as normal addon", resName.c_str());
        resType = "addon";
      }
    }

    if (resName.size() == 0 || resName == "unknown" ||
        resType == "unknown")
      CLog::Log(logERROR, "JSONHandler: Parse resource: no valid JSON data while iterating");
    mapResources[resName] = resType;
    CLog::Log(logINFO, "JSONHandler: found resource on Transifex server: %s, Type: %s", resName.c_str(), resType.c_str());
  };
  CLog::Log(logINFO, "JSONHandler: Found %i resources at Transifex server", mapResources.size());
  return mapResources;
};




std::list<std::string> CJSONHandler::ParseAvailLanguages(std::string strJSON)
{
  Json::Value root;   // will contains the root value after parsing.
  Json::Reader reader;
  std::string lang;
  std::list<std::string> listLangs;

  bool parsingSuccessful = reader.parse(strJSON, root );
  if ( !parsingSuccessful )
  {
    CLog::Log(logERROR, "JSONHandler: ParseAvailLanguages: no valid JSON data");
    return listLangs;
  }

  const Json::Value langs = root;
  std::string strLangsToFetch;
  std::string strLangsToDrop;

  for(Json::ValueIterator itr = langs.begin() ; itr != langs.end() ; itr++)
  {
    lang = itr.key().asString();
    if (lang == "unknown")
      CLog::Log(logERROR, "JSONHandler: ParseLangs: no language code in json data. json string:\n %s", strJSON.c_str());

    Json::Value valu = *itr;
    std::string strCompletedPerc = valu.get("completed", "unknown").asString();

    if (strtol(&strCompletedPerc[0], NULL, 10) > g_Settings.GetMinCompletion()-1)
    {
      strLangsToFetch += lang + ": " + strCompletedPerc + ", ";
      listLangs.push_back(lang);
    }
    else
      strLangsToDrop += lang + ": " + strCompletedPerc + ", ";
  };
  CLog::Log(logINFO, "JSONHandler: ParseAvailLangs: Languages to be Fetcehed: %s", strLangsToFetch.c_str());
  CLog::Log(logINFO, "JSONHandler: ParseAvailLangs: Languages to be Dropped (not enough completion): %s",
            strLangsToDrop.c_str());
  return listLangs;
};






void CJSONHandler::PrintJSONValue( Json::Value val )
{
  if( val.isString() ) {
    printf( "string(%s)", val.asString().c_str() ); 
  } else if( val.isBool() ) {
    printf( "bool(%d)", val.asBool() ); 
  } else if( val.isInt() ) {
    printf( "int(%d)", val.asInt() ); 
  } else if( val.isUInt() ) {
    printf( "uint(%u)", val.asUInt() ); 
  } else if( val.isDouble() ) {
    printf( "double(%f)", val.asDouble() ); 
  }
  else 
  {
    printf( "unknown type=[%d]", val.type() ); 
  }
}

bool CJSONHandler::PrintJSONTree( Json::Value root, unsigned short depth /* = 0 */) 
{
  depth += 1;
  printf( " {type=[%d], size=%d}", root.type(), root.size() ); 
  
  if( root.size() > 0 ) {
    printf("\n");
    for( Json::ValueIterator itr = root.begin() ; itr != root.end() ; itr++ ) {
      // Print depth. 
      for( int tab = 0 ; tab < depth; tab++) {
        printf("-"); 
      }
      printf(" subvalue(");
      PrintJSONValue(itr.key());
      printf(") -");
      PrintJSONTree( *itr, depth); 
    }
    return true;
  } else {
    printf(" ");
    PrintJSONValue(root);
    printf( "\n" ); 
  }
  return true;
}


