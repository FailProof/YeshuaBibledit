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


#include <string>
#include <vector>
using namespace std;
// System configuration.
#include <config.h>
// Bibledit configuration.
#include <config/config.h>
// Specific includes.


#ifdef HAVE_CLOUD


class Database_Git
{
public:
  static void create ();
  static void optimize ();
  static void store_chapter (string user, string bible, int book, int chapter,
                             string oldusfm, string newusfm);
  static vector <string> get_users (string bible);
  static vector <int> get_rowids (string user, string bible);
  static bool get_chapter (int rowid,
                           string & user, string & bible, int & book, int & chapter,
                           string & oldusfm, string & newusfm);
  static void erase_rowid (int rowid);
  static void touch_timestamps (int timestamp);
private:
  static const char * name ();
};


#endif
