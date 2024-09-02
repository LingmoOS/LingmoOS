/*
 * $Id: scroller.h,v 1.5 2013/09/01 21:56:04 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef SCROLLER_H
#define SCROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Copyright 2013 Thomas E. Dickey
 * Copyright 2013 Corentin Delorme
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, distribute with
 * modifications, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * ABOVE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */

#define SCROLLER_FIELDS \
   CDKOBJS  obj; \
   WINDOW * parent; \
   WINDOW * win; \
   WINDOW * scrollbarWin; \
   WINDOW * shadowWin; \
   int      titleAdj;   /* unused */ \
   chtype **    item; \
   int *    itemLen; \
   int *    itemPos; \
\
   int      currentTop; \
   int      currentItem; \
   int      currentHigh; \
\
   int      maxTopItem; \
   int      maxLeftChar; \
   int      maxchoicelen; \
   int      leftChar; \
   int      lastItem; \
   int      listSize; \
   int      boxWidth; \
   int      boxHeight; \
   int      viewSize; \
\
   int      scrollbarPlacement; \
   boolean  scrollbar; \
   int      toggleSize; /* size of scrollbar thumb/toggle */ \
   int      togglePos; /* position of scrollbar thumb/toggle */ \
   float    step; /* increment for scrollbar */ \
\
   EExitType    exitType; \
   boolean  shadow; \
   chtype   highlight

struct SScroller {
   /* This field must stay on top */
   SCROLLER_FIELDS;
};

typedef struct SScroller CDKSCROLLER;

void scroller_KEY_UP (CDKSCROLLER * /* scroller */);

void scroller_KEY_DOWN (CDKSCROLLER * /* scroller */);

void scroller_KEY_LEFT (CDKSCROLLER * /* scroller */);

void scroller_KEY_RIGHT (CDKSCROLLER * /* scroller */);

void scroller_KEY_PPAGE (CDKSCROLLER * /* scroller */);

void scroller_KEY_NPAGE (CDKSCROLLER * /* scroller */);

void scroller_KEY_HOME (CDKSCROLLER * /* scroller */);

void scroller_KEY_END (CDKSCROLLER * /* scroller */);

void scroller_FixCursorPosition (CDKSCROLLER * /* scroller */);

void scroller_SetPosition (CDKSCROLLER * /* scroller */, int /* item */);

int scroller_MaxViewSize (CDKSCROLLER * /* scroller */);

void scroller_SetViewSize (CDKSCROLLER * /* scroller */, int /* size */);

#ifdef __cplusplus
}
#endif

#endif /* SCROLLER_H */
#endif /* CDKINCLUDES */
