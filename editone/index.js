/*
Copyright (©) 2003-2020 Teus Benschop.

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


var quill = undefined;
var Delta = Quill.import ("delta");
var verseEditorUniqueID = Math.floor (Math.random() * 100000000);


$ (document).ready (function ()
{
  // Make the editor's menu to never scroll out of view.
  var bar = $ ("#editorheader").remove ();
  $ ("#workspacemenu").append (bar);

  visualVerseEditorInitializeOnce ();
  visualVerseEditorInitializeLoad ();

  navigationNewPassage ();
  
  $ (window).on ("unload", oneverseEditorUnload);
  
  oneverseIdPollerOn ();

  oneverseBindUnselectable ();
  
  $ ("#stylebutton").on ("click", oneverseStylesButtonHandler);
  
  $ (window).on ("keydown", oneverseWindowKeyHandler);

  if (swipe_operations) {
    $ ("body").swipe ( {
      swipeLeft:function (event, direction, distance, duration, fingerCount, fingerData) {
        oneverseSwipeLeft (event);
      },
      swipeRight:function (event, direction, distance, duration, fingerCount, fingerData) {
        oneverseSwipeRight (event);
      }
    });
  }
  
  $ ("#oneeditor").on ("click", oneEditorNoteCitationClicked);

});


function visualVerseEditorInitializeOnce ()
{
  var Parchment = Quill.import ('parchment');
  
  // Register block formatting class.
  var ParagraphClass = new Parchment.Attributor.Class ('paragraph', 'b', { scope: Parchment.Scope.BLOCK });
  Quill.register (ParagraphClass, true);
  
  // Register inline formatting class.
  var CharacterClass = new Parchment.Attributor.Class ('character', 'i', { scope: Parchment.Scope.INLINE });
  Quill.register (CharacterClass, true);
}


function visualVerseEditorInitializeLoad ()
{
  // Work around https://github.com/quilljs/quill/issues/1116
  // It sets the margins to 0 by adding an overriding class.
  // The Quill editor will remove that class again.
  $ ("#oneeditor > p").each (function (index) {
    var element = $(this);
    element.addClass ("nomargins");
  });
  
  // Instantiate editor.
  quill = new Quill ('#oneeditor', { });

  // Cause it to paste plain text only.
  quill.clipboard.addMatcher (Node.ELEMENT_NODE, function (node, delta) {
    var plaintext = $ (node).text ();
    return new Delta().insert (plaintext);
  });

  if (oneverseEditorWriteAccess) if (!quill.hasFocus ()) quill.focus ();
  
  // Event handlers.
  quill.on ("text-change", visualVerseEditorTextChangeHandler);
  quill.on ("selection-change", visualVerseEditorSelectionChangeHandler);
}


var oneverseBible;
var oneverseBook;
var oneverseNavigationBook;
var oneverseChapter;
var oneverseNavigationChapter;
var oneverseVerse;
var oneverseNavigationVerse;
var oneverseVerseLoading;
var oneverseVerseLoaded;
var oneverseEditorChangedTimeout;
var oneverseLoadedText;
var oneverseIdChapter = 0;
var oneverseReloadCozChanged = false;
var oneverseReloadCozError = false;
var oneverseReloadPosition = undefined;
var oneverseEditorTextChanged = false;
var oneverseSaveAsync;
var oneverseLoadAjaxRequest;
var oneverseSaving = false;
var oneverseEditorWriteAccess = true;
var oneverseEditorLoadDate = new Date(0);
var oneverseEditorSaveDate = new Date(0);


//
//
// Section for the new Passage event from the Navigator.
//
//


function navigationNewPassage ()
{
  if (typeof navigationBook != 'undefined') {
    oneverseNavigationBook = navigationBook;
    oneverseNavigationChapter = navigationChapter;
    oneverseNavigationVerse = navigationVerse;
  } else if (parent.window.navigationBook != 'undefined') {
    oneverseNavigationBook = parent.window.navigationBook;
    oneverseNavigationChapter = parent.window.navigationChapter;
    oneverseNavigationVerse = parent.window.navigationVerse;
  } else {
    return;
  }

  //if ((oneverseNavigationBook != oneverseBook) || (oneverseNavigationChapter != oneverseChapter)) {
  //}
  // Fixed: Reload text message when switching to another chapter.
  // https://github.com/bibledit/cloud/issues/408
  // Going to another verse, it also resets the editor save timer,
  // and the chapter identifier poller.
  oneverseIdPollerOff ();
  oneverseEditorSaveDate = new Date(0);
  oneverseEditorSaveDate = new Date(0);
  oneverseReloadCozChanged = false;
  oneverseReloadCozError = false;
  oneverseIdPollerOn ();
}


//
//
// Section for editor load and save.
//
//


function oneverseEditorUnload ()
{
}


//
//
// Portion dealing with triggers for editor's content change.
//
//


// Arguments: delta: Delta, oldContents: Delta, source: String
function visualVerseEditorTextChangeHandler (delta, oldContents, source)
{
  // Ensure that it does not delete a chapter number or verse number.
  if (!delta.ops [0].retain) {
    quill.history.undo ();
  }
  // Start save delay timer.
  oneverseEditorChanged ();
}


function oneverseEditorChanged ()
{
  if (!oneverseEditorWriteAccess) return;
  oneverseEditorTextChanged = true;
  oneverseEditorStatus (oneverseEditorWillSave);
  if (oneverseEditorChangedTimeout) {
    clearTimeout (oneverseEditorChangedTimeout);
  }
}


//
//
// Section for user interface updates and feedback.
//
//


function oneverseEditorStatus (text)
{
  $ ("#onestatus").empty ();
  $ ("#onestatus").append (text);
  oneverseEditorSelectiveNotification (text);
}


function oneverseActiveStylesFeedback ()
{
  var format = quill.getFormat ();
  var paragraph = "...";
  if (format.paragraph) paragraph = format.paragraph;
  var character = "";
  if (format.character) character = format.character;
  if (character.search ("note") >= 0) character = "";
  character = character.split ("0").join (" ");
  var styles = paragraph + " " + character;
  var element = $ ("#oneverseactivestyles");
  element.text (styles);
}


function oneverseEditorSelectiveNotification (message)
{
  if (message == oneverseEditorVerseLoaded) return;
  if (message == oneverseEditorWillSave) return;
  if (message == oneverseEditorVerseSaving) return;
  if (message == oneverseEditorVerseSaved) return;
  notifyItError (message);
}


//
//
// Section for polling the server for updates on the loaded chapter.
//
//


var oneverseIdTimeout;
var oneverseIdAjaxRequest;


function oneverseIdPollerOff ()
{
  if (oneverseIdTimeout) {
    clearTimeout (oneverseIdTimeout);
  }
  if (oneverseIdAjaxRequest && oneverseIdAjaxRequest.readystate != 4) {
    oneverseIdAjaxRequest.abort();
  }
}


function oneverseIdPollerOn ()
{
  oneverseIdPollerOff ();
}


//
//
// Section for getting and setting the caret position.
//
//


function oneverseCaretPosition ()
{
  var position = undefined;
  var range = quill.getSelection();
  if (range) position = range.index;
  return position;
}


function oneversePositionCaret ()
{
  setTimeout (oneversePositionCaretTimeout, 100);
}


function oneversePositionCaretTimeout ()
{
  var position;
  if (oneverseReloadPosition != undefined) {
    position = oneverseReloadPosition;
    oneverseReloadPosition = undefined;
  } else {
    position = 1 + oneverseVerse.length;
  }
  quill.setSelection (position, 0, "silent");
}


//
//
// Section for scrolling the editable portion into the center.
//
//


function oneverseScrollVerseIntoView ()
{
  $("#workspacewrapper").stop();
  var verseTop = $("#oneeditor").offset().top;
  var workspaceHeight = $("#workspacewrapper").height();
  var currentScrollTop = $("#workspacewrapper").scrollTop();
  var scrollTo = verseTop - (workspaceHeight * verticalCaretPosition / 100) + currentScrollTop;
  var lowerBoundary = currentScrollTop - (workspaceHeight / 10);
  var upperBoundary = currentScrollTop + (workspaceHeight / 10);
  if ((scrollTo < lowerBoundary) || (scrollTo > upperBoundary)) {
    $("#workspacewrapper").animate({ scrollTop: scrollTo }, 500);
  }
}


//
//
// Section for the styles handling.
//
//


function oneverseStylesButtonHandler ()
{
}


function oneverseBindUnselectable ()
{
  var elements = $ (".unselectable");
  elements.off ("mousedown");
  elements.on ("mousedown", function (event) {
    event.preventDefault();
  });
}


function oneverseShowResponse (response)
{
  if (!oneverseEditorWriteAccess) return;
  $ ("#stylebutton").hide ();
  $ ("#nostyles").hide ();
  var area = $ ("#stylesarea");
  area.empty ();
  area.append (response);
}


function oneverseClearStyles ()
{
  var area = $ ("#stylesarea");
  area.empty ();
  $ ("#stylebutton").show ();
  $ ("#nostyles").show ();
}


function oneverseDynamicClickHandlers ()
{
  var elements = $ ("#stylesarea > a");
  elements.on ("click", function (event) {
    event.preventDefault();
    oneverseClearStyles ();
    var href = event.currentTarget.href;
    href = href.substring (href.lastIndexOf ('/') + 1);
    if (href == "cancel") return;
    if (href == "all") {
      oneverseDisplayAllStyles ();
    } else {
      oneverseRequestStyle (href);
    }
  });
  
  $ ("#styleslist").on ("change", function (event) {
    var selection = $ ("#styleslist option:selected").text ();
    var style = selection.substring (0, selection.indexOf (" "));
    event.preventDefault();
    oneverseClearStyles ();
    oneverseRequestStyle (style);
  });
}


function oneverseRequestStyle (style)
{
}


function oneverseDisplayAllStyles ()
{
}


function oneverseApplyParagraphStyle (style)
{
  if (!oneverseEditorWriteAccess) return;
  if (!quill.hasFocus ()) quill.focus ();
  quill.format ("paragraph", style, "user");
  oneverseActiveStylesFeedback ();
}


function oneverseApplyCharacterStyle (style)
{
  if (!oneverseEditorWriteAccess) return;
  if (!quill.hasFocus ()) quill.focus ();
  // No formatting of a verse.
  var range = quill.getSelection();
  if (range) {
    for (i = range.index; i < range.index + range.length; i++) {
      var format = quill.getFormat (i);
      if (format.character && format.character == "v") {
        return;
      }
    }
  }
  // Apply style.
  var format = quill.getFormat ();
  style = editor_determine_character_style (format.character, style)
  quill.format ("character", style, "user");
  // Deal with embedded character styles.
  post_embedded_style_application (style);
  // Feedback.
  oneverseActiveStylesFeedback ();
}


function oneverseApplyMonoStyle (style)
{
  if (!oneverseEditorWriteAccess) return;
  
  quill.focus ();

  var range = quill.getSelection();
  var caret = range.index;

  var start = caret;
  var text = quill.getText (start, 1);
  if (text == "\n") caret--;
  for (i = caret; i >= 0; i--) {
    var text = quill.getText (i, 1);
    if (text == "\n") break;
    start = i;
  }
  
  var end = caret;
  for (i = caret; true; i++) {
    end = i;
    var text = quill.getText (i, 1);
    if (text == "\n") break;
  }

  text = quill.getText (start, end - start);
  var char = text.substring (0, 1);
  if (char == "\\") {
    text = text.substring (1);
    var pos = text.indexOf (' ');
    text = text.substring (pos + 1);
  }
  text = "\\" + style + " " + text;
  
  quill.deleteText (start, end - start);
  quill.insertText (start, text, "paragraph", "mono");

  oneverseActiveStylesFeedback ();
}


//
//
// Section with window events and their basic handlers.
//
//


function oneverseWindowKeyHandler (event)
{
  if (!oneverseEditorWriteAccess) return;
  // Ctrl-S: Style.
  if ((event.ctrlKey == true) && (event.keyCode == 83)) {
    oneverseStylesButtonHandler ();
    return false;
  }
  // Escape.
  if (event.keyCode == 27) {
    oneverseClearStyles ();
  }
}


//
//
// Section responding to a moved caret.
//
//


// Responds to a changed selection or caret.
// range: { index: Number, length: Number }
// oldRange: { index: Number, length: Number }
// source: String
function visualVerseEditorSelectionChangeHandler (range, oldRange, source)
{
  // Bail out if editor not focused.
  if (!range) return;

  // Bail out if text was selected.
  if (range.length != 0) return;

  oneverseCaretMovedTimeoutStart ();
}


var oneverseCaretMovedTimeoutId;


function oneverseCaretMovedTimeoutStart ()
{
  if (oneverseCaretMovedTimeoutId) clearTimeout (oneverseCaretMovedTimeoutId);
  oneverseCaretMovedTimeoutId = setTimeout (oneverseActiveStylesFeedback, 200);
}


//
//
// Section for the notes.
//
//


var oneverseInsertedNotesCount = 0;


function oneverseApplyNotesStyle (style)
{
  if (!oneverseEditorWriteAccess) return;

  quill.focus ();

  // Check for and optionally append the gap between text body and notes.
  var notes = $ (".b-notes");
  if (notes.length == 0) {
    var length = quill.getLength ();
    quill.insertText (length, "\n", "paragraph", "notes", "user")
  }
  
  // Get a new node identifier based on the local time.
  var date = new Date();
  var noteId = String (date.getTime());
  
  var caller = String ((oneverseInsertedNotesCount % 9) + 1);

  // Insert note caller at caret.
  var range = quill.getSelection();
  var caret = range.index;
  quill.setSelection (caret, 0);
  quill.insertText (caret, caller, "character", "notecall" + noteId, "user");

  // Append note text to notes section.
  assetsEditorAddNote (quill, style, caller, noteId, oneverseNavigationChapter, verseSeparator, oneverseNavigationVerse);

  oneverseInsertedNotesCount++;
  
  oneverseActiveStylesFeedback ();
}


function oneEditorNoteCitationClicked (event)
{
  var target = $(event.target);
  var cls = target.attr ("class");
  if (cls.substr (0, 6) != "i-note") return;
  cls = cls.substr (2);
  var length = quill.getLength ();
  if (cls.search ("call") > 0) {
    cls = cls.replace ("call", "body");
    // Start searching for note bodies at the end.
    for (i = length; i >= 0; i--) {
      var format = quill.getFormat (i, 1);
      if (format.character && format.character == cls) {
        quill.setSelection (i + 4, 0);
        oneverseActiveStylesFeedback ();
        return;
      }
    }
  }
  if (cls.search ("body") > 0) {
    cls = cls.replace ("body", "call");
    // Start searching for note callers at the start.
    for (i = 0; i < length; i++) {
      var format = quill.getFormat (i, 1);
      if (format.character && format.character == cls) {
        quill.setSelection (i + 1, 0);
        oneverseActiveStylesFeedback ();
        return;
      }
    }
  }
}


//
//
// Section for navigating to another passage.
//
//


function oneVerseHtmlClicked (event)
{
}


//
//
// Section for swipe navigation.
//
//


function oneverseSwipeLeft (event)
{
  if (typeof navigateNextVerse != 'undefined') {
    navigateNextVerse (event);
  } else if (parent.window.navigateNextVerse != 'undefined') {
    parent.window.navigateNextVerse (event);
  }
}


function oneverseSwipeRight (event)
{
  if (typeof navigatePreviousVerse != 'undefined') {
    navigatePreviousVerse (event);
  } else if (parent.window.navigatePreviousVerse != 'undefined') {
    parent.window.navigatePreviousVerse (event);
  }
}


/*

Section for reload notifications.

*/


function oneverseReloadAlert (message)
{
  // Take action only if the editor has focus and the user can type in it.
  if (!quill.hasFocus ()) return;
  // Do the notification stuff.
  notifyItSuccess (message)
  quill.enable (false);
  setTimeout (oneverseReloadAlertTimeout, 3000);
}


function oneverseReloadAlertTimeout ()
{
  quill.enable (oneverseEditorWriteAccess);
  quill.focus ();
}


