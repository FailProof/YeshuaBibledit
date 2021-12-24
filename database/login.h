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


#pragma once

#include <config/libraries.h>

class Database_Login
{
public:
  static const char * database ();
  static void create ();
  static void trim ();
  static void optimize ();
  static bool healthy ();
  static void setTokens (string username, string address, string agent, string fingerprint, string cookie, bool touch);
  static void removeTokens (string username);
  static void removeTokens (string username, string cookie);
  static void renameTokens (string username_existing, string username_new, string cookie);
  static string getUsername (string cookie, bool & daily);
  static bool getTouchEnabled (string cookie);
  static void testTimestamp ();
private:
  static int timestamp ();
};
