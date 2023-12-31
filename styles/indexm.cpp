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


#include <styles/indexm.h>
#include <assets/view.h>
#include <assets/page.h>
#include <dialog/entry.h>
#include <dialog/list.h>
#include <dialog/yes.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <tasks/logic.h>
#include <webserver/request.h>
#include <journal/index.h>
#include <database/config/user.h>
#include <database/logs.h>
#include <access/user.h>
#include <locale/translate.h>
#include <styles/sheets.h>
#include <styles/logic.h>
#include <assets/header.h>
#include <menu/logic.h>
using namespace std;


string styles_indexm_url ()
{
  return "styles/indexm";
}


bool styles_indexm_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string styles_indexm (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  string page {};
  
  Assets_Header header = Assets_Header (translate("Styles"), webserver_request);
  header.add_bread_crumb (menu_logic_settings_menu (), menu_logic_settings_text ());
  page = header.run ();
  
  Assets_View view {};
  
  Database_Styles database_styles {};
  
  string username {request->session_logic ()->currentUser ()};
  int userlevel {request->session_logic ()->currentLevel ()};
  
  if (request->post.count ("new")) {
    string name {request->post["entry"]};
    // Remove spaces at the ends of the name for the new stylesheet.
    // Because predictive keyboards can add a space to the name,
    // and the stylesheet system is not built for whitespace at the start / end of the name of the stylesheet.
    name = filter::strings::trim (name);
    vector <string> existing {database_styles.getSheets ()};
    if (find (existing.begin(), existing.end (), name) != existing.end ()) {
      page += assets_page::error (translate("This stylesheet already exists"));
    } else {
      database_styles.createSheet (name);
      database_styles.grantWriteAccess (username, name);
      styles_sheets_create_all ();
      page += assets_page::success (translate("The stylesheet has been created"));
    }
  }
  if (request->query.count ("new")) {
    Dialog_Entry dialog_entry = Dialog_Entry ("indexm", translate("Please enter the name for the new stylesheet"), string(), "new", string());
    page += dialog_entry.run();
    return page;
  }
  
  if (request->query.count ("delete")) {
    string del {request->query ["delete"]};
    if (!del.empty()) {
      string confirm {request->query ["confirm"]};
      if (confirm == "yes") {
        bool write = database_styles.hasWriteAccess (username, del);
        if (userlevel >= Filter_Roles::admin ()) write = true;
        if (write) {
          database_styles.deleteSheet (del);
          database_styles.revokeWriteAccess (string(), del);
          page += assets_page::success (translate("The stylesheet has been deleted"));
        }
      } if (confirm.empty()) {
        Dialog_Yes dialog_yes = Dialog_Yes ("indexm", translate("Would you like to delete this stylesheet?"));
        dialog_yes.add_query ("delete", del);
        page += dialog_yes.run ();
        return page;
      }
    }
  }
 
  // Delete empty sheet that may have been there.
  database_styles.deleteSheet (string());

  vector <string> sheets = database_styles.getSheets();
  stringstream sheetblock {};
  for (auto & sheet : sheets) {
    sheetblock << "<p>";
    sheetblock << sheet;
    bool editable = database_styles.hasWriteAccess (username, sheet);
    if (userlevel >= Filter_Roles::admin ()) editable = true;
    // Cannot edit the Standard stylesheet.
    if (sheet == styles_logic_standard_sheet ()) editable = false;
    if (editable) {
      sheetblock << "<a href=" << quoted ("sheetm?name=" + sheet) << ">[" << translate("edit") << "]</a>";
    }
    sheetblock << "</p>";
  }
  
  view.set_variable ("sheetblock", sheetblock.str());

  page += view.render ("styles", "indexm");
  
  page += assets_page::footer ();
  
  return page;
}
