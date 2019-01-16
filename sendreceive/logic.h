/*
 Copyright (©) 2003-2018 Teus Benschop.
 
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


#ifndef INCLUDED_SENDRECEIVE_LOGIC_H
#define INCLUDED_SENDRECEIVE_LOGIC_H


#include <config/libraries.h>
#include <filter/string.hpp>


void sendreceive_queue_bible (string bible);

void sendreceive_queue_sync (int minute);
void sendreceive_queue_paratext ();
bool sendreceive_sync_queued ();
bool sendreceive_paratext_queued ();

void sendreceive_queue_all (bool now);
void sendreceive_queue_startup ();

bool sendreceive_logic_prioritized_task_is_active ();

bool sendreceive_git_repository_linked (string bible);


#endif
