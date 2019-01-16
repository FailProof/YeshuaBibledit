#!/bin/bash

# Copyright (©) 2003-2018 Teus Benschop.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.


cd "$(dirname "$0")"
cd "$(dirname "$PWD")"
SRC=$PWD
echo "Source directory " $SRC
TMP=/tmp/bibledit-i18n
echo Working directory $TMP


echo Synchronizing source files to working directory.
mkdir -p $TMP
rsync -a --delete $SRC/ $TMP/
cd $TMP


echo Removing files not to be processed.
rm -rf unittest*
rm -rf utf8*
rm -rf mbedtls


echo Gathering all the html files for internationalization.
find . -iname "*.html" > i18n.html
if [ $? -ne 0 ]
then
echo Cannot find source files
fi


echo Transfer translatable strings from the html files to a C++ file.
g++ -std=c++11 i18n/i18n.cpp
./a.out
rm a.out


echo Cleaning up raw string literals.
# Removing C++11 raw string literals
# because xgettext would complain about unterminated string literals in, e.g.:
# string script = R"(
# <div id="defid" style="clear:both"></div>
# )";
sed -i.bak '/= R"(/,/)";/d' lexicon/logic.cpp


echo Create a temporal file containing all the files for internationalization.
find . -iname "*.cpp" -o -iname "books.h" > gettextfiles.txt
if [ $? -ne 0 ]
then
echo Cannot find source files
fi


# Remove any previous bibledit.pot because it could have strings no longer in use.
rm -f /tmp/bibledit.pot


echo Extracting translatable strings and storing them in bibledit.pot
xgettext --files-from=gettextfiles.txt --default-domain=bibledit --force-po --copyright-holder="Teus Benschop" -o /tmp/bibledit.pot --from-code=UTF-8 --no-location --keyword=translate --language=C
if [ $? -ne 0 ]
then
echo Failure running xgettext
fi


# The message ids in bibledit.pot are unique already.
# If this were not the case, then $ msguniq could help to make them unique.


echo Copying bibledit.pot into place.
cp /tmp/bibledit.pot $SRC/locale


# Fix bzr: warning: unsupported locale setting on macOS.
export LC_ALL=C


echo Pull translations from launchpad.net.
cd
cd dev/launchpad/po
rm -f .DS_Store
bzr pull lp:~teusbenschop/bibledit/translations
if [ $? -ne 0 ]
then
echo Could not pull translations from launchpad.net
fi


echo Synchronize translations to Bibledit.
cd
cd dev/launchpad/po
cp *.po ~/dev/cloud/locale
if [ $? -ne 0 ]
then
echo Could not synchronize translations to Bibledit
fi


echo Push new translatable messages to Launchpad.
cd
cd dev/launchpad/pot
cp /tmp/bibledit.pot .
bzr add bibledit.pot
bzr commit --message "updated bibledit.pot"
bzr push


echo Clean up.
# Remove dates so they don't appear as daily changes.
sed -i.bak '/POT-Creation-Date/d' ~/dev/cloud/locale/*.po ~/dev/cloud/locale/bibledit.pot
sed -i.bak '/X-Launchpad-Export-Date/d' ~/dev/cloud/locale/*.po ~/dev/cloud/locale/bibledit.pot
rm ~/dev/cloud/locale/*.bak
# Remove temporal .pot.
rm /tmp/bibledit.pot
