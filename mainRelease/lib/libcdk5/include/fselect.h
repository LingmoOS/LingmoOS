/*
 * $Id: fselect.h,v 1.26 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKFSELECT_H
#define CDKFSELECT_H	1

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
#include <entry.h>
#include <scroll.h>
#endif

/*
 * Copyright 1999-2006,2012 Thomas E. Dickey
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
 * Define the CDK file selector widget structure.
 */
struct SFileSelector {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
   CDKENTRY *	entryField;
   CDKSCROLL *	scrollField;
   char **	dirContents;
   int		fileCounter;
   char *	pwd;
   char *	pathname;
   int		xpos;
   int		ypos;
   int		boxHeight;
   int		boxWidth;
   chtype	fieldAttribute;
   chtype	fillerCharacter;
   chtype	highlight;
   char *	dirAttribute;
   char *	fileAttribute;
   char *	linkAttribute;
   char *	sockAttribute;
   EExitType	exitType;
   boolean	shadow;
};
typedef struct SFileSelector CDKFSELECT;

/*
 * This creates a new CDK file selector widget.
 */
CDKFSELECT *newCDKFselect (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		int		/* height */,
		int		/* width */,
		const char *	/* title */,
		const char *	/* label */,
		chtype		/* fieldAttribute */,
		chtype		/* fillerChar */,
		chtype		/* highlight */,
		const char *	/* dirAttributes */,
		const char *	/* fileAttributes */,
		const char *	/* linkAttribute */,
		const char *	/* sockAttribute */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This activates the file selector.
 */
char *activateCDKFselect (
		CDKFSELECT *	/* fselect */,
		chtype *	/* actions */);

/*
 * This injects a single character into the widget.
 */
#define injectCDKFselect(obj,input) injectCDKObject(obj,input,String)

/*
 * This sets various attributes of the file selector.
 */
void setCDKFselect (
		CDKFSELECT *	/* fselect */,
		const char *	/* directory */,
		chtype		/* fieldAttrib */,
		chtype		/* filler */,
		chtype		/* highlight */,
		const char *	/* dirAttribute */,
		const char *	/* fileAttribute */,
		const char *	/* linkAttribute */,
		const char *	/* sockAttribute */,
		boolean		/* Box */);

/*
 * This sets the current directory of the file selector.
 */
int setCDKFselectDirectory (
		CDKFSELECT *	/* fselect */,
		const char *	/* directory */);

char *getCDKFselectDirectory (
		CDKFSELECT *	/* fselect */);

/*
 * This sets the filler character of the entry field.
 */
void setCDKFselectFillerChar (
		CDKFSELECT *	/* fselect */,
		chtype		/* fillerCharacter */);

chtype getCDKFselectFillerChar (
		CDKFSELECT *	/* fselect */);

/*
 * This sets the highlight bar of the scrolling list.
 */
void setCDKFselectHighlight (
		CDKFSELECT *	/* fselect */,
		chtype		/* highlight */);

chtype getCDKFselectHighlight (
		CDKFSELECT *	/* fselect */);

/*
 * These functions set the attribute of the directories, links,
 * files, and sockets in the scrolling list portion of the file
 * selector widget.
 */
void setCDKFselectDirAttribute (
		CDKFSELECT *	/* fselect */,
		const char *	/* attribute */);

void setCDKFselectLinkAttribute (
		CDKFSELECT *	/* fselect */,
		const char *	/* attribute */);

void setCDKFselectFileAttribute (
		CDKFSELECT *	/* fselect */,
		const char *	/* attribute */);

void setCDKFselectSocketAttribute (
		CDKFSELECT *	/* fselect */,
		const char *	/* attribute */);

/*
 * These functions return the attribute of the directories, links,
 * files, and sockets in the scrolling list portion of the file
 * selector widget.
 */
char *getCDKFselectDirAttribute (
		CDKFSELECT *	/* fselect */);

char *getCDKFselectLinkAttribute (
		CDKFSELECT *	/* fselect */);

char *getCDKFselectFileAttribute (
		CDKFSELECT *	/* fselect */);

char *getCDKFselectSocketAttribute (
		CDKFSELECT *	/* fselect */);

/*
 * This sets the box attribute of the widget.
 */
void setCDKFselectBox (
		CDKFSELECT *	/* fselect */,
		boolean		/* Box */);

boolean getCDKFselectBox (
		CDKFSELECT *	/* fselect */);

/*
 * This sets the contents of the file selector.
 */
int setCDKFselectDirContents (
		CDKFSELECT *	/* fselect */);

char **getCDKFselectDirContents (
		CDKFSELECT *	/* fselect */,
		int *		/* count */);

/*
 * This sets the contents of the file selector.
 */
void setCDKFselectContents (
		CDKFSELECT *	/* fselect */,
		CDK_CSTRING2	/* list */,
		int		/* listSize */);

char **getCDKFselectContents (
		CDKFSELECT *	/* fselect */,
		int *		/* size */);

/*
 * Get/set the current position in the scroll-widget.
 */
int getCDKFselectCurrentItem(
		CDKFSELECT *	/* widget */);

void setCDKFselectCurrentItem(
		CDKFSELECT *	/* widget */,
		int		/* item */);

/*
 * These functions set the drawing characters of the widget.
 */
#define setCDKFselectULChar(w,c)           setULCharOf(w,c)
#define setCDKFselectURChar(w,c)           setURCharOf(w,c)
#define setCDKFselectLLChar(w,c)           setLLCharOf(w,c)
#define setCDKFselectLRChar(w,c)           setLRCharOf(w,c)
#define setCDKFselectVerticalChar(w,c)     setVTCharOf(w,c)
#define setCDKFselectHorizontalChar(w,c)   setHZCharOf(w,c)
#define setCDKFselectBoxAttribute(w,c)     setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKFselectBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKFselectBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This draws the widget.
 */
#define drawCDKFselect(obj,Box) drawCDKObject(obj,Box)

/*
 * This erases the widget.
 */
#define eraseCDKFselect(obj) eraseCDKObject(obj)

/*
 * This moves the widget to the given location.
 */
#define moveCDKFselect(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This allows the user to interactively position the widget.
 */
#define positionCDKFselect(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the widget and all the associated memory.
 */
#define destroyCDKFselect(obj) destroyCDKObject(obj)

/*
 * This is a callback which allows you to delete files from within the
 * file selector. It is NOT an active default: it MUST be set by the
 * user.
 */
void deleteFileCB (
		EObjectType	/* objectType */,
		void *		/* object */,
		void *		/* clientData */);

/*
 * This function sets the pre-process function.
 */
void setCDKFselectPreProcess (
		CDKFSELECT *	/* fselect */,
		PROCESSFN	/* callback */,
		void *		/* data */);

/*
 * This function sets the post-process function.
 */
void setCDKFselectPostProcess (
		CDKFSELECT *	/* fselect */,
		PROCESSFN	/* callback */,
		void *		/* data */);

#ifdef __cplusplus
}
#endif

#endif /* CDKFSELECT_H */
#endif /* CDKINCLUDES */
