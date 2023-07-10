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


#include <config/libraries.h>
#ifdef HAVE_GTEST
#include "gtest/gtest.h"
#include <unittests/utilities.h>
#include <filter/url.h>
#include <filter/date.h>
#include <chrono>
#include <filesystem>
using namespace std;
using namespace std::chrono_literals;


TEST (filter, url)
{
  // Test writing to and reading from files, and whether a file exists.
  {
    string filename = "/tmp/בוקר טוב";
    string contents = "בוקר טוב בוקר טוב";
    EXPECT_EQ (false, file_or_dir_exists (filename));
    EXPECT_EQ (false, file_or_dir_exists (filename));
    filter_url_file_put_contents (filename, contents);
    EXPECT_EQ (true, file_or_dir_exists (filename));
    EXPECT_EQ (true, file_or_dir_exists (filename));
    EXPECT_EQ (contents, filter_url_file_get_contents (filename));
    filter_url_unlink (filename);
    EXPECT_EQ (false, file_or_dir_exists (filename));
    EXPECT_EQ (false, file_or_dir_exists (filename));
  }

  // Test function to check existence of directory.
  {
    string folder = "/tmp/בוקר טוב";
    EXPECT_EQ (false, file_or_dir_exists (folder));
    filter_url_mkdir (folder);
    EXPECT_EQ (true, file_or_dir_exists (folder));
    filter_url_rmdir (folder);
    EXPECT_EQ (false, file_or_dir_exists (folder));
  }
  
  // Test unique filename.
  {
    string filename = "/tmp/unique";
    filter_url_file_put_contents (filename, "");
    string filename1 = filter_url_unique_path (filename);
    filter_url_file_put_contents (filename1, "");
    EXPECT_EQ ("/tmp/unique.1", filename1);
    string filename2 = filter_url_unique_path (filename);
    filter_url_file_put_contents (filename2, "");
    EXPECT_EQ ("/tmp/unique.2", filename2);
    filter_url_unlink (filename);
    filter_url_unlink (filename1);
    filter_url_unlink (filename2);
  }
  
  // Html export filenames.
  {
    EXPECT_EQ ("index.html", filter_url_html_file_name_bible ());
    EXPECT_EQ ("path/index.html", filter_url_html_file_name_bible ("path"));
    EXPECT_EQ ("path/01-Genesis.html", filter_url_html_file_name_bible ("path", 1));
    EXPECT_EQ ("01-Genesis.html", filter_url_html_file_name_bible ("", 1));
    EXPECT_EQ ("path/11-1Kings.html", filter_url_html_file_name_bible ("path", 11));
    EXPECT_EQ ("path/22-SongofSolomon-000.html", filter_url_html_file_name_bible ("path", 22, 0));
    EXPECT_EQ ("path/33-Micah-333.html", filter_url_html_file_name_bible ("path", 33, 333));
    EXPECT_EQ ("33-Micah-333.html", filter_url_html_file_name_bible ("", 33, 333));
  }
  
  // Testing mkdir and rmdir including parents.
  {
    // Do test a folder name that starts with a dot.
    string directory = filter_url_create_path ({testing_directory, "a", ".git"});
    filter_url_mkdir (directory);
    string path = filter_url_create_path ({directory, "c"});
    string contents = "unittest";
    filter_url_file_put_contents (path, contents);
    EXPECT_EQ (contents, filter_url_file_get_contents (path));
    
    path = filter_url_create_path ({testing_directory, "a"});
    EXPECT_EQ (true, file_or_dir_exists (path));
    EXPECT_EQ (true, filter_url_is_dir (path));
    
    filter_url_rmdir (path);
    EXPECT_EQ (false, file_or_dir_exists (path));
    EXPECT_EQ (false, filter_url_is_dir (path));
  }
  
  // Test filter_url_escape_shell_argument.
  {
    EXPECT_EQ ("'argument'", filter_url_escape_shell_argument ("argument"));
    EXPECT_EQ ("'arg\\'ument'", filter_url_escape_shell_argument ("arg'ument"));
  }

  // Test URL decoder.
  {
    EXPECT_EQ ("Store settings", filter_url_urldecode ("Store+settings"));
    EXPECT_EQ ("test@mail", filter_url_urldecode ("test%40mail"));
    EXPECT_EQ ("ᨀab\\d@a", filter_url_urldecode ("%E1%A8%80ab%5Cd%40a"));
    EXPECT_EQ ("\xFF", filter_url_urldecode ("%FF"));
    EXPECT_EQ ("\xFF", filter_url_urldecode ("%ff"));
  }
  
  // Test URL encoder.
  {
    EXPECT_EQ ("Store%20settings", filter_url_urlencode ("Store settings"));
    EXPECT_EQ ("test%40mail", filter_url_urlencode ("test@mail"));
    EXPECT_EQ ("%E1%A8%80ab%5Cd%40a", filter_url_urlencode ("ᨀab\\d@a"));
    EXPECT_EQ ("foo%3Dbar%26baz%3D", filter_url_urlencode ("foo=bar&baz="));
    EXPECT_EQ ("%D7%91%D6%BC%D6%B0%D7%A8%D6%B5%D7%90%D7%A9%D7%81%D6%B4%D6%96%D7%99%D7%AA", filter_url_urlencode ("בְּרֵאשִׁ֖ית"));
    EXPECT_EQ ("ABC", filter_url_urlencode ("ABC"));
    EXPECT_EQ ("%FF", filter_url_urlencode ("\xFF"));
  }

  // Test encode and decode round trip.
  {
    string original ("\0\1\2", 3);
    string encoded ("%00%01%02");
    EXPECT_EQ (encoded, filter_url_urlencode (original));
    EXPECT_EQ (original, filter_url_urldecode (encoded));
  }

  // Test encode and decode unsafe chars, RFC1738.
  {
    string unsafe (" <>#{}|\\^~[]`");
    string unsafe_encoded = filter_url_urlencode (unsafe);
    EXPECT_EQ (true, unsafe_encoded.find_first_of (unsafe) == string::npos);
    EXPECT_EQ (unsafe, filter_url_urldecode (unsafe_encoded));
  }
  
  // Test char values used in encoding and decoding.
  {
    char one = -1;
    char two = static_cast<char> (255);
    EXPECT_EQ (one, two);
  }
    
  // Test dirname and basename functions.
  {
    EXPECT_EQ (".", filter_url_dirname (string()));
    EXPECT_EQ (".", filter_url_dirname ("/"));
    EXPECT_EQ (".", filter_url_dirname ("dir/"));
    // C++17 version: EXPECT_EQ ("/", filter_url_dirname ("/dir"));
    EXPECT_EQ (".", filter_url_dirname ("/dir"));
    EXPECT_EQ ("foo", filter_url_dirname ("foo/bar"));
    EXPECT_EQ ("/foo", filter_url_dirname ("/foo/bar"));
    EXPECT_EQ ("/foo", filter_url_dirname ("/foo/bar/"));
    EXPECT_EQ ("a.txt", filter_url_basename ("/a.txt"));
    EXPECT_EQ ("txt", filter_url_basename ("/txt/"));
    EXPECT_EQ ("foo.bar", filter_url_basename ("/path/to/foo.bar"));
    EXPECT_EQ ("foo.bar", filter_url_basename ("foo.bar"));
  }
  
  // Test http GET and POST
  {
    string result, error;
    result = filter_url_http_get ("http://localhost/none", error, false);
#ifndef HAVE_CLIENT
    EXPECT_EQ ("Couldn't connect to server", error);
#endif
    EXPECT_EQ ("", result);
    map <string, string> values = {pair ("a", "value1"), pair ("b", "value2")};
    result = filter_url_http_post ("http://localhost/none", string(), values, error, false, false, {});
#ifndef HAVE_CLIENT
    EXPECT_EQ ("Couldn't connect to server", error);
#endif
    EXPECT_EQ ("", result);
  }
  
  // Test low-level http(s) client error for unknown host.
  {
    string result, error;
    result = filter_url_http_request_mbed ("http://unknownhost", error, {}, "", false);
    EXPECT_EQ ("", result);
    EXPECT_EQ ("Internet connection failure: unknownhost: nodename nor servname provided, or not known", error);
  }
  
  // Test low-level http(s) client error for closed port.
  {
    string result, error;
    result = filter_url_http_request_mbed ("http://bibledit.org:8086/non-existing", error, {}, "", false);
    EXPECT_EQ ("", result);
    EXPECT_EQ ("bibledit.org:8086: Connection refused | bibledit.org:8086: Connection refused", error);
  }
  
  // Test low-level http(s) client result.
  {
    string result, error;
    result = filter_url_http_request_mbed ("http://185.87.186.229", error, {}, "", false);
    EXPECT_EQ (true, result.find ("Home") != string::npos);
    EXPECT_EQ (true, result.find ("Ndebele Bible") != string::npos);
    EXPECT_EQ (true, result.find ("Shona Bible") != string::npos);
    EXPECT_EQ (true, result.find ("Downloads") != string::npos);
    EXPECT_EQ ("", error);
  }
  {
    string result, error;
    result = filter_url_http_request_mbed ("https://bibledit.org", error, {}, "", false);
    EXPECT_EQ (true, result.find ("Bibledit") != string::npos);
    EXPECT_EQ (true, result.find ("Linux") != string::npos);
    EXPECT_EQ (true, result.find ("Cloud") != string::npos);
    EXPECT_EQ (true, result.find ("Android") != string::npos);
    EXPECT_EQ ("", error);
  }

  // Test removing credentials from a URL.
  {
    string url = "https://username:password@github.com/username/repository.git";
    url = filter_url_remove_username_password (url);
    EXPECT_EQ ("https://github.com/username/repository.git", url);
  }
  
  // Test recursively copying a directory.
  {
    string input = filter_url_create_root_path ({"unittests"});
    string output = "/tmp/test_copy_directory";
    filter_url_rmdir (output);
    filter_url_dir_cp (input, output);
    string path = filter_url_create_path ({output, "tests", "basic.css"});
    EXPECT_EQ (true, file_or_dir_exists (path));
  }
  
  // Secure communications.
  {
    filter_url_ssl_tls_initialize ();
    
    string url;
    string error;
    string result;
    
    url = filter_url_set_scheme (" localhost ", false);
    EXPECT_EQ ("http://localhost", url);
    url = filter_url_set_scheme ("httpx://localhost", false);
    EXPECT_EQ ("http://localhost", url);
    url = filter_url_set_scheme ("http://localhost", true);
    EXPECT_EQ ("https://localhost", url);
    
    result = filter_url_http_request_mbed ("http://www.google.nl", error, {}, "", false);
    EXPECT_EQ (true, result.find ("google") != string::npos);
    EXPECT_EQ (true, result.find ("search") != string::npos);
    EXPECT_EQ (true, result.find ("background") != string::npos);
    EXPECT_EQ ("", error);
    
    result = filter_url_http_request_mbed ("https://www.google.nl", error, {}, "", false);
    EXPECT_EQ (true, result.find ("google") != string::npos);
    EXPECT_EQ (true, result.find ("search") != string::npos);
    EXPECT_EQ (true, result.find ("background") != string::npos);
    EXPECT_EQ ("", error);
    
    result = filter_url_http_request_mbed ("https://bibledit.org:8091", error, {}, "", false);
    EXPECT_EQ ("", result);
    EXPECT_EQ ("Response code: 302 Found", error);
    
    filter_url_ssl_tls_finalize ();
  }
  
  // Testing is_dir.
  {
    string path = filter_url_create_root_path ({"git"});
    EXPECT_EQ (true, filter_url_is_dir (path));
    path = filter_url_create_root_path ({"setup", "index.html"});
    EXPECT_EQ (false, filter_url_is_dir (path));
  }
  
  // Testing checking for and setting write permissions.
  {
    string directory = filter_url_create_root_path ({filter_url_temp_dir ()});
    string file1 = filter_url_create_path ({directory, "1"});
    string file2 = filter_url_create_path ({directory, "2"});
    filter_url_file_put_contents (file1, "x");
    filter_url_file_put_contents (file2, "x");
    
    EXPECT_EQ (true, filter_url_get_write_permission (directory));
    EXPECT_EQ (true, filter_url_get_write_permission (file1));
    EXPECT_EQ (true, filter_url_get_write_permission (file2));

    chmod (directory.c_str(), S_IRUSR);
    chmod (file1.c_str(), S_IRUSR);
    chmod (file2.c_str(), S_IRUSR);
    
    EXPECT_EQ (false, filter_url_get_write_permission (directory));
    EXPECT_EQ (false, filter_url_get_write_permission (file1));
    EXPECT_EQ (false, filter_url_get_write_permission (file2));
    
    filter_url_set_write_permission (directory);
    filter_url_set_write_permission (file1);
    filter_url_set_write_permission (file2);
    
    EXPECT_EQ (true, filter_url_get_write_permission (directory));
    EXPECT_EQ (true, filter_url_get_write_permission (file1));
    EXPECT_EQ (true, filter_url_get_write_permission (file2));
  }

  // Email address validity.
  {
    EXPECT_EQ (true, filter_url_email_is_valid ("user@web.site"));
    EXPECT_EQ (false, filter_url_email_is_valid ("user@website"));
    EXPECT_EQ (false, filter_url_email_is_valid (" user@web.site"));
    EXPECT_EQ (false, filter_url_email_is_valid ("user @ web.site"));
  }
  
  // Getting the file extension.
  {
    EXPECT_EQ ("txt", filter_url_get_extension ("foo/bar.txt"));
    // C++17 version: EXPECT_EQ (string(), filter_url_get_extension (".hidden"));
    EXPECT_EQ ("hidden", filter_url_get_extension (".hidden"));
    EXPECT_EQ (string(), filter_url_get_extension (""));
  }
  
  // Reading the directory content.
  {
    string directory = filter_url_create_root_path ({filter_url_temp_dir (), "dirtest"});
    filter_url_mkdir(directory);
    string file1 = filter_url_create_path ({directory, "1"});
    string file2 = filter_url_create_path ({directory, "2"});
    filter_url_file_put_contents (file1, "1");
    filter_url_file_put_contents (file2, "2");
    vector <string> files = filter_url_scandir (directory);
    EXPECT_EQ ((vector <string>{"1", "2"}), files);
  }
  
  // Testing the file modification time.
  {
    string directory = filter_url_create_root_path ({filter_url_temp_dir (), "timetest"});
    filter_url_mkdir(directory);
    string file = filter_url_create_path ({directory, "file.txt"});
    filter_url_file_put_contents (file, "file.txt");
    int mod_time = filter_url_file_modification_time (file);
    int ref_time = filter::date::seconds_since_epoch ();
    bool check = (mod_time < ref_time - 1) || (mod_time > ref_time + 1);
    if (check) EXPECT_EQ (ref_time, mod_time);
  }
  
  // Testing the splitting of scheme and host and port.
  {
    string scheme {};
    string host {};
    int port {0};
    
    filter_url_get_scheme_host_port ("https://bibledit.org:8080", scheme, host, port);
    EXPECT_EQ ("https", scheme);
    EXPECT_EQ ("bibledit.org", host);
    EXPECT_EQ (8080, port);
    
    filter_url_get_scheme_host_port ("bibledit.org:8080", scheme, host, port);
    EXPECT_EQ (string(), scheme);
    EXPECT_EQ ("bibledit.org", host);
    EXPECT_EQ (8080, port);
    
    filter_url_get_scheme_host_port ("bibledit.org", scheme, host, port);
    EXPECT_EQ (string(), scheme);
    EXPECT_EQ ("bibledit.org", host);
    EXPECT_EQ (0, port);
  }
  
  refresh_sandbox (true);
}

#endif

