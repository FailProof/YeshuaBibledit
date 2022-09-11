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


#include <database/books.h>
#include <config/globals.h>
#include <database/sqlite.h>
#include <filter/string.h>
#include <filter/diff.h>
#include <locale/translate.h>
#include <database/booksdata.h>


namespace database::books {


// Internal function for the number of data elements.
constexpr size_t data_count = sizeof (books_table) / sizeof (*books_table);


vector <int> get_ids ()
{
  vector <int> ids;
  for (unsigned int i = 0; i < data_count; i++) {
    int id = books_table[i].id;
    ids.push_back (id);
  }
  return ids;
}


int get_id_from_english (string english)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (english == books_table[i].english) {
      return books_table[i].id;
    }
  }
  return 0;  
}


string get_english_from_id (int id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].english;
    }
  }
  return translate ("Unknown");
}


string get_usfm_from_id (int id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].usfm;
    }
  }
  return "XXX";
}


string get_bibleworks_from_id (int id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].bibleworks;
    }
  }
  return "Xxx";
}


string get_osis_from_id (int id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].osis;
    }
  }
  return translate ("Unknown");
}


int get_id_from_usfm (string usfm)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (usfm == books_table[i].usfm) {
      return books_table[i].id;
    }
  }
  return 0;
}


int get_id_from_osis (string osis)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (osis == books_table[i].osis) {
      return books_table[i].id;
    }
  }
  return 0;
}


int get_id_from_bibleworks (string bibleworks)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (bibleworks == books_table[i].bibleworks) {
      return books_table[i].id;
    }
  }
  return 0;
}


/*
Tries to interprete $text as the name of a Bible book.
Returns the book's identifier if it succeeds.
If it fails, it returns 0.
*/
int get_id_like_text (string text)
{
  // Go through all known book names and abbreviations.
  // Note how much the $text differs from the known names.
  // Then return the best match.
  vector <int> ids;
  vector <int> similarities;
  for (unsigned int i = 0; i < data_count; i++) {
    int id = books_table[i].id;
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, unicode_string_casefold(books_table[i].english)));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, unicode_string_casefold(books_table[i].osis)));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, books_table[i].usfm));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, unicode_string_casefold(books_table[i].bibleworks)));
    ids.push_back (id);
    similarities.push_back (filter_diff_character_similarity (text, unicode_string_casefold(books_table[i].onlinebible)));
  }
  quick_sort (similarities, ids, 0, static_cast<unsigned>(ids.size()));
  return ids.back ();
}


int get_id_from_onlinebible (string onlinebible)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (onlinebible == books_table[i].onlinebible) {
      return books_table[i].id;
    }
  }
  return 0;
}


string get_onlinebible_from_id (int id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].onlinebible;
    }
  }
  return "";
}


int get_order_from_id (int id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].order;
    }
  }
  return 0;
}


string get_type (int id)
{
  for (unsigned int i = 0; i < data_count; i++) {
    if (id == books_table[i].id) {
      return books_table[i].type;
    }
  }
  return string();
}


} // End of namespace.
