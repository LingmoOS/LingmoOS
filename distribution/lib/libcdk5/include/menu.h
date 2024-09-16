/*
 * $Id: menu.h,v 1.23 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKMENU_H
#define CDKMENU_H	1

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
 * Copyright 1999-2005,2012 Thomas E. Dickey
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
 * Define menu specific values.
 */
#define MAX_MENU_ITEMS	30
#define MAX_SUB_ITEMS	98

/*
 * Define the CDK menu widget structure.
 */
struct SMenu {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	pullWin[MAX_MENU_ITEMS];
   WINDOW *	titleWin[MAX_MENU_ITEMS];
   chtype *	title[MAX_MENU_ITEMS];
   int		titleLen[MAX_MENU_ITEMS];
   chtype *	sublist[MAX_MENU_ITEMS][MAX_SUB_ITEMS];
   int		sublistLen[MAX_MENU_ITEMS][MAX_SUB_ITEMS];
   int		subsize[MAX_MENU_ITEMS];
   int		menuPos;
   int		menuItems;
   chtype	titleAttr;
   chtype	subtitleAttr;
   int		currentTitle;
   int		currentSubtitle;
   int		lastTitle;
   int		lastSubtitle;
   EExitType	exitType;
   int		lastSelection;
};
typedef struct SMenu CDKMENU;

/*
 * This creates a new CDK menu widget pointer.
 */
CDKMENU *newCDKMenu (
		CDKSCREEN *	/* cdkscreen */,
		const char *	/* menulist */ [MAX_MENU_ITEMS][MAX_SUB_ITEMS],
		int 		/* menuitems */,
		int *		/* subsize */,
		int *		/* menuloc */,
		int		/* menuPos */,
		chtype 		/* titleattr */,
		chtype 		/* subtitleattr */);

/*
 * This activates the menu.
 */
int activateCDKMenu (
		CDKMENU *	/* menu */,
		chtype *	/* actions */);

/*
 * This injects a single character into the menu widget.
 */
#define injectCDKMenu(obj,input) injectCDKObject(obj,input,Int)

/*
 * These set specific attributes of the menu.
 */
void setCDKMenu (
		CDKMENU *	/* menu */,
		int 		/* menuItem */,
		int 		/* subMenuItem */,
		chtype 		/* titleHighlight */,
		chtype 		/* subTitleHighlight */);

/*
 * This returns the current item the menu is on.
 */
void setCDKMenuCurrentItem (
		CDKMENU *	/* menu */,
		int 		/* menuItem */,
		int 		/* subMenuItem */);

void getCDKMenuCurrentItem (
		CDKMENU *	/* menu */,
		int *		/* menuItem */,
		int *		/* subMenuItem */);

/*
 * This sets the highlight of the title.
 */
void setCDKMenuTitleHighlight (
		CDKMENU *	/* menu */,
		chtype 		/* highlight */);

chtype getCDKMenuTitleHighlight (
		CDKMENU *	/* menu */);

/*
 * This sets the sub-menu title highlight.
 */
void setCDKMenuSubTitleHighlight (
		CDKMENU *	/* menu */,
		chtype 		/* highlight */);

chtype getCDKMenuSubTitleHighlight (
		CDKMENU *	/* menu */);

/*
 * This draws the menu on the screen.
 */
#define drawCDKMenu(obj,box) drawCDKObject(obj,box)

void drawCDKMenuSubwin (
		CDKMENU *	/* menu */);

/*
 * This erases the complere menu widget from the screen.
 */
#define eraseCDKMenu(obj) eraseCDKObject(obj)

void eraseCDKMenuSubwin (
		CDKMENU *	/* menu */);

/*
 * This sets the background color of the widget.
 */
#define setCDKMenuBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKMenuBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This destroys the menu widget.
 */
#define destroyCDKMenu(obj) destroyCDKObject(obj)

/*
 * These set the pre/post process callback functions.
 */
#define setCDKMenuPreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDKMenuPostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

#ifdef __cplusplus
}
#endif

#endif /* CDKMENU_H */
#endif /* CDKINCLUDES */
