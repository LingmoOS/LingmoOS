/*
 * $Id: radio.h,v 1.31 2013/09/01 18:06:41 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKRADIO_H
#define CDKRADIO_H	1

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
 * Changes 1999-2012,2013 copyright Thomas E. Dickey
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
 * Define the CDK radio list widget structure.
 */
struct SRadio {
   /* This field must stay on top */
   SCROLLER_FIELDS;

   chtype	choiceChar;
   chtype	leftBoxChar;
   chtype	rightBoxChar;
   int		widestItem;
   int		selectedItem;
   int		defItem;
};
typedef struct SRadio CDKRADIO;

/*
 * This creates a new radio widget pointer.
 */
CDKRADIO *newCDKRadio (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		int		/* spos */,
		int		/* height */,
		int		/* width */,
		const char *	/* title */,
		CDK_CSTRING2	/* mesg */,
		int		/* items */,
		chtype		/* choiceChar */,
		int		/* defItem */,
		chtype		/* highlight */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This activates the widget.
 */
int activateCDKRadio (
		CDKRADIO *	/* radio */,
		chtype *	/* actions */);

/*
 * This injects a single character into the widget.
 */
#define injectCDKRadio(obj,input) injectCDKObject(obj,input,Int)

/*
 * These set various attributes of the widget.
 */
void setCDKRadio (
		CDKRADIO *	/* radio */,
		chtype		/* highlight */,
		chtype		/* character */,
		boolean		/* Box */);

/*
 * This sets the contents of the radio list.
 */
void setCDKRadioItems (
		CDKRADIO *	/* radio */,
		CDK_CSTRING2	/* list */,
		int		/* listSize */);

int getCDKRadioItems (
		CDKRADIO *	/* radio */,
		char **		/* list */);

/*
 * This sets the highlight bar attribute.
 */
void setCDKRadioHighlight (
		CDKRADIO *	/* radio */,
		chtype		/* highlight */);

chtype getCDKRadioHighlight (
		CDKRADIO *	/* radio */);

/*
 * This sets the 'selected' character.
 */
void setCDKRadioChoiceCharacter (
		CDKRADIO *	/* radio */,
		chtype		/* character */);

chtype getCDKRadioChoiceCharacter (
		CDKRADIO *	/* radio */);

/*
 * This sets the character to draw on the left/right side of
 * the choice box.
 */
void setCDKRadioLeftBrace (
		CDKRADIO *	/* radio */,
		chtype		/* character */);

chtype getCDKRadioLeftBrace (
		CDKRADIO *	/* radio */);

void setCDKRadioRightBrace (
		CDKRADIO *	/* radio */,
		chtype		/* character */);

chtype getCDKRadioRightBrace (
		CDKRADIO *	/* radio */);

/*
 * This sets the box attribute of the widget.
 */
void setCDKRadioBox (
		CDKRADIO *	/* radio */,
		boolean		/* Box */);

boolean getCDKRadioBox (
		CDKRADIO *	/* radio */);

/*
 * This sets the current high lighted item of the widget
 */
void setCDKRadioCurrentItem (
		CDKRADIO *	/* radio */,
 		int 		/* current item */);

int getCDKRadioCurrentItem (
		CDKRADIO *	/* radio */);

/*
 * This sets the current selected item of the widget
 */
void setCDKRadioSelectedItem (
		CDKRADIO *	/* radio */,
 		int 		/* current item */);

int getCDKRadioSelectedItem (
		CDKRADIO *	/* radio */);


/*
 * These set the drawing characters of the widget.
 */
#define setCDKRadioULChar(w,c)             setULCharOf(w,c)
#define setCDKRadioURChar(w,c)             setURCharOf(w,c)
#define setCDKRadioLLChar(w,c)             setLLCharOf(w,c)
#define setCDKRadioLRChar(w,c)             setLRCharOf(w,c)
#define setCDKRadioVerticalChar(w,c)       setVTCharOf(w,c)
#define setCDKRadioHorizontalChar(w,c)     setHZCharOf(w,c)
#define setCDKRadioBoxAttribute(w,c)       setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKRadioBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */
#define setCDKRadioBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This draws the widget on the screen.
 */
#define drawCDKRadio(obj,Box) drawCDKObject(obj,Box)

/*
 * This erases the widget from the screen.
 */
#define eraseCDKRadio(obj) eraseCDKObject(obj)

/*
 * This moves the widget to the given screen location.
 */
#define moveCDKRadio(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This interactively moves the widget to a new location on the screen.
 */
#define positionCDKRadio(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys a widget pointer.
 */
#define destroyCDKRadio(obj) destroyCDKObject(obj)

/*
 * These set the pre/post process callback functions.
 */
#define setCDKRadioPreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDKRadioPostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

#ifdef __cplusplus
}
#endif

#endif /* CDKRADIO_H */
#endif /* CDKINCLUDES */
