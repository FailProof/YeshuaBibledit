/*
 Copyright (©) 2003-2022 Teus Benschop.
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <editor/style.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <webserver/request.h>
#include <editor/styles.h>
#include <access/bible.h>


string editor_style_url ()
{
  return "editor/style";
}


bool editor_style_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  auto [ read, write ] = AccessBible::Any (webserver_request);
  return read;
}


string editor_style (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  
  if (request->query.count ("style")) {
    string style = request->query["style"];
    Editor_Styles::recordUsage (request, style);
    string action = Editor_Styles::getAction (request, style);
    return style + "\n" + action;
  }
  
  
  if (request->query.count ("all")) {
    return Editor_Styles::getAll (request);
  }
  
  
  return Editor_Styles::getRecentlyUsed (request);
}

