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
#include <styles/logic.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <pugixml/pugixml.hpp>
#pragma GCC diagnostic pop

using namespace pugi;

class odf_text
{
public:
  odf_text (string bible_in);
  ~odf_text ();
  void new_paragraph (string style = styles_logic_standard_sheet ());
  void add_text (string text);
  void new_heading1 (string text, bool hide = false);
  void create_page_break_style ();
  void new_page_break ();
  void create_paragraph_style (string name,
                               string fontname,
                               float fontsize,
                               int italic, int bold, int underline,
                               int smallcaps,
                               int alignment,
                               float spacebefore, float spaceafter,
                               float leftmargin, float rightmargin,
                               float firstlineindent,
                               bool keep_with_next,
                               int dropcaps);
  void update_current_paragraph_style (string name);
  void open_text_style (Database_Styles_Item style, bool note, bool embed);
  void close_text_style (bool note, bool embed);
  void place_text_in_frame (string text, string style, float fontsize, int italic, int bold);
  void create_superscript_style ();
  void add_note (string caller, string style, bool endnote = false);
  void add_note_text (string text);
  void close_current_note ();
  void save (string name);
  string current_paragraph_style;
  string current_paragraph_content;
  vector <string> current_text_style;
  void add_image (string alt, string src, string caption);
  void add_tab ();
private:
  string bible {};
  string unpacked_odt_folder {};
  xml_document content_dom {}; // The content.xml DOMDocument.
  xml_node office_text_node {}; // The office:text DOMNode.
  xml_document styles_dom {}; // The styles.xml DOMDocument.
  vector <string> created_styles {}; // An array with styles already created in the $stylesDom.
  xml_node office_styles_node {}; // The office:styles DOMNode.
  xml_node current_text_p_node {}; // The current text:p DOMElement.
  bool current_text_p_node_opened {false}; // Whether the text:p element has been opened.
  xml_attribute current_text_p_node_style_name {}; // The DOMAttr of the name of the style of the current text:p element.
  int frame_count {0};
  int note_count {0};
  xml_node note_text_p_dom_element {}; // The text:p DOMElement of the current footnote, if any.
  bool note_text_p_opened {false}; // Whether the text:p for notes has been opened.
  vector <string> current_note_text_style {};
  void initialize_content_xml ();
  void initialize_styles_xml ();
  void new_named_heading (string style, string text, bool hide = false);
  string convert_style_name (string style);
  int image_counter {0};
  bool automatic_note_caller {false};
};

