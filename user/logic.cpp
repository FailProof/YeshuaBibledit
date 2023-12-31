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


#include <user/logic.h>
#include <filter/string.h>
#include <filter/url.h>
#include <filter/md5.h>
#include <filter/date.h>
#include <filter/roles.h>
#include <database/users.h>
#include <database/config/user.h>
#include <database/config/general.h>
#include <database/logs.h>
#include <database/privileges.h>
#include <database/login.h>
#include <database/noteassignment.h>
#include <email/send.h>
#include <ldap/logic.h>
#include <webserver/request.h>
using namespace std;


void user_logic_optional_ldap_authentication (void * webserver_request, string user, string pass)
{
  if (ldap_logic_is_on ()) {
    // Query the LDAP server and log the response.
    bool ldap_okay;
    string email;
    int role;
    ldap_logic_fetch (user, pass, ldap_okay, email, role, true);
    if (ldap_okay) {
      Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
      if (request->database_users ()->usernameExists (user)) {
        // Verify and/or update the fields for the user in the local database.
        if (request->database_users ()->get_md5 (user) != md5 (pass)) {
          request->database_users ()->set_password (user, pass);
        }
        if (request->database_users ()->get_level (user) != role) {
          request->database_users ()->set_level (user, role);
        }
        if (request->database_users ()->get_email (user) != email) {
          request->database_users ()->updateUserEmail (user, email);
        }
        if (!request->database_users ()->get_enabled (user)) {
          request->database_users ()->set_enabled (user, true);
        }
      } else {
        // Enter the user into the database.
        request->database_users ()->add_user (user, pass, role, email);
      }
    }
  }
}


int user_logic_login_failure_time = 0;


bool user_logic_login_failure_check_okay ()
{
  // No time set yet: OK.
  if (!user_logic_login_failure_time) return true;
  // A login failure was recorded during this very second: Check fails.
  if (user_logic_login_failure_time == filter::date::seconds_since_epoch ()) return false;
  // Default: OK.
  return true;
}


void user_logic_login_failure_register ()
{
  // Register a login failure for the current second.
  user_logic_login_failure_time = filter::date::seconds_since_epoch ();
}


void user_logic_login_failure_clear ()
{
  // Clear login failure tracker.
  user_logic_login_failure_time = 0;
}


void user_logic_store_account_creation (string username)
{
  vector <string> account_creation_times = Database_Config_General::getAccountCreationTimes ();
  string account_creation_time = filter::strings::convert_to_string(filter::date::seconds_since_epoch()) + "|" + username;
  account_creation_times.push_back(account_creation_time);
  Database_Config_General::setAccountCreationTimes(account_creation_times);
}


void user_logic_delete_account (string user, string role, string email, string & feedback)
{
  feedback = "Deleted user " + user + " with role " + role + " and email " + email;
  Database_Logs::log (feedback, Filter_Roles::admin ());
  Database_Users database_users;
  database_users.removeUser (user);
  database_privileges_client_remove (user);
  // Also remove any privileges for this user.
  // In particular for the Bible privileges this is necessary,
  // beause if old users remain in the privileges storage,
  // then a situation where no user has any privileges to any Bible,
  // and thus all relevant users have all privileges,
  // can never be achieved again.
  DatabasePrivileges::remove_user (user);
  // Remove any login tokens the user might have had: Just to clean things up.
  Database_Login::removeTokens (user);
  // Remove any settings for the user.
  // The advantage of this is that when a user is removed, all settings are gone,
  // so when the same user would be created again, all settings will go back to their defaults.
  Database_Config_User database_config_user (nullptr);
  database_config_user.remove (user);
  // Remove note assignments for clients for this user.
  Database_NoteAssignment database_noteassignment;
  database_noteassignment.remove (user);
  // Remove the account creation time.
  vector <string> updated;
  vector <string> existing = Database_Config_General::getAccountCreationTimes ();
  for (auto line : existing) {
    vector <string> bits = filter::strings::explode(line, '|');
    if (bits.size() != 2) continue;
    if (bits[1] == user) continue;
    updated.push_back(line);
  }
  Database_Config_General::setAccountCreationTimes(updated);
}
