/*
 * $Id: alphalist.h,v 1.26 2014/01/19 01:58:40 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKALPHALIST_H
#define CDKALPHALIST_H	1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef CDK_H
#define CDKINCLUDES
#include <cdk.h>
#undef CDKINCLUDES
#include <binding.h>
#include <cdkscreen.h>
#include <cdk_objs.h>
#endif

/*
 * Changes 1999-2012,2014 copyright Thomas E. Dickey
 * Copyright 1999, Mike Glover
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

#include <entry.h>
#include <scroll.h>

/*
 * Define the CDK alphalist widget structure.
 */
struct SAlphalist {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
   CDKENTRY *	entryField;
   CDKSCROLL *	scrollField;
   char **	list;
   int		listSize;
   int		xpos;
   int		ypos;
   int		height;
   int		width;
   int		boxHeight;
   int		boxWidth;
   chtype	highlight;
   chtype	fillerChar;
   boolean	shadow;
   EExitType	exitType;
};
typedef struct SAlphalist CDKALPHALIST;

/*
 * This creates a pointer to a new CDK alphalist widget.
 */
CDKALPHALIST *newCDKAlphalist (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		int		/* height */,
		int		/* width */,
		const char *	/* title */,
		const char *	/* label */,
		CDK_CSTRING *	/* list */,
		int		/* listSize */,
		chtype		/* fillerChar */,
		chtype		/* highlight */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This allows the user to interact with the widget.
 */
char *activateCDKAlphalist (
		CDKALPHALIST *	/* alphalist */,
		chtype *	/* actions */);

/*
 * This injects a single character into the widget.
 */
#define injectCDKAlphalist(obj,input) injectCDKObject(obj,input,String)

/*
 * This sets multiple attributes of the alphalist widget.
 */
void setCDKAlphalist (
		CDKALPHALIST *	/* alphalist */,
		CDK_CSTRING *	/* list */,
		int		/* listSize */,
		chtype		/* fillerChar */,
		chtype		/* highlight */,
		boolean		/* Box */);

/*
 * This sets the contents of the alpha list.
 */
void setCDKAlphalistContents (
		CDKALPHALIST *	/* alphalist */,
		CDK_CSTRING *	/* list */,
		int		/* listSize */);

char **getCDKAlphalistContents (
		CDKALPHALIST *	/* alphalist */,
		int *		/* size */);

/*
 * Get/set the current position in the scroll-widget.
 */
int getCDKAlphalistCurrentItem(
		CDKALPHALIST *	/* widget */);

void setCDKAlphalistCurrentItem(
		CDKALPHALIST *	/* widget */,
		int		/* item */);

/*
 * This sets the filler character of the entry field of the alphalist.
 */
void setCDKAlphalistFillerChar (
		CDKALPHALIST *	/* alphalist */,
		chtype		/* fillerCharacter */);

chtype getCDKAlphalistFillerChar (
		CDKALPHALIST *	/* alphalist */);

/*
 * This sets the highlight bar attributes.
 */
void setCDKAlphalistHighlight (
		CDKALPHALIST *	/* alphalist */,
		chtype		/* highlight */);

chtype getCDKAlphalistHighlight (
		CDKALPHALIST *	/* alphalist */);

/*
 * This sets the box attribute of the widget.
 */
void setCDKAlphalistBox (
		CDKALPHALIST *	/* alphalist */,
		boolean		/* Box */);

boolean getCDKAlphalistBox (
		CDKALPHALIST *	/* alphalist */);

/*
 * These functions set the drawing characters of the widget.
 */
#define setCDKAlphalistULChar(w,c)         setULCharOf(w,c)
#define setCDKAlphalistURChar(w,c)         setURCharOf(w,c)
#define setCDKAlphalistLLChar(w,c)         setLLCharOf(w,c)
#define setCDKAlphalistLRChar(w,c)         setLRCharOf(w,c)
#define setCDKAlphalistVerticalChar(w,c)   setVTCharOf(w,c)
#define setCDKAlphalistHorizontalChar(w,c) setHZCharOf(w,c)
#define setCDKAlphalistBoxAttribute(w,c)   setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKAlphalistBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */
#define setCDKAlphalistBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This draws the widget on the screen.
 */
#define drawCDKAlphalist(obj,box) drawCDKObject(obj,box)

/*
 * This removes the widget from the screen.
 */
#define eraseCDKAlphalist(obj) eraseCDKObject(obj)

/*
 * This moves the widget to the location specified.
 */
#define moveCDKAlphalist(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This allows the user to interactively position the widget.
 */
#define positionCDKAlphalist(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the widget and all the memory associated with it.
 */
#define destroyCDKAlphalist(obj) destroyCDKObject(obj)

/*
 * These functions set the pre and post process functions for the widget.
 */
void setCDKAlphalistPreProcess (
		CDKALPHALIST *	/* alphalist */,
		PROCESSFN	/* callback */,
		void *		/* data */);

void setCDKAlphalistPostProcess (
		CDKALPHALIST *	/* alphalist */,
		PROCESSFN	/* callback */,
		void *		/* data */);

#ifdef __cplusplus
}
#endif

#endif /* CDKALPHALIST_H */
#endif /* CDKINCLUDES */
