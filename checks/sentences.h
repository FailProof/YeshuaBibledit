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

class Checks_Sentences
{
public:
  void enter_capitals (const string & capitals);
  void enter_small_letters (const string & small_letters);
  void enter_end_marks (const string & end_marks);
  void enter_center_marks (const string & center_marks);
  void enter_disregards (const string & disregards);
  void enter_names (string names);
  void initialize ();
  vector <pair<int, string>> get_results ();
  void check (const map <int, string> & texts);
  void paragraphs (const vector <string> & paragraph_start_markers,
                   const vector <string> & within_sentence_paragraph_markers,
                   const vector <map <int, string>> & verses_paragraphs);

private:
  // Sentence structure parameters.
  vector <string> m_capitals {};
  vector <string> m_small_letters {};
  vector <string> m_end_marks {};
  vector <string> m_center_marks {};
  vector <string> m_disregards {};
  vector <string> m_names {};
  
  // State.
  int verse_number {0};
  int current_position {0};
  
  // Grapheme analysis.
  string character {};
  bool is_space {false};
  int space_position {0};
  bool is_capital {false};
  int capital_position {0};
  bool is_small_letter {false};
  int small_letter_position {0};
  bool is_end_mark {false};
  int end_mark_position {0};
  bool is_center_mark {false};
  int center_mark_position {0};
  int punctuation_mark_position {0};
  int previous_mark_position {0};
  
  // Context.
  string full_text {};
  
  // Results of the checks.
  vector <pair<int, string>> checking_results {};
  static constexpr int display_character_only {1};
  static constexpr int display_context {2};
  static constexpr int skip_names {3};
  
  void add_result (string text, int modifier);
  void check_unknown_character ();
  void analyze_characters ();
  void check_character ();
};
