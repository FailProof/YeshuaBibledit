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


#include <notes/bulk.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/url.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <database/notes.h>
#include <database/volatile.h>
#include <database/logs.h>
#include <database/noteassignment.h>
#include <notes/logic.h>
#include <access/bible.h>
#include <ipc/focus.h>
#include <notes/index.h>
#include <dialog/yes.h>
#include <trash/handler.h>
#include <menu/logic.h>
using namespace std;


string notes_bulk_url ()
{
  return "notes/bulk";
}


bool notes_bulk_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::translator ());
}


string notes_bulk (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Database_Notes database_notes (webserver_request);
  Notes_Logic notes_logic = Notes_Logic (webserver_request);
  Database_NoteAssignment database_noteassignment;

  
  string page;
  
  Assets_Header header = Assets_Header (translate("Bulk update"), request);
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  header.add_bread_crumb (notes_index_url (), menu_logic_consultation_notes_text ());
  page = header.run();
  
  Assets_View view;
  string success, error;

  
  vector <string> bibles = access_bible::bibles (webserver_request);
  int book = Ipc_Focus::getBook (webserver_request);
  int chapter = Ipc_Focus::getChapter (webserver_request);
  int verse = Ipc_Focus::getVerse (webserver_request);
  int passage_selector = request->database_config_user()->getConsultationNotesPassageSelector();
  int edit_selector = request->database_config_user()->getConsultationNotesEditSelector();
  int non_edit_selector = request->database_config_user()->getConsultationNotesNonEditSelector();
  string status_selector = request->database_config_user()->getConsultationNotesStatusSelector();
  string bible_selector = request->database_config_user()->getConsultationNotesBibleSelector();
  string assignment_selector = request->database_config_user()->getConsultationNotesAssignmentSelector();
  bool subscription_selector = request->database_config_user()->getConsultationNotesSubscriptionSelector();
  int severity_selector = request->database_config_user()->getConsultationNotesSeveritySelector();
  int text_selector = request->database_config_user()->getConsultationNotesTextSelector();
  string search_text = request->database_config_user()->getConsultationNotesSearchText();
  
  
  int userid = filter::strings::user_identifier (webserver_request);
  
  
  // The admin disables notes selection on Bibles, so the admin sees all notes, even notes referring to non-existing Bibles.
  if (request->session_logic ()->currentLevel () == Filter_Roles::admin ()) bibles.clear ();

  
  
  // Action to take.
  bool subscribe = request->query.count ("subscribe");
  bool unsubscribe = request->query.count ("unsubscribe");
  bool assign = request->query.count ("assign");
  bool unassign = request->query.count ("unassign");
  bool unassignme = request->query.count ("unassignme");
  bool status = request->query.count ("status");
  bool severity = request->query.count ("severity");
  bool bible = request->query.count ("bible");
  bool erase = request->query.count ("delete");
  
  
  // In case there is no relevant GET action yet,
  // that is, the first time the page gets opened,
  // assemble the list of identifiers of notes to operate on.
  // This is done to remember them as long as this page is active.
  // Thus erroneous bulk operations on notes can be rectified somewhat easier.
  if (!subscribe && !unsubscribe && !assign && !unassign && !status && !severity && !bible && !erase) {
    vector <int> identifiers = database_notes.select_notes (bibles,
                                              book,
                                              chapter,
                                              verse,
                                              passage_selector,
                                              edit_selector,
                                              non_edit_selector,
                                              status_selector,
                                              bible_selector,
                                              assignment_selector,
                                              subscription_selector,
                                              severity_selector,
                                              text_selector,
                                              search_text,
                                              -1);
    vector <string> sids;
    for (auto id : identifiers) sids.push_back (filter::strings::convert_to_string (id));
    Database_Volatile::setValue (userid, "identifiers", filter::strings::implode (sids, " "));
  }


  
  // Get the stored note identifiers from the database.
  vector <int> identifiers;
  {
    vector <string> sids = filter::strings::explode (Database_Volatile::getValue (userid, "identifiers"), ' ');
    for (auto id : sids) identifiers.push_back (filter::strings::convert_to_int (id));
  }
  
  
  string identifierlist;
  for (auto identifier : identifiers) {
    identifierlist.append (" ");
    identifierlist.append (filter::strings::convert_to_string (identifier));
  }
  
  

  if (subscribe) {
    for (auto identifier : identifiers) {
      notes_logic.subscribe (identifier);
    }
    success = translate("You subscribed to these notes");
  }
  
  
  if (unsubscribe) {
    for (auto identifier : identifiers) {
      notes_logic.unsubscribe (identifier);
    }
    success = translate("You unsubscribed from these notes");
  }
  
  
  if (assign) {
    string assignee = request->query["assign"];
    string user = request->session_logic ()->currentUser ();
    vector <string> assignees = database_noteassignment.assignees (user);
    if (in_array (assignee, assignees)) {
      for (auto identifier : identifiers) {
        if (!database_notes.is_assigned (identifier, assignee)) {
          notes_logic.assignUser (identifier, assignee);
        }
      }
    }
    success = translate("The notes were assigned to the user");
    Database_Logs::log ("Notes assigned to user " + assignee + ": " + identifierlist);
  }
  
  
  if (unassign) {
    string unassignee = request->query["unassign"];
    for (auto identifier : identifiers) {
      if (database_notes.is_assigned (identifier, unassignee)) {
        notes_logic.unassignUser (identifier, unassignee);
      }
    }
    success = translate("The notes are no longer assigned to the user");
    Database_Logs::log ("Notes unassigned from user " + unassignee + ": " + identifierlist);
  }

  
  if (unassignme) {
    string username = request->session_logic()->currentUser ();
    for (auto identifier : identifiers) {
      if (database_notes.is_assigned (identifier, username)) {
        notes_logic.unassignUser (identifier, username);
      }
    }
    success = translate("The notes are no longer assigned to you");
    Database_Logs::log ("Notes unassigned from user " + username + ": " + identifierlist);
  }

  
  if (status) {
    string new_status = request->query["status"];
    for (auto identifier : identifiers) {
      if (database_notes.get_raw_status (identifier) != new_status) {
        notes_logic.setStatus (identifier, new_status);
      }
    }
    success = translate("The status of the notes was updated");
    Database_Logs::log ("Status update of notes: " + identifierlist);
  }
  
  
  if (severity) {
    int new_severity = filter::strings::convert_to_int (request->query["severity"]);
    for (auto identifier : identifiers) {
      if (database_notes.get_raw_severity (identifier) != new_severity) {
        notes_logic.setRawSeverity (identifier, new_severity);
      }
    }
    success = translate("The severity of the notes was updated");
    Database_Logs::log ("Severity update of notes: " + identifierlist);
  }
  
  
  if (bible) {
    string new_bible = request->query["bible"];
    if (new_bible == notes_logic.generalBibleName ()) new_bible.clear();
    for (auto identifier : identifiers) {
      if (database_notes.get_bible (identifier) != new_bible) {
        notes_logic.setBible (identifier, new_bible);
      }
    }
    success = translate("The Bible of the notes was updated");
    Database_Logs::log ("Bible update of notes: " + identifierlist);
  }
  
  
  if (erase) {
    string confirm = request->query["confirm"];
    if (confirm != "yes") {
      Dialog_Yes dialog_yes = Dialog_Yes ("bulk", translate("Would you like to delete the notes?"));
      dialog_yes.add_query ("delete", "");
      page += dialog_yes.run ();
      return page;
    } else {
      for (auto identifier : identifiers) {
        notes_logic.erase (identifier); // Notifications handling.
      }
      success = translate("The notes were deleted");
    }
  }
  
  
  view.set_variable ("notescount", filter::strings::convert_to_string (identifiers.size()));

  
  bool manager = Filter_Roles::access_control (webserver_request, Filter_Roles::manager ());
  if (manager) {
    view.enable_zone ("manager");
  }

  
  view.set_variable ("success", success);
  view.set_variable ("error", error);
  
  
  page += view.render ("notes", "bulk");
  
  page += assets_page::footer ();
  
  return page;
}
