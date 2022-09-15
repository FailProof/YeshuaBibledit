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


#include <client/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <filter/date.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/general.h>
#include <database/noteactions.h>
#include <database/bibleactions.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <sendreceive/logic.h>
#include <assets/external.h>
#include <bb/logic.h>


string client_index_url ()
{
  return "client/index";
}


bool client_index_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::member ());
}


void client_index_remove_all_users (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  vector <string> existingusers {request->database_users()->get_users ()};
  for (const auto & existinguser : existingusers) {
    request->database_users()->removeUser (existinguser);
  }
}


void client_index_enable_client (void * webserver_request, const string & username, const string & password, int level)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);

  // Enable client mode upon a successful connection.
  client_logic_enable_client (true);
  
  // Remove all users from the database, and add the current one.
  client_index_remove_all_users (request);
  request->database_users ()->add_user (username, password, level, string());
  
  // Update the username and the level in the current session.
  request->session_logic ()->set_username (username);
  request->session_logic ()->currentLevel (true);
  
  // If there's pending Bible updates, send them off to the user.
  bible_logic_client_mail_pending_bible_updates (username);
  
  // Clear all pending note actions and Bible actions and settings updates.
  Database_NoteActions database_noteactions;
  Database_BibleActions database_bibleactions;
  database_noteactions.clear ();
  database_noteactions.create ();
  database_bibleactions.clear ();
  database_bibleactions.create ();
  request->session_logic ()->set_username (username);
  request->database_config_user()->setUpdatedSettings ({});
  Database_Config_General::setUnsentBibleDataTime (0);
  Database_Config_General::setUnreceivedBibleDataTime (filter::date::seconds_since_epoch ());
  
  // Set flag for first run after connecting.
  Database_Config_General::setJustConnectedToCloud (true);
  
  // Set it to repeat sync every so often.
  if (Database_Config_General::getRepeatSendReceive () == 0) {
    Database_Config_General::setRepeatSendReceive (2);
  }
  
  // Schedule a sync operation straightaway.
  sendreceive_queue_sync (-1, 0);
}


string client_index (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  Assets_View view {};
  
  if (request->query.count ("disable")) {
    client_logic_enable_client (false);
    client_index_remove_all_users (request);
    Database_Config_General::setRepeatSendReceive (0);
    Database_Config_General::setUnsentBibleDataTime (0);
    Database_Config_General::setUnreceivedBibleDataTime (0);
    Database_Config_General::setJustConnectedToCloud (false);
  }
  
  bool connect = request->post.count ("connect");
  bool demo = request->query.count ("demo");
  if (connect || demo) {

    bool proceed {true};
    
    string address {};
    if (proceed) address = request->post ["address"];
    if (demo) address = demo_address ();
    // If there's not something like "http" in the server address, then add it.
    if (address.find ("http") == string::npos) address = filter_url_set_scheme (address, false);
    if (proceed) {
      // Get schema, host and port.
      string scheme {};
      string host {};
      int port {0};
      filter_url_get_scheme_host_port (address, scheme, host, port);
      // If no address given, then that's an error.
      if (proceed) if (host.empty()) {
        view.set_variable ("error", translate ("Supply an internet address"));
        proceed = false;
      }
      // If the user entered a port number here too, then that's an error.
      if (proceed) if (port > 0) {
        view.set_variable ("error", translate ("Remove the port number from the internet address"));
        proceed = false;
      }
    }
    // Store the address.
    Database_Config_General::setServerAddress (address);
    
    int port = convert_to_int (config::logic::http_network_port ());
    if (proceed) port = convert_to_int (request->post ["port"]);
    if (demo) port = demo_port ();
    if (proceed) if (port == 0) {
      view.set_variable ("error", translate ("Supply a port number"));
      proceed = false;
    }
    Database_Config_General::setServerPort (port);
    
    string user {};
    if (proceed) user = request->post ["user"];
    if (demo) user = session_admin_credentials ();
    if (proceed) if (user.empty()) {
      view.set_variable ("error", translate ("Supply a username"));
      proceed = false;
    }
    
    string pass {};
    if (proceed) pass = request->post ["pass"];
    if (demo) pass = session_admin_credentials ();
    if (proceed) if (pass.empty()) {
      view.set_variable ("error", translate ("Supply a password"));
      proceed = false;
    }

    if (proceed) {
      string response = client_logic_connection_setup (user, md5 (pass));
      int iresponse = convert_to_int (response);
      if ((iresponse >= Filter_Roles::guest ()) && (iresponse <= Filter_Roles::admin ())) {
        // Enable client mode upon a successful connection.
        client_index_enable_client (request, user, pass, iresponse);
        // Feedback.
        view.set_variable ("success", translate("Connection is okay."));
      } else {
        view.set_variable ("error", translate ("Could not create a connection with Bibledit Cloud") + ": " + response);
      }
    }
  }

  if (client_logic_client_enabled ()) view.enable_zone ("clienton");
  else view.enable_zone ("clientoff");
  
  string address {Database_Config_General::getServerAddress ()};
  view.set_variable ("address", address);
  
  int port {Database_Config_General::getServerPort ()};
  view.set_variable ("port", convert_to_string (port));
  
  view.set_variable ("url", client_logic_link_to_cloud ("", ""));
  
  vector <string> users {request->database_users ()->get_users ()};
  for (const auto & user : users) {
    int level = request->database_users()->get_level (user);
    view.set_variable ("role", Filter_Roles::text (level));
  }
  
  view.set_variable ("demo", demo_client_warning ());

  view.set_variable ("external", assets_external_logic_link_addon ());

  if (request->query.count ("info")) {
    view.enable_zone ("info");
  }
  
  bool basic_mode {config::logic::basic_mode (request)};
  if (basic_mode) view.enable_zone("basicmode");
  
  string page {};

  // Since the role of the user may change after a successful connection to the server,
  // the menu generation in the header should be postponed till when the actual role is known.
  page = Assets_Page::header (translate ("Server"), webserver_request);
  
  page += view.render ("client", "index");
  
  page += Assets_Page::footer ();
  
  return page;
}
