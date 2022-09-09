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
#include <filter/url.h>
#include <filter/google.h>
#include <filter/shell.h>
#include <locale/translate.h>
#include <menu/logic.h>
#include <database/logs.h>


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
  
  string page {};
  string success {};
  string error {};
  
  // The header.
  Assets_Header header = Assets_Header (translate("Google Translate"), webserver_request);
  header.addBreadCrumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view {};
  
  view.set_variable ("external", assets_external_logic_link_addon ());
  
  // The location of the googletranslate.txt file with required information about setup.
  view.set_variable ("config", filter_url_create_root_path ({config::logic::config_folder (), "googletranslate.txt"}));

  // Check whether the Google Translate JSON key can be read.
  auto [ json_key, json_error ] = filter::google::get_json_key_value_error ();
  if (!json_error.empty()) Database_Logs::log(json_error);
  error.assign(json_error);

  // Check whether gcloud has been installed on the server.
  if (error.empty()) {
    bool gcloud_present = filter_shell_is_present ("gcloud");
    if (!gcloud_present) {
      error.assign("The gcloud CLI was not found on the server.");
    }
  }

  // Check whether the service account can be activated.
  if (error.empty()) {
    auto [ activate_ok, activate_output ] = filter::google::activate_service_account ();
    if (!activate_ok) error.assign(activate_output);
    Database_Logs::log (activate_output);
  }

  // Print and store the gcloud access token.
  if (error.empty()) {
    auto [ access_ok, access_token ] = filter::google::print_store_access_token ();
    if (!access_ok) error.assign(access_token);
    Database_Logs::log ("Access token: " + access_token);
  }

  // Do a translation.
  string english_text { "Jesus the Christ the Messiah" };
  string greek_text;
  if (error.empty()) {
    auto [ trans_ok, translation, trans_err ] = filter::google::translate (english_text, "en", "el");
    if (!trans_ok) error.assign(trans_err);
    if (trans_ok) greek_text = translation;
  }
  
  // Handle the OK message.
  if (error.empty()) if (json_key.length()) {
    stringstream ss;
    ss << translate("The connection to Google Translate looks good.");
    ss << " ";
    ss << translate("An example translation was made.");
    ss << " - ";
    ss << translate ("English") << ": ";
    ss << quoted(english_text);
    ss << " - ";
    ss << translate ("Greek") << ": ";
    ss << quoted(greek_text);
    success = ss.str();
  }
  
  // Set the feedback.
  view.set_variable ("success", success);
  view.set_variable ("error", error);

  page += view.render ("system", "googletranslate");
  page += Assets_Page::footer ();
  return page;
}


// One method to use Google Translate is through the Google Cloud Platform C++ Client Libraries.
// Information: https://github.com/googleapis/google-cloud-cpp
// While building this through $ cmake the build machine started to freeze up.
// It then indicated that there's was no application memory left.
// So this method is not used.
