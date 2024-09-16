/*
 * $Id: swindow.h,v 1.25 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKSWINDOW_H
#define CDKSWINDOW_H	1

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
 * Copyright 1999-2004,2012 Thomas E. Dickey
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
 * Define the CDK scrolling window widget structure.
 */
struct SSwindow {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	fieldWin;
   WINDOW *	shadowWin;
   chtype **	list;
   int *	listPos;
   int *	listLen;
   int		titleAdj;
   int		listSize;
   int		boxHeight;
   int		boxWidth;
   int		viewSize;
   int		maxTopLine;
   int		currentTop;
   int		leftChar;
   int		maxLeftChar;
   int		widestLine;
   int		saveLines;
   EExitType	exitType;
   boolean	shadow;
};
typedef struct SSwindow CDKSWINDOW;
typedef void (*SWINDOWCB) (CDKSWINDOW *swindow, chtype input);

/*
 * This creates a new pointer to a scrolling window widget.
 */
CDKSWINDOW *newCDKSwindow (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		int		/* height */,
		int		/* width */,
		const char *	/* title */,
		int		/* saveLines */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This activates the scrolling window.
 */
void activateCDKSwindow (
		CDKSWINDOW *	/* swindow */,
		chtype *	/* actions */);

/*
 * This injects a single character into the scrolling window.
 */
#define injectCDKSwindow(obj,input) injectCDKObject(obj,input,Int)

/*
 * This executes the given command and puts the output of the
 * command into the scrolling window.
 */
int execCDKSwindow (
		CDKSWINDOW *	/* swindow */,
		const char *	/* command */,
		int		/* insertPos */);

/*
 * This dumps the contents of the scrolling window to the given filename.
 */
int dumpCDKSwindow (
		CDKSWINDOW *	/* swindow */,
		const char *	/* filename */);

/*
 * This jumps to the given line in the window.
 */
void jumpToLineCDKSwindow (
		CDKSWINDOW *	/* swindow */,
		int		/* line */);

/*
 * This saves the contents of the scrolling window via an
 * interactive window.
 */
void saveCDKSwindowInformation (
		CDKSWINDOW *	/* swindow */);

/*
 * This loads the window up with information from a filename
 * interactively provided.
 */
void loadCDKSwindowInformation (
		CDKSWINDOW *	/* swindow */);

/*
 * These functions set the attributes of the scrolling window.
 */
void setCDKSwindow (
		CDKSWINDOW *	/* swindow */,
		CDK_CSTRING2	/* info */,
		int		/* lines */,
		boolean		/* Box */);

/*
 * This sets the contents of the scrolling window.
 */
void setCDKSwindowContents (
		CDKSWINDOW *	/* swindow */,
		CDK_CSTRING2	/* info */,
		int		/* lines */);

chtype **getCDKSwindowContents (
		CDKSWINDOW *	/* swindow */,
		int *		/* size */);

/*
 * This sets the box attribute of the scrolling window.
 */
void setCDKSwindowBox (
		CDKSWINDOW *	/* swindow */,
		boolean		/* Box */);

boolean getCDKSwindowBox (
		CDKSWINDOW *	/* swindow */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDKSwindowULChar(w,c)           setULCharOf(w,c)
#define setCDKSwindowURChar(w,c)           setURCharOf(w,c)
#define setCDKSwindowLLChar(w,c)           setLLCharOf(w,c)
#define setCDKSwindowLRChar(w,c)           setLRCharOf(w,c)
#define setCDKSwindowVerticalChar(w,c)     setVTCharOf(w,c)
#define setCDKSwindowHorizontalChar(w,c)   setHZCharOf(w,c)
#define setCDKSwindowBoxAttribute(w,c)     setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKSwindowBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKSwindowBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This draws the scrolling window on the screen.
 */
#define drawCDKSwindow(obj,Box) drawCDKObject(obj,Box)

/*
 * This removes the widget from the screen.
 */
#define eraseCDKSwindow(obj) eraseCDKObject(obj)

/*
 * This cleans out all of the information from the window.
 */
void cleanCDKSwindow (
		CDKSWINDOW *	/* swindow */);

/*
 * This adds a line to the window.
 */
void addCDKSwindow (
		CDKSWINDOW *	/* swindow */,
		const char *	/* info */,
		int		/* insertPos */);

/*
 * This trims lines from the window.
 */
void trimCDKSwindow (
		CDKSWINDOW *	/* swindow */,
		int		/* start */,
		int		/* finish */);

/*
 * This moves the window to the given location.
 */
#define moveCDKSwindow(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This interactively positions the widget on the screen.
 */
#define positionCDKSwindow(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the widget and all associated memory.
 */
#define destroyCDKSwindow(obj) destroyCDKObject(obj)

/*
 * These set the pre/post process callback functions.
 */
#define setCDKSwindowPreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDKSwindowPostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

#ifdef __cplusplus
}
#endif

#endif /* CDKSWINDOW_H */
#endif /* CDKINCLUDES */
