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


#include <client/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/roles.h>
#include <filter/md5.h>
#include <database/config/general.h>
#include <database/users.h>
#include <database/usfmresources.h>
#include <database/logs.h>
#include <sync/setup.h>
#include <client/index.h>
#include <locale/translate.h>
#include <assets/external.h>
#include <database/logic.h>
using namespace std;


// Returns whether Client mode is enabled.
bool client_logic_client_enabled ()
{
#ifndef HAVE_CLIENT
  return false;
#endif
  return Database_Config_General::getClientMode ();
}


// Sets the Client mode.
// $enable: boolean: true or false.
void client_logic_enable_client (bool enable)
{
  Database_Config_General::setClientMode (enable);
}


// Generates a URL for connecting to Bibledit Cloud.
// $address is the website.
// $port is the port number.
// $path is the path after the website.
string client_logic_url (const string & address, int port, const string & path)
{
  return address + ":" + filter::strings::convert_to_string (port) + "/" + path;
}


// This function does the initial connection from the client to the server.
// It receives settings from the server and applies them to the client.
// It returns the level of the user.
// It returns an empty string in case of failure or the response from the server.
string client_logic_connection_setup (string user, string hash)
{
  Database_Users database_users {};
  
  if (user.empty ()) {
    vector <string> users = database_users.get_users ();
    if (users.empty()) return string();
    user = users [0];
    hash = database_users.get_md5 (user);
  }
  
  string encoded_user = filter::strings::bin2hex (user);
  
  string address = Database_Config_General::getServerAddress ();
  int port = Database_Config_General::getServerPort ();
  
  string url = client_logic_url (address, port, sync_setup_url ()) + "?user=" + encoded_user + "&pass=" + hash;
  
  string error {};
  string response = filter_url_http_get (url, error, true);
  int iresponse = filter::strings::convert_to_int (response);
  
  if ((iresponse >= Filter_Roles::guest ()) && (iresponse <= Filter_Roles::admin ())) {
    // Set user's role on the client to be the same as on the server.
    // Do this only when it differs, to prevent excessive database writes on the client.
    int level = database_users.get_level (user);
    if (iresponse != level) {
      database_users.set_level (user, iresponse);
    }
  } else {
    Database_Logs::log (error, Filter_Roles::translator ());
    // In case Bibledit Cloud requires the client to connect through https,
    // and the client connects through http,
    // it will give a response code 426 plus text.
    // So in such a case clarify the meaning of that to the user.
    // https://github.com/bibledit/cloud/issues/829.
    string upgrade_required = filter_url_http_response_code_text (426);
    size_t pos = error.find (upgrade_required);
    if (pos != string::npos) {
      // Since the error code ends without a full stop, add a full stop to it first.
      error.append (". ");
      // Add a good explanation to the error code so the user knows what to do if this error occurs.
      error.append ("Bibledit Cloud requires the client to connect via the secure https protocol. The client now tried to connect through the insecure http protocol. If connected, please disconnect from Bibledit Cloud and connect again via https. Use the secure port number instead of the insecure port number. Usually the secure port number is the insecure port number plus one.");
      Database_Logs::log (error, Filter_Roles::translator ());
    }
  }
  
  if (response.empty ()) response = error;
  return response;
}


string client_logic_create_note_encode (const string & bible, int book, int chapter, int verse,
                                        const string & summary, const string & contents, bool raw)
{
  vector <string> data {};
  data.push_back (bible);
  data.push_back (filter::strings::convert_to_string (book));
  data.push_back (filter::strings::convert_to_string (chapter));
  data.push_back (filter::strings::convert_to_string (verse));
  data.push_back (summary);
  data.push_back (filter::strings::convert_to_string (raw));
  data.push_back (contents);
  return filter::strings::implode (data, "\n");
}


void client_logic_create_note_decode (const string & data,
                                      string& bible, int& book, int& chapter, int& verse,
                                      string& summary, string& contents, bool& raw)
{
  vector <string> lines = filter::strings::explode (data, '\n');
  if (!lines.empty ()) {
    bible = lines [0];
    lines.erase (lines.begin());
  }
  if (!lines.empty ()) {
    book = filter::strings::convert_to_int (lines [0]);
    lines.erase (lines.begin());
  }
  if (!lines.empty ()) {
    chapter = filter::strings::convert_to_int (lines [0]);
    lines.erase (lines.begin());
  }
  if (!lines.empty ()) {
    verse = filter::strings::convert_to_int (lines [0]);
    lines.erase (lines.begin());
  }
  if (!lines.empty ()) {
    summary = lines [0];
    lines.erase (lines.begin());
  }
  if (!lines.empty ()) {
    raw = filter::strings::convert_to_bool (lines [0]);
    lines.erase (lines.begin());
  }
  contents = filter::strings::implode (lines, "\n");
}


// This provides a html link to Bibledit Cloud / $path.
// It displays the $linktext.
string client_logic_link_to_cloud (string path, string linktext)
{
  string url {};
  string external {};
  if (client_logic_client_enabled ()) {
    string address = Database_Config_General::getServerAddress ();
    int port = Database_Config_General::getServerPort ();
    url = address + ":" + filter::strings::convert_to_string (port);
    if (!path.empty ()) {
      url.append ("/");
      url.append (path);
    }
    external = " " + assets_external_logic_link_addon ();
  } else {
    // Client disconnected: Provide the link and the text to connect to the Cloud.
    url = "/" + client_index_url ();
    linktext.append (" ");
    linktext.append (translate("You are not yet connected to Bibledit Cloud."));
    linktext.append (" ");
    linktext.append (translate("Connect."));
  }

  if (linktext.empty ()) {
    // Empty link text: Select the link itself as the text to display.
    linktext = url;
  }
  
  stringstream link {};
  link << "<a href=" << quoted(url) << external << ">" << linktext << "</a>";
  return link.str();
}


// Path to the file in the client files area that contains a list of USFM resources on the server.
string client_logic_usfm_resources_path ()
{
  return filter_url_create_root_path ({database_logic_databases (), "client", "usfm_resources.txt"});
}


void client_logic_usfm_resources_update ()
{
  // The Cloud stores the list of USFM resources.
  // It is stored in the client files area.
  // Clients can access it from there.
  string path = client_logic_usfm_resources_path ();
  Database_UsfmResources database_usfmresources {};
  vector <string> resources = database_usfmresources.getResources ();
  filter_url_file_put_contents (path, filter::strings::implode (resources, "\n"));
}


vector <string> client_logic_usfm_resources_get ()
{
  string contents = filter_url_file_get_contents (client_logic_usfm_resources_path ());
  return filter::strings::explode (contents, '\n');
}


string client_logic_get_username ()
{
  // Set the user name to the first one in the database.
  // Or if the database has no users, make the user admin.
  // That happens when disconnected from the Cloud.
  string user = session_admin_credentials ();
  Database_Users database_users;
  vector <string> users = database_users.get_users ();
  if (!users.empty()) user = users [0];
  return user;
}


string client_logic_no_cache_resources_path ()
{
  return filter_url_create_root_path ({database_logic_databases (), "client", "no_cache_resources.txt"});
}


void client_logic_no_cache_resources_save (vector<string> resources)
{
  string contents = filter::strings::implode(resources, "\n");
  string path = client_logic_no_cache_resources_path ();
  filter_url_file_put_contents(path, contents);
}


void client_logic_no_cache_resource_add (string name)
{
  vector <string> resources = client_logic_no_cache_resources_get();
  if (in_array(name, resources)) return;
  resources.push_back(name);
  client_logic_no_cache_resources_save(resources);
}


void client_logic_no_cache_resource_remove (string name)
{
  vector <string> resources = client_logic_no_cache_resources_get();
  if (!in_array(name, resources)) return;
  resources = filter::strings::array_diff(resources, {name});
  client_logic_no_cache_resources_save(resources);
}


vector <string> client_logic_no_cache_resources_get ()
{
  string contents = filter_url_file_get_contents (client_logic_no_cache_resources_path());
  vector<string> resources = filter::strings::explode(contents, "\n");
  return resources;
}
