/*
 * $Id: itemlist.h,v 1.25 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKITEMLIST_H
#define CDKITEMLIST_H	1

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
 * Define the CDK itemlist widget structure.
 */
struct SItemList {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
   WINDOW *	labelWin;
   WINDOW *	fieldWin;
   int		titleAdj;
   chtype *	label;
   int		labelLen;
   int		fieldWidth;
   chtype **	item;
   int *	itemPos;
   int *	itemLen;
   int		listSize;
   int		currentItem;
   int		defaultItem;
   int		boxWidth;
   int		boxHeight;
   EExitType	exitType;
   boolean	shadow;
};
typedef struct SItemList CDKITEMLIST;

/*
 * This creates a pointer to a CDK itemlist widget.
 */
CDKITEMLIST *newCDKItemlist (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		const char *	/* title */,
		const char *	/* label */,
		CDK_CSTRING2	/* itemlist */,
		int		/* count */,
		int		/* defaultItem */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This activates the itemlist widget.
 */
int activateCDKItemlist (
		CDKITEMLIST *	/* itemlist */,
		chtype *	/* actions */);

/*
 * This injects a single character into the itemlist widget.
 */
#define injectCDKItemlist(obj,input) injectCDKObject(obj,input,Int)

/*
 * These functions set specific elements of the itemlist widget.
 */
void setCDKItemlist (
		CDKITEMLIST *	/* itemlist */,
		CDK_CSTRING2	/* list */,
		int		/* count */,
		int		/* current */,
		boolean		/* Box */);

/*
 * This function sets the values of the item list widget.
 */
void setCDKItemlistValues (
		CDKITEMLIST *	/* itemlist */,
		CDK_CSTRING2	/* list */,
		int		/* count */,
		int		/* defaultItem */);

chtype **getCDKItemlistValues (
		CDKITEMLIST *	/* itemlist */,
		int *		/* size */);

/*
 * This sets the default item in the list.
 */
void setCDKItemlistDefaultItem (
		CDKITEMLIST *	/* itemlist */,
		int		/* defaultItem */);

int getCDKItemlistDefaultItem (
		CDKITEMLIST *	/* itemlist */);

/*
 * This returns an index to the current item in the list.
 */
void setCDKItemlistCurrentItem (
		CDKITEMLIST *	/* itemlist */,
		int		/* currentItem */);

int getCDKItemlistCurrentItem (
		CDKITEMLIST *	/* itemlist */);

/*
 * This sets the box attribute of the widget.
 */
void setCDKItemlistBox (
		CDKITEMLIST *	/* itemlist */,
		boolean		/* Box */);

boolean getCDKItemlistBox (
		CDKITEMLIST *	/* itemlist */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDKItemlistULChar(w,c)          setULCharOf(w,c)
#define setCDKItemlistURChar(w,c)          setURCharOf(w,c)
#define setCDKItemlistLLChar(w,c)          setLLCharOf(w,c)
#define setCDKItemlistLRChar(w,c)          setLRCharOf(w,c)
#define setCDKItemlistVerticalChar(w,c)    setVTCharOf(w,c)
#define setCDKItemlistHorizontalChar(w,c)  setHZCharOf(w,c)
#define setCDKItemlistBoxAttribute(w,c)    setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKItemlistBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKItemlistBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This draws the itemlist widget.
 */
#define drawCDKItemlist(obj,Box) drawCDKObject(obj,Box)

/*
 * This draws the itemlist field.
 */
void drawCDKItemlistField (
		CDKITEMLIST *	/* itemlist */,
		boolean         /* highlight */);

/*
 * This removes the widget from the screen.
 */
#define eraseCDKItemlist(obj) eraseCDKObject(obj)

/*
 * This moves the widget to the given position.
 */
#define moveCDKItemlist(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This allows the user to interactively move the widget.
 */
#define positionCDKItemlist(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the widget and all the associated memory.
 */
#define destroyCDKItemlist(obj) destroyCDKObject(obj)

/*
 * These functions set the pre/post process functions.
 */
#define setCDKItemlistPreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDKItemlistPostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

#ifdef __cplusplus
}
#endif

#endif /* CDKITEMLIST_H */
#endif /* CDKINCLUDES */
