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

class Editor_Styles
{
public:
  static std::string getRecentlyUsed (void * webserver_request);
  static std::string getAll (void * webserver_request);
  static void recordUsage (void * webserver_request, std::string style);
  static std::string getAction (void * webserver_request, std::string style);
private:
  static std::string unknown ();
  static std::string paragraph ();
  static std::string character ();
  static std::string mono ();
  static std::string note ();
};
