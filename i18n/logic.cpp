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


#include <i18n/logic.h>
#include <locale/logic.h>
#include <filter/url.h>
#include <filter/google.h>
#include <filter/string.h>


void i18n_logic_augment_via_google_translate ()
{
  cout << "Not implemented yet" << endl;
  
  return;
  
  // The following assumes that Google Translate has been set up already.
  // Run $ gcloud auth application-default print-access-token.
  auto [ google_translate_ok, google_translate_output ] = filter::google::print_store_access_token ();
  cout << "Store Google Translate access token: " << (google_translate_ok?"Ok":"Fail") << endl;
  cout << google_translate_output << endl;
  if (!google_translate_ok) return;
  
  // Get the available localizations in the "locale" folder.
  map <string, string> localizations = locale_logic_localizations ();
  for (auto & element : localizations) {
    // The language abbreviation and name, like e.g. "nl" for Dutch.
    string language_abbrev = element.first;
    if (language_abbrev.empty ()) continue;
    string language_name = element.second;

    // The path to the .po file that contains the translatable strings.
    string po_path = filter_url_create_root_path ({"locale", language_abbrev + ".po"});

    // Read the current .po file.
    cout << "Reading current " << language_name << " definitions from " << po_path << endl;
    unordered_map <string, string> current_msgid_msgstr_map = locale_logic_read_msgid_msgstr (po_path);
    
    // Find the number of defined strings, and how many have been translated already.
    int translated_messages {0};
    for (const auto & message : current_msgid_msgstr_map) {
      if (!message.second.empty ()) translated_messages++;
    }
    cout << "Translated definitions " << translated_messages << " out of total " << current_msgid_msgstr_map.size() << endl;

    // The suffix for the Google translate generated content.
    string google_suffix {"_po_google.txt"};
    
    // Load the content of the translations generated by Google for this language.
    string google_translate_txt_path = filter_url_create_root_path ({"locale", language_abbrev + google_suffix});
    unordered_map <string, string> google_msgid_msgstr_map = locale_logic_read_msgid_msgstr (google_translate_txt_path);
    
    // Find the ones in the google file and insert them into the real .po file
   
    // Container for updated .po file contents.
    stringstream updated_po_contents {};
    
    // Iterate over the current messages and assemble translations.
    for (auto & message : current_msgid_msgstr_map) {
      const string & msgid = element.first;
      string & msgstr = element.second;

      // In case of the en_GB.po file,
      // copy the msgid to the msgstr, instead of requesting that from Google Translate.
      if (language_abbrev == "en_GB") {
        msgstr = msgid;
      }
      
      // Request the missing translation via Google Translate.
      else {
        
      }
      
      // Add the translation to the updated po file.
      updated_po_contents << R"(msgid ")" << msgid << R"(")" << endl;
      updated_po_contents << R"(msgstr ")" << msgstr << R"(")" << endl;
    }
    
    // Write the updated .po file to the desktop.
    string home_folder {getenv ("HOME")};
    string desktop_po_path = filter_url_create_path ({home_folder, "Desktop", language_abbrev + ".po"});
    filter_url_file_put_contents (desktop_po_path, updated_po_contents.str());

    // Add the headers to the above.
    
    // Write the update google .po file to the desktop.
  }
  
  // Give information about what to do at the end,
  // like to copy wich files back to where, and to check them first.
}
