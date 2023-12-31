/*
 Copyright (©) 2003-2023 Teus Benschop.
 
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


#include <resource/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <sword/logic.h>
#include <demo/logic.h>
#include <resource/external.h>
#include <menu/logic.h>
#include <access/logic.h>
#include <config/globals.h>
#include <database/config/general.h>
using namespace std;


string resource_index_url ()
{
  return "resource/index";
}


bool resource_index_acl (void * webserver_request)
{
  return access_logic::privilege_view_resources (webserver_request);
}


string resource_index (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  
  bool touch = request->session_logic ()->touchEnabled ();

  
  string page;
  Assets_Header header = Assets_Header (translate("Resources"), request);
  header.set_navigator ();
  header.set_stylesheet ();
  if (touch) header.jquery_touch_on ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  Assets_View view;
  
  
  vector <string> resources = request->database_config_user()->getActiveResources ();


  // If no resources are displayed, set a default selection of them.
  // If a default selection hasn't been set by an administrator, use the
  // default set from demo.
  if (resources.empty ()) {
    vector <string> default_resources = Database_Config_General::getDefaultActiveResources ();
    if (default_resources.empty ()) resources = demo_logic_default_resources ();
    else resources = default_resources;
    request->database_config_user()->setActiveResources (resources);
  }

  
  string resourceblock;
  for (size_t i = 1; i <= resources.size (); i++) {
    resourceblock.append ("<div id=\"line" + filter::strings::convert_to_string (i) + "\" style=\"clear:both\">\n");
    string resource = resources[i - 1];
    if (!sword_logic_get_remote_module (resource).empty ()) {
      if (!sword_logic_get_installed_module (resource).empty ()) {
        resource = sword_logic_get_name (resource);
      }
    }
    resourceblock.append ("<span id=\"name" + filter::strings::convert_to_string (i) + "\" class=\"title\">" + resource + "</span>\n");
    resourceblock.append ("<span id=\"content" + filter::strings::convert_to_string (i) + "\" class=\"resource\"></span>\n");
    resourceblock.append ("<hr style=\"clear:both\">");
    resourceblock.append ("</div>\n");
  }
  view.set_variable ("resourceblock", resourceblock);
  
  
  size_t resource_count = resources.size ();
  string username = request->session_logic()->currentUser ();
  int window_position = config_globals_resource_window_positions [username];
  string script = "var resourceCount = " + filter::strings::convert_to_string (resource_count) + ";\n"
                  "var resourceWindowPosition = " + filter::strings::convert_to_string (window_position) + ";";
  config::logic::swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);
  
  
  bool can_organize_active_resources = true;
  if (can_organize_active_resources) view.enable_zone("organize");
  
  
  page += view.render ("resource", "index");
  page += assets_page::footer ();
  return page;
}


/*

 The Digital Bible Library
 https://thedigitalbiblelibrary.org/
 DBL@BibleSocieties.org
 
 Question:
 The Digital Bible Library (https://thedigitalbiblelibrary.org/) has quite an amount of Bibles. 
 Would it be any use for translators using Bibledit to be able to access that data? 
 And would there be any way, legally, to do so?
 Answer:
 Getting direct access to the Digital Bible Library is hard, 
 and only open to organizations, not individuals.
 However, I got access as eBible.org. 
 Whatever is there that I can legally get and post for you to use 
 can be found at http://eBible.org/Scriptures/.
 Conslusion:
 It is useful what eBible.org already can get and have posted on eBible.org. 
 Isn't these the Bibles that also have SWORD modules made from them on the same website? 
 If that's the case, then Bibledit is already providing access to them via those modules.

*/
