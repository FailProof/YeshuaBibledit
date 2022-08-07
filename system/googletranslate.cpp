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


#include <system/googletranslate.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <assets/external.h>
#include <filter/roles.h>
#include <locale/translate.h>
#include <menu/logic.h>


string system_googletranslate_url ()
{
  return "system/googletranslate";
}


bool system_googletranslate_acl ([[maybe_unused]] void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string system_googletranslate (void * webserver_request)
{
//  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  string page;
  string success;
  string error;
  
  // The header.
  Assets_Header header = Assets_Header (translate("Google Translate"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view;
  
  view.set_variable ("external", assets_external_logic_link_addon ());

  // Set some feedback, if any.
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  
  page += view.render ("system", "googletranslate");
  page += Assets_Page::footer ();
  return page;
}