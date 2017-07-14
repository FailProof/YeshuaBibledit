/*
Copyright (©) 2003-2017 Teus Benschop.

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


#ifndef INCLUDED_DATABASE_NOTES_H
#define INCLUDED_DATABASE_NOTES_H


#include <config/libraries.h>
#include <filter/passage.h>


class Database_Notes_Text
{
public:
  string raw;
  string localized;
};


class Database_Notes
{
public:
  Database_Notes (void * webserver_request_in);
  void create_v12 ();
  string database_path_v12 ();
  string checksums_database_path_v12 ();
  bool healthy_v12 ();
  bool checksums_healthy_v12 ();
  bool checkup_v12 ();
  bool checkup_checksums_v12 ();
  void trim_v12 ();
  void trim_server_v12 ();
  void optimize_v12 ();
  void sync ();
  void updateDatabase (int identifier);
  string main_folder_v12 ();
  string note_folder_v1 (int identifier);
  string note_file_v2 (int identifier);
  string bible_file_v1 (int identifier);
  string passage_file_v1 (int identifier);
  string status_file_v1 (int identifier);
  string severity_file_v1 (int identifier);
  string modified_file_v1 (int identifier);
  string summary_file_v1 (int identifier);
  string contents_file_v1 (int identifier);
  string subscriptions_file_v1 (int identifier);
  string assigned_file_v1 (int identifier);
  string public_file_v1 (int identifier);
  bool identifier_exists_v12 (int identifier);
  void setIdentifier (int identifier, int new_identifier);
  int getNewUniqueIdentifier ();
  vector <int> getIdentifiers ();
  int store_new_note_v1 (const string& bible, int book, int chapter, int verse, string summary, string contents, bool raw);
  int store_new_note_v2 (const string& bible, int book, int chapter, int verse, string summary, string contents, bool raw);
  vector <int> selectNotes (vector <string> bibles, int book, int chapter, int verse, int passage_selector, int edit_selector, int non_edit_selector, const string& status_selector, string bible_selector, string assignment_selector, bool subscription_selector, int severity_selector, int text_selector, const string& search_text, int limit);
  string get_summary_v1 (int identifier);
  string get_summary_v2 (int identifier);
  void set_summary_v1 (int identifier, const string& summary);
  void set_summary_v2 (int identifier, string summary);
  string get_contents_v1 (int identifier);
  string get_contents_v2 (int identifier);
  void set_contents_v1 (int identifier, const string& contents);
  void set_contents_v2 (int identifier, const string& contents);
  void erase_v12 (int identifier);
  void add_comment_v1 (int identifier, const string& comment);
  void add_comment_v2 (int identifier, const string& comment);
  void subscribe (int identifier);
  void subscribeUser (int identifier, const string& user);
  vector <string> get_subscribers_v1 (int identifier);
  vector <string> get_subscribers_v2 (int identifier);
  void set_subscribers_v1 (int identifier, vector <string> subscribers);
  void set_subscribers_v2 (int identifier, vector <string> subscribers);
  bool is_subscribed_v1 (int identifier, const string& user);
  bool is_subscribed_v2 (int identifier, const string& user);
  void unsubscribe_v1 (int identifier);
  void unsubscribe_v2 (int identifier);
  void unsubscribe_user_v1 (int identifier, const string& user);
  void unsubscribe_user_v2 (int identifier, const string& user);
  vector <string> getAllAssignees (const vector <string>& bibles);
  vector <string> getAssignees (int identifier);
  void setAssignees (int identifier, vector <string> assignees);
  void assignUser (int identifier, const string& user);
  bool isAssigned (int identifier, const string& user);
  void unassignUser (int identifier, const string& user);
  string getBible (int identifier);
  void setBible (int identifier, const string& bible);
  vector <string> getAllBibles ();
  string encodePassage (int book, int chapter, int verse);
  Passage decodePassage (string passage);
  string getRawPassage (int identifier);
  vector <Passage> getPassages (int identifier);
  void setPassages (int identifier, const vector <Passage>& passages, bool import = false);
  void setRawPassage (int identifier, const string& passage);
  string getRawStatus (int identifier);
  string getStatus (int identifier);
  void setStatus (int identifier, const string& status, bool import = false);
  vector <Database_Notes_Text> getPossibleStatuses ();
  int getRawSeverity (int identifier);
  string getSeverity (int identifier);
  void setRawSeverity (int identifier, int severity);
  vector <Database_Notes_Text> getPossibleSeverities ();
  int getModified (int identifier);
  void set_modified_v1 (int identifier, int time);
  void set_modified_v2 (int identifier, int time);
  bool getPublic (int identifier);
  void setPublic (int identifier, bool value);
  vector <int> selectDuplicateNotes (const string& rawpassage, const string& summary, const string& contents);
  void update_search_fields_v1 (int identifier);
  void update_search_fields_v2 (int identifier);
  string getSearchField (int identifier);
  vector <int> searchNotes (string search, const vector <string> & bibles);
  void markForDeletion (int identifier);
  void unmarkForDeletion (int identifier);
  bool isMarkedForDeletion (int identifier);
  void touchMarkedForDeletion ();
  vector <int> getDueForDeletion ();
  void setChecksum (int identifier, const string & checksum);
  string getChecksum (int identifier);
  void deleteChecksum (int identifier);
  void updateChecksum (int identifier);
  string getMultipleChecksum (const vector <int> & identifiers);
  vector <int> getNotesInRangeForBibles (int lowId, int highId, const vector <string> & bibles, bool anybible);
  void set_availability (bool available);
  bool available ();
  string getBulk (vector <int> identifiers);
  vector <string> setBulk (string json);
private:
  void * webserver_request;
  sqlite3 * connect ();
  sqlite3 * connect_checksums ();
  string expiryFile (int identifier);
  string assembleContents (int identifier, string contents);
  string assembleContentsV2 (int identifier, string contents);
  vector <string> getAssigneesInternal (string assignees);
  vector <string> standard_severities ();
  void note_edited_actions_v1 (int identifier);
  void note_edited_actions_v2 (int identifier);
  string getBibleSelector (vector <string> bibles);
  string availability_flag ();
  string notesSelectIdentifier ();
  string notesOptionalFulltextSearchRelevanceStatement (string search);
  string notesFromWhereStatement ();
  string notesOptionalFulltextSearchStatement (string search);
  string notesOrderByRelevanceStatement ();
  string getFieldV2 (int identifier, string key);
  void set_field_v2 (int identifier, string key, string value);
};


#endif
