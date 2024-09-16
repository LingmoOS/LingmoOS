/*
 * $Id: scroll.h,v 1.34 2013/09/01 18:06:41 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKSCROLL_H
#define CDKSCROLL_H	1

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

#include <scroller.h>

/*
 * Copyright 1999-2012,2013 Thomas E. Dickey
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

/*
 * Declare scrolling list definitions.
 */
#define	NUMBERS		TRUE
#define	NONUMBERS	FALSE

/*
 * Declare scrolling list definitions.
 */
struct SScroll {
   /* This field must stay on top */
   SCROLLER_FIELDS;

   boolean	numbers;	/* */
   chtype	titlehighlight;	/* */
   WINDOW	*listWin;
};
typedef struct SScroll CDKSCROLL;

/*
 * This creates a new CDK scrolling list pointer.
 */
CDKSCROLL *newCDKScroll (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		int		/* spos */,
		int		/* height */,
		int		/* width */,
		const char *	/* title */,
		CDK_CSTRING2	/* itemList */,
		int		/* items */,
		boolean		/* numbers */,
		chtype		/* highlight */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This activates the scrolling list.
 */
int activateCDKScroll (
		CDKSCROLL *	/* scroll */,
		chtype *	/* actions */);

/*
 * This injects a single character into the scrolling list.
 */
#define injectCDKScroll(obj,input) injectCDKObject(obj,input,Int)

/*
 * This sets various attributes of the scrolling list.
 */
void setCDKScroll (
		CDKSCROLL *	/* scroll */,
		CDK_CSTRING2	/* itemList */,
		int		/* items */,
		boolean		/* numbers */,
		chtype		/* highlight */,
		boolean		/* Box */);

void setCDKScrollPosition (
		CDKSCROLL *	/* scroll */,
		int		/* item */);

int  getCDKScrollCurrent(
		CDKSCROLL *	/* scroll */);

void setCDKScrollCurrent(
		CDKSCROLL *	/* scroll */,
		int		/* i */);

int  getCDKScrollCurrentTop(
		CDKSCROLL *	/* scroll */);

void setCDKScrollCurrentTop(
		CDKSCROLL *	/* scroll */,
		int		/* i */);

int  getCDKScrollCurrentItem(
		CDKSCROLL *	/* scroll */);

void setCDKScrollCurrentItem(
		CDKSCROLL *	/* scroll */,
		int		/* i */);

/*
 * This sets the contents of the scrolling list.
 */
void setCDKScrollItems (
		CDKSCROLL *	/* scroll */,
		CDK_CSTRING2	/* itemList */,
		int		/* items */,
		boolean		/* numbers */);

int getCDKScrollItems (
		CDKSCROLL *	/* scroll */,
		char **		/* itemList */);

/*
 * This sets the highlight bar of the scrolling list.
 */
void setCDKScrollHighlight (
		CDKSCROLL *	/* scroll */,
		chtype		/* highlight */);

chtype getCDKScrollHighlight (
		CDKSCROLL *	/* scroll */,
		chtype		/* highlight */);

/*
 * This sets the box attribute of the widget.
 */
void setCDKScrollBox (
		CDKSCROLL *	/* scroll */,
		boolean		/* Box */);

boolean getCDKScrollBox (
		CDKSCROLL *	/* scroll */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDKScrollULChar(w,c)            setULCharOf(w,c)
#define setCDKScrollURChar(w,c)            setURCharOf(w,c)
#define setCDKScrollLLChar(w,c)            setLLCharOf(w,c)
#define setCDKScrollLRChar(w,c)            setLRCharOf(w,c)
#define setCDKScrollVerticalChar(w,c)      setVTCharOf(w,c)
#define setCDKScrollHorizontalChar(w,c)    setHZCharOf(w,c)
#define setCDKScrollBoxAttribute(w,c)      setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */ 
#define setCDKScrollBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKScrollBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This adds a single item to the end of scrolling list.
 */
void addCDKScrollItem (
		CDKSCROLL *	/* scroll */,
		const char *	/* item */);

/*
 * This adds a single item into the scrolling list.
 */
void insertCDKScrollItem (
		CDKSCROLL *	/* scroll */,
		const char *	/* item */);

/*
 * This deletes a single item from the scrolling list.
 */
void deleteCDKScrollItem (
		CDKSCROLL *	/* scroll */,
		int		/* position */);

/*
 * This draws the scrolling list on the screen.
 */
#define drawCDKScroll(obj,Box) drawCDKObject(obj,Box)

/*
 * This removes the scrolling list from the screen.
 */
#define eraseCDKScroll(obj) eraseCDKObject(obj)

/*
 * This moves the widget to the given location.
 */
#define moveCDKScroll(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This allows the user to interactively position the widget on the screen.
 */
#define positionCDKScroll(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the widget and all associated memory.
 */
#define destroyCDKScroll(obj) destroyCDKObject(obj)

/*
 * These set the scrolling list pre/post process functions.
 */
#define setCDKScrollPreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDKScrollPostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

#ifdef __cplusplus
}
#endif

#endif /* CDKSCROLL_H */
#endif /* CDKINCLUDES */
