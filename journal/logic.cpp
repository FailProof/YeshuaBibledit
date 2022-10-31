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


#include <journal/logic.h>
#include <locale/translate.h>
#include <sendreceive/notes.h>
#include <sendreceive/bibles.h>
#include <sendreceive/settings.h>
#include <paratext/logic.h>
#include <sendreceive/changes.h>
#include <sendreceive/files.h>
#include <sendreceive/sendreceive.h>
#include <journal/index.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include <pugixml/pugixml.hpp>
#pragma GCC diagnostic pop
#include <webserver/request.h>
#include <filter/string.h>
#include <database/logs.h>
using namespace std;
using namespace pugi;


// This returns true if the $entry can be filtered out from the Journal.
bool journal_logic_filter_entry (const string& entry)
{
  if (entry.find (sendreceive_notes_sendreceive_text ()) != string::npos) return true;
  if (entry.find (sendreceive_notes_up_to_date_text ()) != string::npos) return true;
  if (entry.find (sendreceive_bibles_sendreceive_text ()) != string::npos) return true;
  if (entry.find (sendreceive_bibles_up_to_date_text ()) != string::npos) return true;
  if (entry.find (sendreceive_settings_sendreceive_text ()) != string::npos) return true;
  if (entry.find (sendreceive_settings_up_to_date_text ()) != string::npos) return true;
  if (entry.find (Paratext_Logic::synchronizeStartText ()) != string::npos) return true;
  if (entry.find (Paratext_Logic::synchronizeReadyText ()) != string::npos) return true;
  if (entry.find (sendreceive_changes_sendreceive_text ()) != string::npos) return true;
  if (entry.find (sendreceive_changes_up_to_date_text ()) != string::npos) return true;
  if (entry.find (sendreceive_files_sendreceive_text ()) != string::npos) return true;
  if (entry.find (sendreceive_files_up_to_date_text ()) != string::npos) return true;
  if (entry.find (sendreceive_sendreceive_sendreceive_text ()) != string::npos) return true;
  if (entry.find (sendreceive_sendreceive_send_text ()) != string::npos) return true;
  if (entry.find (sendreceive_sendreceive_sendreceive_ready_text ()) != string::npos) return true;
  if (entry.find (sendreceive_sendreceive_send_ready_text ()) != string::npos) return true;
  return false;
}


string journal_logic_filtered_message ()
{
  return translate ("Has been sending and receiving during the past hour");
}


string journal_logic_see_journal_for_progress ()
{
  xml_document document;
  xml_node a_node = document.append_child ("a");
  string href = "../";
  href.append (journal_index_url ());
  a_node.append_attribute ("href") = href.c_str ();
  a_node.text () = translate ("See the Journal for progress.").c_str();
  stringstream output;
  document.print (output, "", format_default);
  return output.str ();
}


