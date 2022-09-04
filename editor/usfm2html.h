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


#pragma once

#include <config/libraries.h>
#include <database/styles.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#include <pugixml/pugixml.hpp>
#pragma GCC diagnostic pop
#include <filter/text.h>

using namespace pugi;

class Editor_Usfm2Html
{
public:
  void load (string usfm);
  void stylesheet (string stylesheet);
  void run ();
  string get ();
  size_t textLength {0};
  map <int, int> verseStartOffsets {};
  string currentParagraphStyle {};
  string currentParagraphContent {};
  void set_preview ();
private:
  vector <string> markers_and_text {}; // Strings alternating between USFM and text.
  unsigned int markers_and_text_pointer {0};
  
  // All the style information.
  map <string, Database_Styles_Item> styles {};
  
  // XML nodes.
  xml_document document {};
  xml_node body_node {};
  xml_node notes_node {};
  
  // Standard content markers for notes.
  string standardContentMarkerFootEndNote {};
  string standardContentMarkerCrossReference {};

  xml_node current_p_node {}; // The current p node.
  bool current_p_open {false};
  vector <string> currentTextStyles {};
  
  int noteCount {0};
  xml_node notePnode {}; // The p DOM element of the current footnote, if any.
  bool note_p_open {false};
  vector <string> currentNoteTextStyles {};
  
  // The note citations.
  filter::note::citations note_citations {};

  // Whether note is open.
  bool noteOpened {false};
  
  // Lengths and offsets.
  bool first_line_done {false};
  
  // Whether it runs in preview-mode.
  bool m_preview { false };
  
  void preprocess ();
  void process ();
  void postprocess ();
  void outputAsIs (string marker, bool isOpeningMarker);
  void newParagraph (string style = "");
  void closeParagraph ();
  void openTextStyle (Database_Styles_Item & style, bool embed);
  void closeTextStyle (bool embed);
  void addText (string text);
  void add_note (string citation, string style, bool endnote = false);
  void addNoteText (string text);
  void closeCurrentNote ();
  void addNotelLink (xml_node domNode, int identifier, string style, string text);
  
  bool roadIsClear ();
};
