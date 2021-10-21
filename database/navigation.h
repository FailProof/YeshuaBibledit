/*
Copyright (©) 2003-2021 Teus Benschop.

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


#ifndef INCLUDED_DATABASE_NAVIGATION_H
#define INCLUDED_DATABASE_NAVIGATION_H


#include <config/libraries.h>
#include <filter/passage.h>


class Database_Navigation
{
public:
  void create ();
  void trim ();
  void record (int time, string user, int book, int chapter, int verse);
  bool previous_exists (const string& user);
  bool next_exists (const string& user);
  Passage get_previous (const string& user);
  Passage get_next (const string& user);
private:
  sqlite3 * connect ();
  int get_previous_id (const string& user);
  int get_next_id (const string& user);
};


#endif
