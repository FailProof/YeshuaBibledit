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


#include <sync/mail.h>
#include <filter/string.h>
#include <database/mail.h>
#include <sync/logic.h>
#include <locale/translate.h>
#include <webserver/request.h>
#include <email/send.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#ifndef HAVE_PUGIXML
#include <pugixml/pugixml.hpp>
#endif
#ifdef HAVE_PUGIXML
#include <pugixml.hpp>
#endif
#pragma GCC diagnostic pop
using namespace std;
using namespace pugi;


string sync_mail_url ()
{
  return "sync/mail";
}


string sync_mail (void * webserver_request)
{
  Webserver_Request * request = static_cast<Webserver_Request *>(webserver_request);
  Sync_Logic sync_logic = Sync_Logic (webserver_request);

  if (!sync_logic.security_okay ()) {
    // When the Cloud enforces https, inform the client to upgrade.
    request->response_code = 426;
    return "";
  }
  
  // Get the relevant parameters the client may have POSTed to us, the server.
  string name = hex2bin (request->post ["n"]);
  string subject = request->post ["s"];
  string body = request->post ["b"];
  
  // Mail will only be sent if all three fields are filled.
  if (!name.empty ()) {
    if (!subject.empty ()) {
      if (!body.empty ()) {
        
        // Add information to the bottom of the email about the client sender for clarity.
        // This was added because email formatting in Bibledit Cloud was updated.
        // https://github.com/bibledit/cloud/issues/676
        // But since some users do not update their clients often,
        // mails generated on the clients would still be in the old format.
        // Adding a client-generated signature to the email would indicate old clients.
        // That helps in tracking down things related to issue 676.
        xml_document document;
        document.append_child ("br");
        xml_node node = document.append_child ("p");
        string client_information;
        client_information.append (translate ("Mail generated by a Bibledit client at IP address"));
        client_information.append (request->remote_address);
        node.text ().set (client_information.c_str());
        stringstream output;
        document.print (output, "", format_raw);
        client_information = output.str ();
        body.append(client_information);
        
        // Send the email.
        email_schedule (name, subject, body);
        return "";
      }
    }
  }
  
  // Bad request.
  // Delay a while to obstruct a flood of bad requests.
  this_thread::sleep_for (chrono::seconds (1));
  request->response_code = 400;
  return "";
}
