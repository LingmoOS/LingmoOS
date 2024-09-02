/*
 * $Id: entry.h,v 1.30 2016/11/20 14:41:44 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKENTRY_H
#define CDKENTRY_H	1

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
 * Changes 1999-2015,2016 copyright Thomas E. Dickey
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
 * Define the CDK entry widget structure.
 */
typedef struct SEntry CDKENTRY;
typedef void (*ENTRYCB) (struct SEntry *entry, chtype character);

struct SEntry {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
   WINDOW *	labelWin;
   WINDOW *	fieldWin;
   chtype *	label;
   int		labelLen;
   int		titleAdj;
   chtype	fieldAttr;
   int		fieldWidth;
   char *	info;
   int		infoWidth;
   int		screenCol;
   int		leftChar;
   int		min;
   int		max;
   int		boxWidth;
   int		boxHeight;
   EExitType	exitType;
   EDisplayType dispType;
   boolean	shadow;
   chtype	filler;
   chtype	hidden;
   ENTRYCB	callbackfn;
   void		*callbackData;
};

/*
 * This sets the callback function of the button's argument.
 */
#define setCDKEntryCBArgs(entry, argPtr)	((entry)->callbackData = (void*)(argPtr))

#define getCDKEntryCBArgs(entry, argType)	((argType) ((entry)->callbackData))

/*
 * This creates a pointer to a new CDK entry widget.
 */
CDKENTRY *newCDKEntry (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		const char *	/* title */,
		const char *	/* label */,
		chtype		/* fieldAttrib */,
		chtype		/* filler */,
		EDisplayType	/* disptype */,
		int		/* fieldWidth */,
		int		/* min */,
		int		/* max */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This activates the entry widget.
 */
char *activateCDKEntry (
		CDKENTRY *	/* entry */,
		chtype *	/* actions */);

/*
 * This injects a single character into the widget.
 */
#define injectCDKEntry(obj,input) injectCDKObject(obj,input,String)

/*
 * This sets various attributes of the entry field.
 */
void setCDKEntry (
		CDKENTRY *	/* entry */,
		const char *	/* value */,
		int		/* min */,
		int		/* max */,
		boolean		/* Box */);

/*
 * This sets the value of the entry field.
 */
void setCDKEntryValue (
		CDKENTRY *	/* entry */,
		const char *	/* value */);
char *getCDKEntryValue (
		CDKENTRY *	/* entry */);

/*
 * This sets the maximum length of a string allowable for
 * the given widget.
 */
void setCDKEntryMax (
		CDKENTRY *	/* entry */,
		int		/* max */);
int getCDKEntryMax (
		CDKENTRY *	/* entry */);

/*
 * This sets the minimum length of a string allowable for
 * the given widget.
 */
void setCDKEntryMin (
		CDKENTRY *	/* entry */,
		int		/* min */);
int getCDKEntryMin (
		CDKENTRY *	/* entry */);

/*
 * This sets the filler character of the entry field.
 */
void setCDKEntryFillerChar (
		CDKENTRY *	/* entry */,
		chtype		/* fillerCharacter */);
chtype getCDKEntryFillerChar (
		CDKENTRY *	/* entry */);

/*
 * This sets the character to use when a hidden type is used.
 */
void setCDKEntryHiddenChar (
		CDKENTRY *	/* entry */,
		chtype		/* hiddenCharacter */);
chtype getCDKEntryHiddenChar (
		CDKENTRY *	/* entry */);

/*
 * This sets the box attribute of the widget.
 */
void setCDKEntryBox (
		CDKENTRY *	/* entry */,
		boolean		/* Box */);
boolean getCDKEntryBox (
		CDKENTRY *	/* entry */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDKEntryULChar(w,c)             setULCharOf(w,c)
#define setCDKEntryURChar(w,c)             setURCharOf(w,c)
#define setCDKEntryLLChar(w,c)             setLLCharOf(w,c)
#define setCDKEntryLRChar(w,c)             setLRCharOf(w,c)
#define setCDKEntryVerticalChar(w,c)       setVTCharOf(w,c)
#define setCDKEntryHorizontalChar(w,c)     setHZCharOf(w,c)
#define setCDKEntryBoxAttribute(w,c)       setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKEntryBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKEntryBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This sets the attribute of the entry field.
 */
void setCDKEntryHighlight (
		CDKENTRY *	/* entry */,
		chtype		/* highlight */,
		boolean		/* cursor */);

/*
 * This draws the entry field.
 */
#define drawCDKEntry(obj,box) drawCDKObject(obj,box)

/*
 * This erases the widget from the screen.
 */
#define eraseCDKEntry(obj) eraseCDKObject(obj)

/*
 * This cleans out the value of the entry field.
 */
void cleanCDKEntry (
		CDKENTRY *	/* entry */);

/*
 * This moves the widget to the given location.
 */
#define moveCDKEntry(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This is an interactive method of moving the widget.
 */
#define positionCDKEntry(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the widget and all the memory associated with the widget.
 */
#define destroyCDKEntry(obj) destroyCDKObject(obj)

/*
 * This sets the callback to the entry fields main handler
 */
void setCDKEntryCB (
		CDKENTRY *	/* entry */,
		ENTRYCB		/* callback */);

/*
 * These set the callbacks to the pre and post process functions.
 */
#define setCDKEntryPreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDKEntryPostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

#ifdef __cplusplus
}
#endif

#endif /* CDKENTRY_H */
#endif /* CDKINCLUDES */
