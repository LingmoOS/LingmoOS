/*
 * $Id: selection.h,v 1.29 2013/09/01 18:06:41 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKSELECTION_H
#define CDKSELECTION_H	1

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
 * Declare selection list definitions.
 */
#define MAX_CHOICES	100	/* unused by widgets */

/*
 * Define the CDK selection widget structure.
 */
struct SSelection {
   /* This field must stay on top */
   SCROLLER_FIELDS;

   chtype **	choice;
   int *	choicelen;
   int		choiceCount;
   int *	selections;
   int *	mode;
};
typedef struct SSelection CDKSELECTION;

/*
 * This creates a new pointer to a selection widget.
 */
CDKSELECTION *newCDKSelection (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		int		/* spos */,
		int		/* height */,
		int		/* width */,
		const char *	/* title */,
		CDK_CSTRING2	/* list */,
		int		/* listSize */,
		CDK_CSTRING2	/* choices */,
		int		/* choiceSize */,
		chtype		/* highlight */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This activates the selection widget.
 */
int activateCDKSelection (
		CDKSELECTION *	/* selection */,
		chtype *	/* actions */);

/*
 * This injects a single character into the widget.
 */
#define injectCDKSelection(obj,input) injectCDKObject(obj,input,Int)

/*
 * This draws the selection widget.
 */
#define drawCDKSelection(obj,Box) drawCDKObject(obj,Box)

/*
 * This erases the selection widget from the screen.
 */
#define eraseCDKSelection(obj) eraseCDKObject(obj)

/*
 * This moves the widget to the given location.
 */
#define moveCDKSelection(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This interactively moves the widget on the screen.
 */
#define positionCDKSelection(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the widget and all associated memory.
 */
#define destroyCDKSelection(obj) destroyCDKObject(obj)

/*
 * This sets various attributes of the selection widget.
 */
void setCDKSelection (
		CDKSELECTION *	/* selection */,
		chtype		/* highlight */,
		int *		/* defChoices */,
		boolean		/* Box */);

/*
 * This sets the contents of the selection list.
 */
void setCDKSelectionItems (
		CDKSELECTION *	/* selection */,
		CDK_CSTRING2	/* list */,
		int		/* listSize */);

int getCDKSelectionItems (
		CDKSELECTION *	/* selection */,
		char **		/* list */);

/*
 *
 */
void setCDKSelectionTitle (
		CDKSELECTION *	/* selection */,
		const char *	/* title */);

char *getCDKSelectionTitle (
		CDKSELECTION *	/* selection */);

/*
 * This sets the selection list highlight bar.
 */
void setCDKSelectionHighlight (
		CDKSELECTION *	/* selection */,
		chtype		/* highlight */);

chtype getCDKSelectionHighlight (
		CDKSELECTION *	/* selection */);

/*
 * set/get the current item index
 */
void setCDKSelectionCurrent (
		CDKSELECTION *	/* selection */,
		int		/* index */);

int getCDKSelectionCurrent (
		CDKSELECTION *	/* selection */);

/*
 * This sets the choices of the selection list.
 */
void setCDKSelectionChoices (
		CDKSELECTION *	/* selection */,
		int *		/* choices */);

int *getCDKSelectionChoices (
		CDKSELECTION *	/* selection */);

void setCDKSelectionChoice (
		CDKSELECTION *	/* selection */,
		int		/* index */,
		int		/* choice */);

int getCDKSelectionChoice (
		CDKSELECTION *	/* selection */,
		int		/* index */);

/*
 * This sets the modes of the items in the selection list. Currently
 * there are only two: editable=0 and read-only=1
 */
void setCDKSelectionModes (
		CDKSELECTION *	/* selection */,
		int *		/* modes */);

int *getCDKSelectionModes (
		CDKSELECTION *	/* selection */);

void setCDKSelectionMode (
		CDKSELECTION *	/* selection */,
		int		/* index */,
		int		/* mode */);

int getCDKSelectionMode (
		CDKSELECTION *	/* selection */,
		int		/* index */);

/*
 * This sets the box attribute of the widget.
 */
void setCDKSelectionBox (
		CDKSELECTION *	/* selection */,
		boolean		/* Box */);

boolean getCDKSelectionBox (
		CDKSELECTION *	/* selection */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDKSelectionULChar(w,c)         setULCharOf(w,c)
#define setCDKSelectionURChar(w,c)         setURCharOf(w,c)
#define setCDKSelectionLLChar(w,c)         setLLCharOf(w,c)
#define setCDKSelectionLRChar(w,c)         setLRCharOf(w,c)
#define setCDKSelectionVerticalChar(w,c)   setVTCharOf(w,c)
#define setCDKSelectionHorizontalChar(w,c) setHZCharOf(w,c)
#define setCDKSelectionBoxAttribute(w,c)   setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKSelectionBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */
#define setCDKSelectionBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * These set the pre/post process callback functions.
 */
#define setCDKSelectionPreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDKSelectionPostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

#ifdef __cplusplus
}
#endif

#endif /* CDKSELECTION_H */
#endif /* CDKINCLUDES */
