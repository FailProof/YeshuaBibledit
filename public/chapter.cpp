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


#include <public/chapter.h>
#include <filter/roles.h>
#include <filter/string.h>
#include <filter/usfm.h>
#include <filter/text.h>
#include <filter/css.h>
#include <webserver/request.h>
#include <database/config/bible.h>
using namespace std;


string public_chapter_url ()
{
  return "public/chapter";
}


bool public_chapter_acl (void * webserver_request)
{
  return Filter_Roles::access_control (webserver_request, Filter_Roles::guest ());
}


string public_chapter (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
 
  string bible = request->query ["bible"];
  int book = filter::strings::convert_to_int (request->query ["book"]);
  int chapter = filter::strings::convert_to_int (request->query ["chapter"]);
  
  string stylesheet = Database_Config_Bible::getExportStylesheet (bible);
  
  string usfm = request->database_bibles()->get_chapter (bible, book, chapter);
  
  Filter_Text filter_text = Filter_Text (bible);
  filter_text.html_text_standard = new HtmlText (bible);
  filter_text.html_text_standard->custom_class = Filter_Css::getClass (bible);
  filter_text.add_usfm_code (usfm);
  filter_text.run (stylesheet);

  string html = filter_text.html_text_standard->get_inner_html ();
  
  return html;
}
