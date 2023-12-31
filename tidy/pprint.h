#pragma clang diagnostic ignored "-Wunused-parameter"
#ifndef __PPRINT_H__
#define __PPRINT_H__

/* pprint.h -- pretty print parse tree  
  
   (c) 1998-2007 (W3C) MIT, ERCIM, Keio University
   See tidy.h for the copyright notice.
  
*/

#include "forward.h"

/*
  Block-level and unknown elements are printed on
  new lines and their contents indented 2 spaces

  Inline elements are printed inline.

  Inline content is wrapped on spaces (except in
  attribute values or preformatted text, after
  start tags and before end tags
*/

#define NORMAL        0u
#define PREFORMATTED  1u
#define COMMENT       2u
#define ATTRIBVALUE   4u
#define NOWRAP        8u
#define CDATA         16u


/* The pretty printer keeps at most two lines of text in the
** buffer before flushing output.  We need to capture the
** indent state (indent level) at the _beginning_ of _each_
** line, not the end of just the second line.
**
** We must also keep track "In Attribute" and "In String"
** states at the _end_ of each line, 
*/

typedef struct _TidyIndent
{
    int spaces;
    int attrValStart;
    int attrStringStart;
} TidyIndent;

typedef struct _TidyPrintImpl
{
    TidyAllocator *allocator; /* Allocator */

    uint *linebuf;
    uint lbufsize;
    uint linelen;
    uint wraphere;
    uint line;
  
    uint ixInd;
    TidyIndent indent[2];  /* Two lines worth of indent state */
} TidyPrintImpl;


TY_PRIVATE void TY_(InitPrintBuf)( TidyDocImpl* doc );
TY_PRIVATE void TY_(FreePrintBuf)( TidyDocImpl* doc );

TY_PRIVATE void TY_(PFlushLine)( TidyDocImpl* doc, uint indent );


/* print just the content of the body element.
** useful when you want to reuse material from
** other documents.
** 
** -- Sebastiano Vigna <vigna@dsi.unimi.it>
*/

TY_PRIVATE void TY_(PrintBody)( TidyDocImpl* doc );       /* you can print an entire document */
                                          /* node as body using PPrintTree() */

TY_PRIVATE void TY_(PPrintTree)( TidyDocImpl* doc, uint mode, uint indent, Node *node );

TY_PRIVATE void TY_(PPrintXMLTree)( TidyDocImpl* doc, uint mode, uint indent, Node *node );

/*\
 * 20150515 - support using tabs instead of spaces
\*/
TY_PRIVATE void TY_(PPrintTabs)(void);
TY_PRIVATE void TY_(PPrintSpaces)(void);

#endif /* __PPRINT_H__ */
