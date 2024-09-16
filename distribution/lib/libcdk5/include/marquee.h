/*
 * $Id: marquee.h,v 1.20 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKMARQUEE_H
#define CDKMARQUEE_H	1

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
 * Define the CDK marquee widget structure.
 */
struct SMarquee {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
   int		active;
   int		width;
   int		boxWidth;
   int		boxHeight;
   boolean	shadow;
};
typedef struct SMarquee CDKMARQUEE;

/*
 * This creates a new marquee widget pointer.
 */
CDKMARQUEE *newCDKMarquee (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		int		/* width */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This turns the marquee 'on'.
 */
int activateCDKMarquee (
		CDKMARQUEE *	/* marquee */,
		const char *	/* message */,
		int		/* delay */,
		int		/* repeat */,
		boolean		/* Box */);

/*
 * This turns 'off' the marquee.
 */
void deactivateCDKMarquee (
		CDKMARQUEE *	/* marquee */);

/*
 * This draws the marquee on the screen.
 */
#define drawCDKMarquee(obj,Box) drawCDKObject(obj,Box)

/*
 * This removes the widget from the screen.
 */
#define eraseCDKMarquee(obj) eraseCDKObject(obj)

/*
 * This moves the widget to the given location.
 */
#define moveCDKMarquee(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This interactively positions the widget on the screen.
 */
#define positionCDKMarquee(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the marquee widget.
 */
#define destroyCDKMarquee(obj) destroyCDKObject(obj)

/*
 * These set the drawing characters of the widget.
 */
#define setCDKMarqueeULChar(w,c)           setULCharOf(w,c)
#define setCDKMarqueeURChar(w,c)           setURCharOf(w,c)
#define setCDKMarqueeLLChar(w,c)           setLLCharOf(w,c)
#define setCDKMarqueeLRChar(w,c)           setLRCharOf(w,c)
#define setCDKMarqueeVerticalChar(w,c)     setVTCharOf(w,c)
#define setCDKMarqueeHorizontalChar(w,c)   setHZCharOf(w,c)
#define setCDKMarqueeBoxAttribute(w,c)     setBXAttrOf(w,c)

/*
 * This sets the box attribute of the widget.
 */
void setCDKMarqueeBox (
		CDKMARQUEE *	/* marquee */,
		boolean		/* Box */);

boolean getCDKMarqueeBox (
		CDKMARQUEE *	/* marquee */);

/*
 * This sets the background color of the widget.
 */
#define setCDKMarqueeBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKMarqueeBackgroundAttrib(w,c) setBKAttrOf(w,c)

#ifdef __cplusplus
}
#endif

#endif /* CDKMARQUEE_H */
#endif /* CDKINCLUDES */
