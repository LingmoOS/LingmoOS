/*
 * $Id: mentry.h,v 1.24 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKMENTRY_H
#define CDKMENTRY_H	1

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
 * Define the CDK multiple line entry field widget.
 */
typedef struct SMentry CDKMENTRY;
typedef void (*MENTRYCB) (CDKMENTRY *mentry, chtype character);

struct SMentry {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
   WINDOW *	labelWin;
   WINDOW *	fieldWin;
   int		titleAdj;
   chtype *	label;
   int		labelLen;
   int		labelPos;
   chtype	fieldAttr;
   int		fieldWidth;
   int		boxHeight;
   int		boxWidth;
   char *	info;
   int		totalWidth;
   int		rows;
   int		currentRow;
   int		currentCol;
   int		topRow;
   EDisplayType dispType;
   int		min;
   int		logicalRows;
   EExitType	exitType;
   boolean	shadow;
   chtype	filler;
   chtype	hidden;
   MENTRYCB	callbackfn;
};

/*
 * This creates a new CDK mentry field pointer.
 */
CDKMENTRY *newCDKMentry (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		const char *	/* title */,
		const char *	/* label */,
		chtype		/* fieldAttr */,
		chtype		/* filler */,
		EDisplayType	/* disptype */,
		int		/* fieldWidth */,
		int		/* fieldrows */,
		int		/* logicalRows */,
		int		/* min */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This activates the widget.
 */
char *activateCDKMentry (
		CDKMENTRY *	/* mentry */,
		chtype *	/* input */);

/*
 * This injects a single character into the widget.
 */
#define injectCDKMentry(obj,input) injectCDKObject(obj,input,String)

/*
 * These set specific attributes of the widget.
 */
void setCDKMentry (
		CDKMENTRY *	/* mentry */,
		const char *	/* value */,
		int		/* min */,
		boolean		/* Box */);

/*
 * This sets the value of the widget.
 */
void setCDKMentryValue (
		CDKMENTRY *	/* mentry */,
		const char *	/* value */);

char *getCDKMentryValue (
		CDKMENTRY *	/* mentry */);

/*
 * This sets the minimum length of the value for the widget.
 */
void setCDKMentryMin (
		CDKMENTRY *	/* mentry */,
		int		/* min */);

int getCDKMentryMin (
		CDKMENTRY *	/* mentry */);

/*
 * This sets the filler character to use when drawing the widget.
 */
void setCDKMentryFillerChar (
		CDKMENTRY *	/* mentry */,
		chtype		/* filler */);

chtype getCDKMentryFillerChar (
		CDKMENTRY *	/* mentry */);

/*
 * This sets the character to use when a hidden character type is used.
 */
void setCDKMentryHiddenChar (
		CDKMENTRY *	/* mentry */,
		chtype		/* character */);

chtype getCDKMentryHiddenChar (
		CDKMENTRY *	/* mentry */);

/*
 * This sets the box attribute of the mentry widget.
 */
void setCDKMentryBox (
		CDKMENTRY *	/* mentry */,
		boolean		/* Box */);

boolean getCDKMentryBox (
		CDKMENTRY *	/* mentry */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDKMentryULChar(w,c)            setULCharOf(w,c)
#define setCDKMentryURChar(w,c)            setURCharOf(w,c)
#define setCDKMentryLLChar(w,c)            setLLCharOf(w,c)
#define setCDKMentryLRChar(w,c)            setLRCharOf(w,c)
#define setCDKMentryVerticalChar(w,c)      setVTCharOf(w,c)
#define setCDKMentryHorizontalChar(w,c)    setHZCharOf(w,c)
#define setCDKMentryBoxAttribute(w,c)      setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKMentryBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKMentryBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This draws the widget on the screen.
 */
#define drawCDKMentry(obj,Box) drawCDKObject(obj,Box)

/*
 * This erases the widget from the screen.
 */
#define eraseCDKMentry(obj) eraseCDKObject(obj)

/*
 * This cleans out the information in the widget.
 */
void cleanCDKMentry (
		CDKMENTRY *	/* mentry */);

/*
 * This moves the widget to the given location.
 */
#define moveCDKMentry(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This interactively moves the widget on the screen.
 */
#define positionCDKMentry(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the widget pointer.
 */
#define destroyCDKMentry(obj) destroyCDKObject(obj)

/*
 * This draws the field of the widget.
 */
void drawCDKMentryField (
		CDKMENTRY *	/* mentry */);

/*
 * This sets the widgets main callback.
 */
void setCDKMentryCB (
		CDKMENTRY *	/* mentry */,
		MENTRYCB	/* callback */);

/*
 * These set the pre/post process callback functions.
 */
#define setCDKMentryPreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDKMentryPostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

#ifdef __cplusplus
}
#endif

#endif /* CDKMENTRY_H */
#endif /* CDKINCLUDES */
