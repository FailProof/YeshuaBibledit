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


#include <resource/translated9edit.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/google.h>
#include <filter/url.h>
#include <locale/translate.h>
#include <resource/logic.h>
#include <menu/logic.h>
#include <dialog/entry.h>
#include <dialog/yes.h>
#include <database/config/general.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop
#include <resource/translated1edit.h>
#include <client/logic.h>
using namespace std;
using namespace pugi;


string resource_translated9edit_url ()
{
  return "resource/translated9edit";
}


bool resource_translated9edit_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
}


string resource_translated9edit (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  
  string page;
  Assets_Header header = Assets_Header (translate("Translated resources"), request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  Assets_View view;
  string error, success;
  

  // New translated resource handler.
  if (request->query.count ("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("translated9edit", translate("Please enter a name for the new translated resource"), "", "new", "");
    page += dialog_entry.run ();
    return page;
  }
  if (request->post.count ("new")) {
    // The title for the new resource as entered by the user.
    // Clean the title up and ensure it always starts with "Translated ".
    // This word flags the translated resource as being one of that category.
    string new_resource = request->post ["entry"];
    size_t pos = new_resource.find (resource_logic_translated_resource ());
    if (pos != string::npos) {
      new_resource.erase (pos, resource_logic_translated_resource ().length());
    }
    new_resource.insert (0, resource_logic_translated_resource ());
    vector <string> titles;
    vector <string> resources = Database_Config_General::getTranslatedResources ();
    for (auto resource : resources) {
      string title;
      if (resource_logic_parse_translated_resource (resource, &title)) {
        titles.push_back (title);
      }
    }
    if (in_array (new_resource, titles)) {
      error = translate("This translated resource already exists");
    } else if (new_resource.empty ()) {
      error = translate("Please give a name for the translated resource");
    } else {
      // Store the new resource in the list.
      string resource = resource_logic_assemble_translated_resource (new_resource);
      resources.push_back (resource);
      Database_Config_General::setTranslatedResources (resources);
      success = translate("The translated resource was created");
      // Since the default for a new resource is not to cache it,
      // add the resource to the ones not to be cached by the client.
      client_logic_no_cache_resource_add (new_resource);
      // Redirect the user to the place where to edit that new resource.
      string url = resource_translated1edit_url () + "?name=" + new_resource;
      redirect_browser (webserver_request, url);
      return string();
    }
  }

  
  // Delete resource. 
  string title2remove = request->query ["delete"];
  if (!title2remove.empty()) {
    string confirm = request->query ["confirm"];
    if (confirm.empty()) {
      Dialog_Yes dialog_yes = Dialog_Yes ("translated9edit", translate("Would you like to delete this resource?"));
      dialog_yes.add_query ("delete", title2remove);
      page += dialog_yes.run ();
      return page;
    } if (confirm == "yes") {
      vector <string> updated_resources;
      vector <string> existing_resources = Database_Config_General::getTranslatedResources ();
      for (auto resource : existing_resources) {
        string title;
        resource_logic_parse_translated_resource (resource, &title);
        if (title != title2remove) updated_resources.push_back (resource);
      }
      Database_Config_General::setTranslatedResources (updated_resources);
      client_logic_no_cache_resource_remove (title2remove);
      success = translate ("The resource was deleted");
    }
  }


  vector <string> resources = Database_Config_General::getTranslatedResources ();
  {
    xml_document document;
    for (auto & resource : resources) {
      string title;
      if (!resource_logic_parse_translated_resource (resource, &title)) continue;
      xml_node p_node = document.append_child ("p");
      xml_node a_node = p_node.append_child("a");
      string href = "translated1edit?name=" + title;
      a_node.append_attribute ("href") = href.c_str();
      title.append (" [" + translate("edit") + "]");
      a_node.text().set (title.c_str());
    }
    stringstream resourceblock;
    document.print (resourceblock, "", format_raw);
    view.set_variable ("resourceblock", resourceblock.str ());
  }

  
  // If Google Translate has not yet been set up, then enable a bit of information about that.
  // Do this only in the Cloud.
#ifdef HAVE_CLOUD
  auto [ json_key, json_error ] = filter::google::get_json_key_value_error ();
  if (json_key.empty()) view.enable_zone("setup_translated");
#endif

   
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  page += view.render ("resource", "translated9edit");
  page += assets_page::footer ();
  return page;
}
