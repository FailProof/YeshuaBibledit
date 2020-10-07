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
  
  setTimeout (oneverseCoordinatingTimeout, 500);

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
var oneverseChapterId = 0;
var oneverseReloadCozChanged = false;
var oneverseReloadCozError = false;
var oneverseReloadPosition = undefined;
var oneverseSaveAsync;
var oneverseLoadAjaxRequest;
var oneverseSaving = false;
var oneverseEditorWriteAccess = true;


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
  oneverseEditorSaveVerse (true);
  oneverseReloadCozChanged = false;
  oneverseReloadCozError = false;
  oneverseEditorLoadVerse ();
  oneverseIdPollerOn ();
}


//
//
// Section for editor load and save.
//
//


function oneverseEditorLoadVerse ()
{
  if ((oneverseNavigationBook != oneverseBook) || (oneverseNavigationChapter != oneverseChapter) || (oneverseNavigationVerse != oneverseVerse) || oneverseReloadCozChanged || oneverseReloadCozError ) {
    oneverseBible = navigationBible;
    oneverseBook = oneverseNavigationBook;
    oneverseChapter = oneverseNavigationChapter;
    oneverseVerse = oneverseNavigationVerse;
    oneverseVerseLoading = oneverseNavigationVerse;
    oneverseChapterId = 0;
    if (oneverseReloadCozChanged) {
      oneverseReloadPosition = oneverseCaretPosition ();
    } else {
      oneverseReloadPosition = undefined;
    }
    if (oneverseLoadAjaxRequest && oneverseLoadAjaxRequest.readystate != 4) {
      oneverseLoadAjaxRequest.abort();
    }
    oneverseLoadAjaxRequest = $.ajax ({
      url: "load",
      type: "GET",
      data: { bible: oneverseBible, book: oneverseBook, chapter: oneverseChapter, verse: oneverseVerseLoading, id: verseEditorUniqueID },
      success: function (response) {
        // Flag for editor read-write or read-only.
        oneverseEditorWriteAccess = checksum_readwrite (response);
        // If this is the second or third or higher editor in the workspace,
        // make the editor read-only.
        if (window.frameElement) {
          iframe = $(window.frameElement);
          var data_editor_number = iframe.attr("data-editor-no");
          if (data_editor_number > 1) {
            oneverseEditorWriteAccess = false;
          }
        }
        // Checksumming.
        response = checksum_receive (response);
        // Splitting.
        var bits;
        if (response !== false) {
          bits = response.split ("#_be_#");
          if (bits.length != 3) response == false;
        }
        if (response !== false) {
          $ ("#oneprefix").empty ();
          $ ("#oneprefix").append (bits [0]);
          $ ("#oneprefix").off ("click");
          $ ("#oneprefix").on ("click", oneVerseHtmlClicked);
        }
        if (response !== false) {
          // Destroy existing editor.
          if (quill) delete quill;
          // Load the html in the DOM.
          $ ("#oneeditor").empty ();
          $ ("#oneeditor").append (bits [1]);
          oneverseVerseLoaded = oneverseVerseLoading;
          oneverseEditorStatus (oneverseEditorVerseLoaded);
          // Create the editor based on the DOM's content.
          visualVerseEditorInitializeLoad ();
          quill.enable (oneverseEditorWriteAccess);
          // The browser may reformat the loaded html, so take the possible reformatted data for reference.
          oneverseLoadedText = $ (".ql-editor").html ();
          oneverseCaretMovedTimeoutStart ();
          // Create CSS for embedded styles.
          css4embeddedstyles ();
        }
        if (response !== false) {
          oneverseEditorChangeOffsets = []
          oneverseEditorChangeInserts = []
          oneverseEditorChangeDeletes = []
        }
        if (response !== false) {
          $ ("#onesuffix").empty ();
          $ ("#onesuffix").append (bits [2]);
          $ ("#onesuffix").off ("click");
          $ ("#onesuffix").on ("click", oneVerseHtmlClicked);
        }
        if (response !== false) {
          oneverseScrollVerseIntoView ();
          oneversePositionCaret ();
          // In case of network error, don't keep showing the notification.
          if (!oneverseReloadCozError) {
            if (oneverseReloadCozChanged)  {
              if (oneverseEditorWriteAccess) oneverseReloadAlert (oneverseEditorVerseUpdatedLoaded);
            }
          }
          oneverseReloadCozChanged = false;
          oneverseReloadCozError = false;
        }
        if (response === false) {
          // Checksum or other error: Reload.
          oneverseReloadCozError = true;
          oneverseEditorLoadVerse ();
        }
      },
    });
  }
}


function oneverseEditorUnload ()
{
  oneverseEditorSaveVerse (true);
}


function oneverseEditorSaveVerse (sync)
{
  if (oneverseSaving) {
    oneverseEditorChanged ();
    return;
  }
  if (!oneverseEditorWriteAccess) return;
  if (!oneverseBible) return;
  if (!oneverseBook) return;
  if (!oneverseVerseLoaded) return;
  var html = $ (".ql-editor").html ();
  if (html == oneverseLoadedText) return;
  oneverseEditorStatus (oneverseEditorVerseSaving);

  // Chapter identifier poller off as network latency may lead to problems if left on.
  oneverseIdPollerOff ();
  
  oneverseLoadedText = html;
  // oneverseChapterId = 0;
  oneverseSaveAsync = true;
  if (sync) oneverseSaveAsync = false;
  var encodedHtml = filter_url_plus_to_tag (html);
  var checksum = checksum_get (encodedHtml);
  oneverseSaving = true;
  $.ajax ({
    url: "save",
    type: "POST",
    async: oneverseSaveAsync,
    data: { bible: oneverseBible, book: oneverseBook, chapter: oneverseChapter, verse: oneverseVerseLoaded, html: encodedHtml, checksum: checksum, id: verseEditorUniqueID },
    error: function (jqXHR, textStatus, errorThrown) {
      oneverseEditorStatus (oneverseEditorVerseRetrying);
      oneverseLoadedText = "";
      oneverseEditorChanged ();
      if (!oneverseSaveAsync) oneverseEditorSaveVerse (true);
    },
    success: function (response) {
      oneverseEditorStatus (response);
    },
    complete: function (xhr, status) {
      oneverseSaveAsync = true;
      oneverseSaving = false;
      oneverseIdPollerOn ();
    }
  });
}


//
//
// Portion dealing with triggers for editor's content change.
//
//


var oneverseEditorChangeOffsets = []
var oneverseEditorChangeInserts = []
var oneverseEditorChangeDeletes = []


// Arguments: delta: Delta, oldContents: Delta, source: String
function visualVerseEditorTextChangeHandler (delta, oldContents, source)
{
  // Record the change.
  var retain = 0;
  var insert = 0;
  var del = 0;
  for (let i = 0; i < delta.ops.length; i++) {
    let obj = delta.ops[i];
    if (obj.retain) retain = obj.retain;
    // For Unicode handling, see:
    // https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/String/length
    if (obj.insert) insert = obj.insert.length;
    if (obj.delete) del = obj.delete;
  }
  oneverseEditorChangeOffsets.push(retain);
  oneverseEditorChangeInserts.push(insert);
  oneverseEditorChangeDeletes.push(del);
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
  oneverseEditorStatus (oneverseEditorWillSave);
  if (oneverseEditorChangedTimeout) {
    clearTimeout (oneverseEditorChangedTimeout);
  }
  //oneverseEditorChangedTimeout = setTimeout (oneverseEditorSaveVerse, 1000);
  oneverseEditorChangedTimeout = setTimeout (oneverseEditorTriggerSave, 1000);
}


function oneverseEditorTriggerSave ()
{
  if (!oneverseUpdateTrigger) {
    oneverseUpdateTrigger = true;
  } else {
    if (oneverseEditorChangedTimeout) {
      clearTimeout (oneverseEditorChangedTimeout);
    }
    oneverseEditorChangedTimeout = setTimeout (oneverseEditorTriggerSave, 400);
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
  if (message == oneverseEditorVerseUpdating) return;
  if (message == oneverseEditorVerseUpdated) return;
  notifyItError (message);
}


//
//
// Section for polling the server for updates on the loaded chapter.
//
//


var oneverseIdAjaxRequest;


function oneverseIdPollerOff ()
{
//  if (oneverseIdTimeout) {
//    clearTimeout (oneverseIdTimeout);
//  }
//  if (oneverseIdAjaxRequest && oneverseIdAjaxRequest.readystate != 4) {
//    oneverseIdAjaxRequest.abort();
//  }
}


function oneverseIdPollerOn ()
{
//  oneverseIdPollerOff ();
//  oneverseIdTimeout = setTimeout (oneverseEditorPollId, 1000);
}


function oneverseEditorPollId ()
{
  oneverseAjaxActive = true;
  oneverseIdAjaxRequest = $.ajax ({
    url: "../edit2/id",
    type: "GET",
    data: { bible: oneverseBible, book: oneverseBook, chapter: oneverseChapter },
    cache: false,
    success: function (response) {
      if (oneverseChapterId != 0) {
        if (response != oneverseChapterId) {
          // The chapter identifier changed.
          // That means that ikely there's updated text on the server.
          // Start the routine to load any possible updates into the editor.
          oneverseUpdateTrigger = true;
        }
      }
      oneverseChapterId = response;
    },
    error: function (jqXHR, textStatus, errorThrown) {
    },
    complete: function (xhr, status) {
      oneverseAjaxActive = false;
    }
  });
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
  if (!oneverseEditorWriteAccess) return;
  $.ajax ({
    url: "../edit2/styles",
    type: "GET",
    cache: false,
    success: function (response) {
      oneverseShowResponse (response);
      oneverseBindUnselectable ();
      oneverseDynamicClickHandlers ();
    },
  });
  return false;
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
  $.ajax ({
    url: "../edit2/styles",
    type: "GET",
    data: { style: style },
    cache: false,
    success: function (response) {
      response = response.split ("\n");
      var style = response [0];
      var action = response [1];
      if (action == "p") {
        oneverseApplyParagraphStyle (style);
      } else if (action == 'c') {
        oneverseApplyCharacterStyle (style);
      } else if (action == 'n') {
        oneverseApplyNotesStyle (style);
      } else if (action == "m") {
        oneverseApplyMonoStyle (style);
      }
    },
  });
}


function oneverseDisplayAllStyles ()
{
  $.ajax ({
    url: "../edit2/styles",
    type: "GET",
    data: { all: "" },
    cache: false,
    success: function (response) {
      oneverseShowResponse (response);
      oneverseBindUnselectable ();
      oneverseDynamicClickHandlers ();
    },
  });
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
  // If the user selects text, do nothing.
  var text = "";
  if (window.getSelection) {
    text = window.getSelection().toString();
  } else if (document.selection && document.selection.type != "Control") {
    text = document.selection.createRange().text;
  }
  if (text.length) return;
  
  var verse = "";
  
  var iterations = 0;
  var target = $(event.target);
  var tagName = target.prop("tagName");
  if (tagName == "P") target = $ (target.children ().last ());
  while ((iterations < 10) && (!target.hasClass ("v"))) {
    var previous = $(target.prev ());
    if (previous.length == 0) {
      target = $ (target.parent ().prev ());
      target = $ (target.children ().last ());
    } else {
      target = previous;
    }
    iterations++;
  }
                                          
  // Too many iterations: Undefined location.
  if (iterations >= 10) return
  
  if (target.length == 0) verse = "0";
  
  if (target.hasClass ("v")) {
    verse = target[0].innerText;
  }

  $.ajax ({
    url: "verse",
    type: "GET",
    data: { verse: verse },
    cache: false
  });
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


/*

Section for the coordinating timer.
This deals with the various events.
It monitors the ongoing AJAX actions for editing and saving and loading,
It decides which action to take.
It ensures that no two actions overlap or interfere with one another.
It also handles network latency,
by ensuring that the next call to the server
only occurs after the first call has been completed.
https://github.com/bibledit/cloud/issues/424

*/


var oneverseAjaxActive = false;
var oneversePollSelector = 0;
var oneversePollDate = new Date();
var oneverseUpdateTrigger = false;


function oneverseCoordinatingTimeout ()
{
  // Handle situation that an AJAX call is ongoing.
  if (oneverseAjaxActive) {
    
  }
  else if (oneverseUpdateTrigger) {
    oneverseUpdateTrigger = false;
    oneverseUpdateExecute (false);
  }
  // Handle situation that no process is ongoing.
  // Now the regular pollers can run again.
  else {
    // There are two regular pollers.
    // Wait 500 ms, then start one of the pollers.
    // So each poller runs once a second.
    var difference = new Date () - oneversePollDate;
    if (difference > 500) {
      oneversePollSelector++;
      if (oneversePollSelector > 1) oneversePollSelector = 0;
      if (oneversePollSelector == 0) {
        oneverseEditorPollId ();
      }
      if (oneversePollSelector == 1) {

      }
      oneversePollDate = new Date();
    }
  }
  setTimeout (oneverseCoordinatingTimeout, 100);
}


/*

Section for the smart editor updating logic.

*/


function oneverseUpdateExecute (sync) // Todo
{
  // Determine whether the conditions for an editor update are all met.
  var goodToGo = true;
  // if (!oneverseEditorWriteAccess) goodToGo = false;
  if (!oneverseBible) goodToGo = false;
  if (!oneverseBook) goodToGo = false;
  if (!oneverseVerseLoaded) goodToGo = false;
  if (!goodToGo) {
    return;
  }

  // Clear the editor's edits.
  // The user can continue making changes in the editor.
  // These changes get recorded.
  oneverseEditorChangeOffsets = [];
  oneverseEditorChangeInserts = [];
  oneverseEditorChangeDeletes = [];

  // A snapshot of the text originally loaded in the editor via AJAX.
  var encodedLoadedHtml = filter_url_plus_to_tag (oneverseLoadedText);

  // A a snapshot of the current editor text at this point of time.
  var html = $ (".ql-editor").html ();
  //if (html == oneverseLoadedText) return;
  var encodedEditedHtml = filter_url_plus_to_tag (html);
  
  // The editor "saves..." if there's changes, and "updates..." if there's no changes.
  if (html == oneverseLoadedText) {
    oneverseEditorStatus (oneverseEditorVerseUpdating);
  } else {
    oneverseEditorStatus (oneverseEditorVerseSaving);
  }

  // Normally the AJAX call is asynchronous.
  oneverseSaveAsync = true;
  if (sync) oneverseSaveAsync = false;

  var checksum = checksum_get (encodedEditedHtml);

  oneverseAjaxActive = true;

  oneverseIdAjaxRequest = $.ajax ({
    url: "update",
    type: "POST",
    async: oneverseSaveAsync,
    data: { bible: oneverseBible, book: oneverseBook, chapter: oneverseChapter, verse: oneverseVerseLoaded, loaded: encodedLoadedHtml, edited: encodedEditedHtml, checksum: checksum, id: verseEditorUniqueID },
    error: function (jqXHR, textStatus, errorThrown) {
      oneverseEditorStatus (oneverseEditorVerseRetrying);
      oneverseEditorChanged ();
      if (!oneverseSaveAsync) oneverseEditorSaveVerse (true);
    },
    success: function (response) {

      // Flag for editor read-write or read-only.
      oneverseEditorWriteAccess = checksum_readwrite (response);

      // Checksumming.
      response = checksum_receive (response);
      if (response !== false) {
        
        // Split the response into the separate bits.
        var bits = [];
        bits = response.split ("#_be_#");

        // The first bit is the feedback message to the user.
        oneverseEditorStatus (bits.shift());

        // The next bit is the new chapter identifier.
        oneverseChapterId = bits.shift();

        console.log (bits);
        // Apply the remaining data, the differences, to the editor.
        while (bits.length > 0) {
          var operator = bits.shift();
          var position = parseInt (bits.shift ());
          // At position 0 is a new line in the changes,
          // but this new line is not counted in Quill.
          if (position > 0) position--
          if (operator == "insert") {
            var text = bits.shift ();
            var style = bits.shift ();
            quill.insertText (position, text, {"character": style}, "silent");
          }
          if (operator == "delete") {
            quill.deleteText (position, 1, "silent");
          }
        }
        
      } else {
        // If the checksum is not valid, the response will become false.
        // Checksum error.
        oneverseEditorStatus (oneverseEditorVerseRetrying);
      }

      // The browser may reformat the loaded html, so take the possible reformatted data for reference.
      oneverseLoadedText = $ (".ql-editor").html ();
      
      // Create CSS for embedded styles.
      css4embeddedstyles ();
    },
    complete: function (xhr, status) {
      oneverseSaveAsync = true;
      oneverseAjaxActive = false;
    }
  });

}

