/*
Copyright (©) 2003-2023 Teus Benschop.

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

#ifdef HAVE_CLOUD
void rss_logic_feed_on_off ();
void rss_logic_schedule_update (std::string user, std::string bible, int book, int chapter,
                                std::string oldusfm, std::string newusfm);
void rss_logic_execute_update (std::string user, std::string bible, int book, int chapter,
                               std::string oldusfm, std::string newusfm);
std::string rss_logic_xml_path ();
void rss_logic_update_xml (std::vector <std::string> titles, std::vector <std::string> authors, std::vector <std::string> descriptions);
#endif
