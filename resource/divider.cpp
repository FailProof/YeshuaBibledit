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


#include <resource/divider.h>
#include <styles/logic.h>
#include <assets/view.h>
#include <assets/page.h>
#include <dialog/entry.h>
#include <dialog/list.h>
#include <dialog/color.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <tasks/logic.h>
#include <webserver/request.h>
#include <journal/index.h>
#include <database/config/general.h>
#include <database/config/user.h>
#include <database/logs.h>
#include <database/volatile.h>
#include <access/user.h>
#include <locale/translate.h>
#include <styles/sheets.h>
#include <assets/header.h>
#include <menu/logic.h>
#include <styles/indexm.h>
#include <resource/logic.h>
#include <resource/organize.h>
#include <sync/logic.h>
using namespace std;


string resource_divider_url ()
{
  return "resource/divider";
}


bool resource_divider_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ());
}


string resource_divider (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Rich Divider"), webserver_request);
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  
  Assets_View view;


  int userid = filter::strings::user_identifier (webserver_request);
  string key = "rich divider";


  // For administrator level default resource management purposes.
  bool is_def = false;
  if (request->query["type"] == "def") is_def = true;
  if (is_def) view.set_variable("type", "def");
  

  // Start off with default comparative resource.
  // Or keep the comparative resource now being constructed.
  bool clean_divider = true;
  if (request->query.count ("title")) clean_divider = false;
  if (request->query.count ("link")) clean_divider = false;
  if (request->query.count ("foreground")) clean_divider = false;
  if (request->query.count ("background")) clean_divider = false;
  if (request->query.count ("foreground2")) clean_divider = false;
  if (request->query.count ("background2")) clean_divider = false;
  if (request->post.count ("entry")) clean_divider = false;
  if (request->query.count ("add")) clean_divider = false;
  if (clean_divider) Database_Volatile::setValue (userid, key, resource_logic_rich_divider());
 

  string divider = Database_Volatile::getValue (userid, key);
  string title;
  string link;
  string foreground;
  string background;
  if (!resource_logic_parse_rich_divider (divider, title, link, foreground, background)) {
    title = "Divider title";
    link = "https://bibledit.org";
    foreground = "#000000";
    background = "#ffdc00";
    divider = resource_logic_assemble_rich_divider (title, link, foreground, background);
  }

  
  bool divider_edited = false;
  
  
  // The divider's title.
  if (request->query.count ("title")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("divider", translate("Please enter the title for the divider"), title, "title", "");
    if (is_def) dialog_entry.add_query ("type", request->query["type"]);
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("title")) {
    title = request->post["entry"];
    divider_edited = true;
  }

  
  // The divider's link.
  if (request->query.count ("link")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("divider", translate("Please enter the link for the divider"), link, "link", "");
    if (is_def) dialog_entry.add_query ("type", request->query["type"]);
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("link")) {
    link = request->post["entry"];
    divider_edited = true;
  }
 

  // The divider's text / foreground color.
  if (request->query.count ("foreground")) {
    Dialog_Color dialog_color = Dialog_Color ("divider", translate("Please specify a new color"));
    dialog_color.add_query ("foreground2", "true");
    if (is_def) dialog_color.add_query ("type", request->query["type"]);
    page += dialog_color.run ();
    return page;
  }
  if (request->query.count ("foreground2")) {
    string color = request->query["color"];
    if (!color.empty()) {
      foreground = color;
      if (foreground.find ("#") == string::npos) foreground.insert (0, "#");
      if (foreground.length () != 7) foreground = "#000000";
      divider_edited = true;
    }
  }

  
  // The divider's background color.
  if (request->query.count ("background")) {
    Dialog_Color dialog_color = Dialog_Color ("divider", translate("Please specify a new color"));
    dialog_color.add_query ("background2", "true");
    if (is_def) dialog_color.add_query ("type", request->query["type"]);
    page += dialog_color.run ();
    return page;
  }
  if (request->query.count ("background2")) {
    string color = request->query["color"];
    if (!color.empty()) {
      background = color;
      if (background.find ("#") == string::npos) background.insert (0, "#");
      if (background.length () != 7) background = "#FFFFFF";
      divider_edited = true;
    }
  }

  
  // Get and optionally save the new divider.
  divider = resource_logic_assemble_rich_divider (title, link, foreground, background);
  if (divider_edited) {
    Database_Volatile::setValue (userid, key, divider);
  }

  
  // Add it to the existing resources.
  if (request->query.count ("add")) {
    vector <string> resources = request->database_config_user()->getActiveResources ();
    if (is_def) resources = Database_Config_General::getDefaultActiveResources ();
    resources.push_back (divider);
    if (is_def) Database_Config_General::setDefaultActiveResources (resources);
    else request->database_config_user()->setActiveResources (resources);
    if (!is_def) request->database_config_user()->addUpdatedSetting (Sync_Logic::settings_send_resources_organization);
    redirect_browser (request, resource_organize_url ());
    return "";
  }
  

  // Render the divider in the example area.
  divider = resource_logic_get_divider (divider);
  view.set_variable ("divider", divider.substr(1));

  
  // Set the building blocks of the divider.
  view.set_variable ("title", title);
  view.set_variable ("link", link);
  view.set_variable ("foreground", foreground);
  view.set_variable ("background", background);
  
  
  page += view.render ("resource", "divider");
  
  page += assets_page::footer ();
  
  return page;
}
