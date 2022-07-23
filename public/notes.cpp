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


#include <public/notes.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <filter/css.h>
#include <webserver/request.h>
#include <database/notes.h>


string public_notes_url ()
{
  return "public/notes";
}


bool public_notes_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string public_notes (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Notes database_notes (webserver_request);

  
  string bible = request->query ["bible"];
  int book = convert_to_int (request->query ["book"]);
  int chapter = convert_to_int (request->query ["chapter"]);
  
  
  vector <int> identifiers = database_notes.select_notes ({bible}, book, chapter, 0, 1, 0, 0, "", "", "", false, -1, 0, "", -1);

  
  stringstream notesblock;
  for (auto & identifier : identifiers) {
    // Display only public notes.
    if (database_notes.get_public (identifier)) {
      notesblock << "<p class=" << quoted ("nowrap") << ">";
      string url_to_note = "note?id=" + convert_to_string (identifier);
      if (config_logic_indonesian_cloud_free_simple ()) url_to_note = "../public/" + url_to_note;
      notesblock << "<a href=" << quoted (url_to_note) << ">";
      vector <Passage> passages = database_notes.get_passages (identifier);
      string verses;
      for (auto & passage : passages) {
        if (passage.m_book != book) continue;
        if (passage.m_chapter != chapter) continue;
        if (!verses.empty ()) verses.append (" ");
        verses.append (passage.m_verse);
      }
      notesblock << verses;
      notesblock << " | ";
      string summary = database_notes.get_summary (identifier);
      notesblock << summary;
      notesblock << "</a>";
      notesblock << "</p>";
    }
  }
  
  
  return notesblock.str();
}
