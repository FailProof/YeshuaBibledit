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


#include <checks/suppress.h>
#include <assets/view.h>
#include <assets/page.h>
#include <filter/roles.h>
#include <filter/url.h>
#include <filter/string.h>
#include <filter/md5.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/config/general.h>
#include <client/logic.h>
#include <demo/logic.h>
#include <sendreceive/logic.h>
#include <access/bible.h>
using namespace std;


string checks_suppress_url ()
{
  return "checks/suppress";
}


bool checks_suppress_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string checks_suppress (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Check database_check {};
  
  
  string page {};
  page = assets_page::header (translate ("Suppressed checking results"), webserver_request);
  Assets_View view {};
  
  
  if (request->query.count ("release")) {
    int release = filter::strings::convert_to_int (request->query["release"]);
    database_check.release (release);
    view.set_variable ("success", translate ("The check result is no longer suppressed."));
  }
  
                        
  // Get the Bibles the user has write-access to.
  vector <string> bibles {};
  {
    vector <string> all_bibles = request->database_bibles()->get_bibles ();
    for (const auto & bible : all_bibles) {
      if (access_bible::write (webserver_request, bible)) {
        bibles.push_back (bible);
      }
    }
  }
  
  
  string block {};
  const vector <Database_Check_Hit> suppressions = database_check.getSuppressions ();
  for (const auto & suppression : suppressions) {
    string bible = suppression.bible;
    // Only display entries for Bibles the user has write access to.
    if (in_array (bible, bibles)) {
      int id = suppression.rowid;
      bible = filter::strings::escape_special_xml_characters (bible);
      string passage = filter_passage_display_inline ({Passage ("", suppression.book, suppression.chapter, filter::strings::convert_to_string (suppression.verse))});
      string result = filter::strings::escape_special_xml_characters (suppression.data);
      result.insert (0, bible + " " + passage + " ");
      block.append (R"(<p style="color:grey;">)");
      block.append (R"(<a href="suppress?release=)" + filter::strings::convert_to_string (id) + R"(">)");
      block.append (filter::strings::emoji_wastebasket ());
      block.append ("</a>");
      block.append (result);
      block.append ("</p>\n");
    }
  }
  view.set_variable ("block", block);
  
  
  page += view.render ("checks", "suppress");
  page += assets_page::footer ();
  return page;
}
