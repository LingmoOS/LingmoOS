/*
 * $Id: dialog.h,v 1.27 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKDIALOG_H
#define CDKDIALOG_H	1

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
 * Copyright 1999-2003,2012 Thomas E. Dickey
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
 * Declare definitions the dialog box may need.
 */
#define MAX_DIALOG_ROWS		50	/* unused by widgets */
#define MAX_DIALOG_BUTTONS	25	/* unused by widgets */
#define MIN_DIALOG_WIDTH	10

/*
 * Define the CDK dialog structure.
 */
struct SDialogBox {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
   chtype **	info;
   int *	infoLen;
   int *	infoPos;
   chtype **	buttonLabel;
   int *	buttonLen;
   int *	buttonPos;
   int		messageRows;
   int		buttonCount;
   int		currentButton;
   int		boxWidth;
   int		boxHeight;
   EExitType	exitType;
   boolean	separator;
   boolean	shadow;
   chtype	highlight;
};
typedef struct SDialogBox CDKDIALOG;

/*
 * This returns a CDK dialog widget pointer.
 */
CDKDIALOG *newCDKDialog (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xPos */,
		int		/* yPos */,
		CDK_CSTRING2	/* message */,
		int		/* Rows */,
		CDK_CSTRING2	/* buttons */,
		int		/* buttonCount */,
		chtype		/* highlight */,
		boolean		/* separator */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This activates the widget.
 */
int activateCDKDialog (
		CDKDIALOG *	/* dialog */,
		chtype *	/* actions */);

/*
 * This injects a single character into the widget.
 */
#define injectCDKDialog(obj,input) injectCDKObject(obj,input,Int)

/*
 * This sets multiple attributes of the widget.
 */
void setCDKDialog (
		CDKDIALOG *	/* dialog */,
		chtype		/* highlight */,
		boolean		/* separator */,
		boolean		/* Box */);

/*
 * This sets the highlight attribute for the buttons.
 */
void setCDKDialogHighlight (
		CDKDIALOG *	/* dialog */,
		chtype		/* highlight */);

chtype getCDKDialogHighlight (
		CDKDIALOG *	/* dialog */);

/*
 * This sets whether or not the dialog box will have a separator line.
 */
void setCDKDialogSeparator (
		CDKDIALOG *	/* dialog */,
		boolean		/* separator */);

boolean getCDKDialogSeparator (
		CDKDIALOG *	/* dialog */);

/*
 * This sets the box attribute of the widget.
 */
void setCDKDialogBox (
		CDKDIALOG *	/* dialog */,
		boolean		/* Box */);

boolean getCDKDialogBox (
		CDKDIALOG *	/* dialog */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDKDialogULChar(w,c)            setULCharOf(w,c)
#define setCDKDialogURChar(w,c)            setURCharOf(w,c)
#define setCDKDialogLLChar(w,c)            setLLCharOf(w,c)
#define setCDKDialogLRChar(w,c)            setLRCharOf(w,c)
#define setCDKDialogVerticalChar(w,c)      setVTCharOf(w,c)
#define setCDKDialogHorizontalChar(w,c)    setHZCharOf(w,c)
#define setCDKDialogBoxAttribute(w,c)      setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKDialogBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKDialogBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This draws the dialog box widget.
 */
#define drawCDKDialog(obj,box) drawCDKObject(obj,box)

/*
 * This erases the dialog box from the screen.
 */
#define eraseCDKDialog(obj) eraseCDKObject(obj)

/*
 * This moves the dialog box to a new screen location.
 */
#define moveCDKDialog(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This allows the user to position the widget on the screen interactively.
 */
#define positionCDKDialog(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the widget and all the memory associated with it.
 */
#define destroyCDKDialog(obj) destroyCDKObject(obj)

/*
 * This draws the dialog box buttons.
 */
void drawCDKDialogButtons (
		CDKDIALOG *	/* dialog */);

/*
 * These set the pre/post process functions of the dialog widget.
 */
#define setCDKDialogPreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDKDialogPostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

#ifdef __cplusplus
}
#endif

#endif /* CDKDIALOG_H */
#endif /* CDKINCLUDES */
