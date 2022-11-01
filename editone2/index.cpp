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


#include <editone2/index.h>
#include <assets/view.h>
#include <assets/page.h>
#include <assets/header.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/css.h>
#include <filter/url.h>
#include <filter/indonesian.h>
#include <webserver/request.h>
#include <locale/translate.h>
#include <locale/logic.h>
#include <access/bible.h>
#include <database/config/bible.h>
#include <database/config/general.h>
#include <fonts/logic.h>
#include <navigation/passage.h>
#include <dialog/list.h>
#include <dialog/list2.h>
#include <ipc/focus.h>
#include <menu/logic.h>
#include <bb/logic.h>
#include <config/globals.h>
#include <workspace/logic.h>
#include <demo/logic.h>
using namespace std;


string editone2_index_url ()
{
  return "editone2/index";
}


bool editone2_index_acl (void * webserver_request)
{
  // Default minimum role for getting access.
  int minimum_role = Filter_Roles::translator ();
  if (config::logic::indonesian_cloud_free ()) {
    minimum_role = Filter_Roles::consultant ();
  }
  if (Filter_Roles::access_control (webserver_request, minimum_role)) return true;
  auto [ read, write ] = access_bible::any (webserver_request);
  return read;
}


string editone2_index (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  
  bool touch = request->session_logic ()->touchEnabled ();
  
  if (request->query.count ("switchbook") && request->query.count ("switchchapter")) {
    int switchbook = convert_to_int (request->query ["switchbook"]);
    int switchchapter = convert_to_int (request->query ["switchchapter"]);
    Ipc_Focus::set (request, switchbook, switchchapter, 1);
    Navigation_Passage::record_history (request, switchbook, switchchapter, 1);
  }

  if (config::logic::indonesian_cloud_free ()) {
    // See issue https://github.com/bibledit/cloud/issues/503
    // Specific configuration for the Indonesian free Cloud instance.
    // The name of the default Bible in the Translate tab will be another Bible than AlkitabKita.
    // Standard it will be Terjemahanku (My Translation).
    // When the user changed that to another name, the editor will load that other name.
    {
      vector <string> bibles = access_bible::bibles (request);
      string selected_bible;
      for (auto bible : bibles) {
        if (bible != filter::indonesian::ourtranslation ()) selected_bible = bible;
      }
      if (selected_bible.empty ()) {
        // No Bible selected yet: Create the Indonesian Sample Bible and take that.
        string user = request->session_logic ()->currentUser ();
        selected_bible = filter::indonesian::mytranslation (user);
        bible_logic::create_empty_bible (selected_bible);
      }
      request->database_config_user()->setBible (selected_bible);
    }
  }

  // Set the user chosen Bible as the current Bible.
  if (request->post.count ("bibleselect")) {
    string bibleselect = request->post ["bibleselect"];
    request->database_config_user ()->setBible (bibleselect);
    return string();
  }

  string page;
  
  Assets_Header header = Assets_Header (translate("Edit verse"), request);
  header.set_navigator ();
  header.set_editor_stylesheet ();
  if (touch) header.jquery_touch_on ();
  header.notify_it_on ();
  header.add_bread_crumb (menu_logic_translate_menu (), menu_logic_translate_text ());
  page = header.run ();
  
  Assets_View view;
  
  // Get active Bible, and check read access to it.
  // Or if the user have used query to preset the active Bible, get the preset Bible.
  // If needed, change Bible to one it has read access to.
  // Set the chosen Bible on the option HTML tag.
  string bible = access_bible::clamp (request, request->database_config_user()->getBible ());
  if (request->query.count ("bible")) bible = access_bible::clamp (request, request->query ["bible"]);
  string bible_html;
  vector <string> bibles = access_bible::bibles (request);
  for (auto selectable_bible : bibles) {
    bible_html = Options_To_Select::add_selection (selectable_bible, selectable_bible, bible_html);
  }
  view.set_variable ("bibleoptags", Options_To_Select::mark_selected (bible, bible_html));
  view.set_variable ("bible", bible);
  
  // Store the active Bible in the page's javascript.
  view.set_variable ("navigationCode", Navigation_Passage::code (bible));
  
  int verticalCaretPosition = request->database_config_user ()->getVerticalCaretPosition ();
  string script =
  "var oneverseEditorVerseLoaded = '" + locale_logic_text_loaded () + "';\n"
  "var oneverseEditorVerseUpdating = '" + locale_logic_text_updating () + "';\n"
  "var oneverseEditorVerseUpdated = '" + locale_logic_text_updated () + "';\n"
  "var oneverseEditorWillSave = '" + locale_logic_text_will_save () + "';\n"
  "var oneverseEditorVerseSaving = '" + locale_logic_text_saving () + "';\n"
  "var oneverseEditorVerseSaved = '" + locale_logic_text_saved () + "';\n"
  "var oneverseEditorVerseRetrying = '" + locale_logic_text_retrying () + "';\n"
  "var oneverseEditorVerseUpdatedLoaded = '" + locale_logic_text_reload () + "';\n"
  "var verticalCaretPosition = " + convert_to_string (verticalCaretPosition) + ";\n"
  "var verseSeparator = '" + Database_Config_General::getNotesVerseSeparator () + "';\n";
  config::logic::swipe_enabled (webserver_request, script);
  view.set_variable ("script", script);

  string custom_class = Filter_Css::getClass (bible);
  string font = Fonts_Logic::get_text_font (bible);
  int current_theme_index = request->database_config_user ()->getCurrentTheme ();
  int direction = Database_Config_Bible::getTextDirection (bible);
  int lineheight = Database_Config_Bible::getLineHeight (bible);
  int letterspacing = Database_Config_Bible::getLetterSpacing (bible);
  view.set_variable ("editor_theme_color", Filter_Css::theme_picker (current_theme_index, 2));
  view.set_variable ("active_editor_theme_color", Filter_Css::theme_picker (current_theme_index, 3));
  view.set_variable ("custom_class", custom_class);
  string custom_css = Filter_Css::get_css (custom_class,
                                          Fonts_Logic::get_font_path (font),
                                          direction, lineheight, letterspacing);
  view.set_variable ("custom_css", custom_css);

  
  // Whether to enable fast Bible editor switching.
  if (request->database_config_user ()->getFastEditorSwitchingAvailable ()) {
    view.enable_zone ("fastswitcheditor");
  }
  if (config::logic::indonesian_cloud_free ()) {
    view.enable_zone ("fastswitcheditor");
  }

  // Whether to enable the styles button.
  if (request->database_config_user ()->getEnableStylesButtonVisualEditors ()) {
    view.enable_zone ("stylesbutton");
  }
  
  page += view.render ("editone2", "index");
  
  page += assets_page::footer ();
  
  return page;
}

// Tests for the editor:
// * Autosave on going to another passage.
// * Autosave on document unload.
// * Autosave shortly after any change.
// * Save the + sign of a note.
// * No loss of white space right after the verse number.
