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


#include <search/replace.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <database/config/general.h>
#include <access/bible.h>
#include <search/logic.h>
#include <menu/logic.h>
#include <dialog/list2.h>
using namespace std;


string search_replace_url ()
{
  return "search/replace";
}


bool search_replace_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::translator ())) return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return write;
}


string search_replace (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  string siteUrl = config::logic::site_url (webserver_request);
  
  string bible = request->database_config_user()->getBible ();
  if (request->query.count ("b")) {
    bible = request->query ["b"];
  }
  
  string searchfor = request->query ["q"];
  string replacewith = request->query ["r"];
  string id = request->query ["id"];
  
  if (!id.empty ()) {
    
    // Get the Bible and passage for this identifier.
    Passage passage = Passage::decode (id);
    string bible2 = passage.m_bible;
    int book = passage.m_book;
    int chapter = passage.m_chapter;
    string verse = passage.m_verse;
    
    // Get the plain text.
    string text = search_logic_get_bible_verse_text (bible2, book, chapter, filter::strings::convert_to_int (verse));
    
    // Format it.
    string link = filter_passage_link_for_opening_editor_at (book, chapter, verse);
    string oldtext = text;
    string newtext = filter::strings::replace (searchfor, replacewith, text);
    if (replacewith != "") newtext = filter::strings::markup_words ({replacewith}, newtext);
    
    string output =
    "<div id=\"" + filter::strings::convert_to_string (id) + "\">\n"
    "<p><a href=\"replace\"> ✔ </a> <a href=\"delete\">" + filter::strings::emoji_wastebasket () + "</a> $link</p>\n"
    "<p>" + oldtext + "</p>\n"
    "<p>" + newtext + "</p>\n"
    "</div>\n";
    
    // Output to browser.
    return output;
  }

  // Set the user chosen Bible as the current Bible.
  if (request->post.count ("bibleselect")) {
    string bibleselect = request->post ["bibleselect"];
    request->database_config_user ()->setBible (bibleselect);
    return string();
  }
  
  string page;
  
  Assets_Header header = Assets_Header (translate("Replace"), request);
  header.add_bread_crumb (menu_logic_search_menu (), menu_logic_search_text ());
  page = header.run ();
  
  Assets_View view;

  {
    string bible_html;
    vector <string> accessible_bibles = access_bible::bibles (request);
    for (auto selectable_bible : accessible_bibles) {
      bible_html = Options_To_Select::add_selection (selectable_bible, selectable_bible, bible_html);
    }
    view.set_variable ("bibleoptags", Options_To_Select::mark_selected (bible, bible_html));
  }
  view.set_variable ("bible", bible);
  
  stringstream script {};
  script << "var searchBible = " << quoted(bible) << ";";
  view.set_variable ("script", script.str());
  
  page += view.render ("search", "replace");
  
  page += assets_page::footer ();
  
  return page;
}
