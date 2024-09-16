/*
 * $Id: label.h,v 1.21 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKLABEL_H
#define CDKLABEL_H	1

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
 * Copyright 2000-2003,2012 Thomas E. Dickey
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
 * Declare any definitions you need to make.
 */
#define MAX_LABEL_ROWS	300	/* unused by widgets */

/*
 * Declare the CDK label structure.
 */
struct SLabel {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
   chtype **	info;
   int *	infoLen;
   int *	infoPos;
   int		boxWidth;
   int		boxHeight;
   int		xpos;
   int		ypos;
   int		rows;
   boolean	shadow;
};
typedef struct SLabel CDKLABEL;

/*
 * This creates a new CDK label widget.
 */
CDKLABEL *newCDKLabel (
		CDKSCREEN *	/* screen */,
		int		/* xPos */,
		int		/* yPos */,
		CDK_CSTRING2	/* mesg */,
		int		/* rows */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This was added to make the builder simpler. All this will
 * do is call drawCDKLabel.
 */
void activateCDKLabel (
		CDKLABEL *	/* label */,
		chtype *	/* actions */);

/*
 * This sets multiple attributes of the widget.
 */
void setCDKLabel (
		CDKLABEL *	/* label */,
		CDK_CSTRING2	/* message */,
		int		/* lines */,
		boolean		/* Box */);

/*
 * This sets the contents of the label.
 */
void setCDKLabelMessage (
		CDKLABEL *	/* label */,
		CDK_CSTRING2	/* mesg */,
		int		/* lines */);
chtype **getCDKLabelMessage (
		CDKLABEL *	/* label */,
		int *		/* size */);

/*
 * This sets the box attribute of the widget.
 */
void setCDKLabelBox (
		CDKLABEL *	/* label */,
		boolean		/* Box */);
boolean getCDKLabelBox (
		CDKLABEL *	/* label */);

/*
 * This draws the label.
 */
#define drawCDKLabel(obj,Box) drawCDKObject(obj,Box)

/*
 * These set the drawing characters of the widget.
 */
#define setCDKLabelULChar(w,c)             setULCharOf(w,c)
#define setCDKLabelURChar(w,c)             setURCharOf(w,c)
#define setCDKLabelLLChar(w,c)             setLLCharOf(w,c)
#define setCDKLabelLRChar(w,c)             setLRCharOf(w,c)
#define setCDKLabelVerticalChar(w,c)       setVTCharOf(w,c)
#define setCDKLabelHorizontalChar(w,c)     setHZCharOf(w,c)
#define setCDKLabelBoxAttribute(w,c)       setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKLabelBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */
#define setCDKLabelBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This erases the label.
 */
#define eraseCDKLabel(obj) eraseCDKObject(obj)

/*
 * This destroys the label and the memory used by it.
 */
#define destroyCDKLabel(obj) destroyCDKObject(obj)

/*
 * This draws the label then waits for the user to press
 * the key defined by the 'key' parameter.
 */
char waitCDKLabel (
		CDKLABEL *	/* label */,
		char		/* key */);

/*
 * This moves the label.
 */
#define moveCDKLabel(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This allows the user to interactively position the label.
 */
#define positionCDKLabel(widget) positionCDKObject(ObjOf(widget),widget->win)

#ifdef __cplusplus
}
#endif

#endif /* CDKLABEL_H */
#endif /* CDKINCLUDES */
