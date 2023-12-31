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


#include <search/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/passage.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <search/logic.h>
#include <menu/logic.h>
#include <access/bible.h>
#include <dialog/list2.h>
using namespace std;


string search_index_url ()
{
  return "search/index";
}


bool search_index_acl (void * webserver_request)
{
  if (Filter_Roles::access_control (webserver_request, Filter_Roles::consultant ())) return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


string search_index (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  
  string bible = request->database_config_user()->getBible ();
  if (request->query.count ("b")) {
    bible = request->query ["b"];
  }
 

  bool q_is_set = request->query.count ("q");
  string q = request->query ["q"];
  

  if (request->query.count ("id")) {
    string id = request->query ["id"];

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
    text =  filter::strings::markup_words ({q}, text);
    string output = "<div>" + link + " " + text + "</div>";
    
    // Output to browser.
    return output;
  }
  

  if (q_is_set) {
    // Search in the active Bible.
    vector <Passage> passages = search_logic_search_bible_text (bible, q);
    // Output results.
    string output;
    for (auto & passage : passages) {
      if (!output.empty ()) output.append ("\n");
      output.append (passage.encode ());
    }
    return output;
  }
  
  
  // Set the user chosen Bible as the current Bible.
  if (request->post.count ("bibleselect")) {
    string bibleselect = request->post ["bibleselect"];
    request->database_config_user ()->setBible (bibleselect);
    return string();
  }


  
  string page;

  Assets_Header header = Assets_Header (translate("Search"), request);
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
  
  page += view.render ("search", "index");
  
  page += assets_page::footer ();
  
  return page;
}
