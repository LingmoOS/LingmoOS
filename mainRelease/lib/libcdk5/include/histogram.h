/*
 * $Id: histogram.h,v 1.20 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKHISTOGRAM_H
#define CDKHISTOGRAM_H	1

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
 * Declare the histogram structure.
 */
struct SHistogram {
   CDKOBJS			obj;
   WINDOW *			parent;
   WINDOW *			win;
   WINDOW *			shadowWin;
   int				titleAdj;
   char *			curString;
   char *			lowString;
   char *			highString;
   chtype			filler;
   float			percent;
   int				fieldHeight;
   int				fieldWidth;
   int				barSize;
   int				orient;
   int				statsPos;
   chtype			statsAttr;
   EHistogramDisplayType	viewType;
   int				high;
   int				low;
   int				value;
   int				lowx;
   int				lowy;
   int				curx;
   int				cury;
   int				highx;
   int				highy;
   int				boxWidth;
   int				boxHeight;
   boolean			shadow;
};
typedef struct SHistogram CDKHISTOGRAM;

/*
 * This returns a new pointer to a histogram pointer.
 */
CDKHISTOGRAM *newCDKHistogram (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		int		/* height */,
		int		/* width */,
		int		/* orient */,
		const char *	/* title */,
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This was added to make the build simpler. All this will
 * do is call drawCDKHistogram.
 */
void activateCDKHistogram (
		CDKHISTOGRAM *	/* histogram */,
		chtype *	/* actions */);

/*
 * These set specific attributes of the histogram.
 */
void setCDKHistogram (
		CDKHISTOGRAM *	/* histogram */,
		EHistogramDisplayType	/* viewtype */,
		int		/* statsPos */,
		chtype		/* statsAttr */,
		int		/* low */,
		int		/* high */,
		int		/* value */,
		chtype		/* filler */,
		boolean		/* Box */);

/*
 * This sets the low/high/current value of the histogram.
 */
void setCDKHistogramValue (
		CDKHISTOGRAM *	/* histogram */,
		int		/* low */,
		int		/* high */,
		int		/* value */);

int getCDKHistogramValue (
		CDKHISTOGRAM *	/* histogram */);

int getCDKHistogramLowValue (
		CDKHISTOGRAM *	/* histogram */);

int getCDKHistogramHighValue (
		CDKHISTOGRAM *	/* histogram */);

/*
 * This sets the view type of the histogram.
 */
void setCDKHistogramDisplayType (
		CDKHISTOGRAM *	/* histogram */,
		EHistogramDisplayType	/* viewtype */);

EHistogramDisplayType getCDKHistogramViewType (
		CDKHISTOGRAM *	/* histogram */);

/*
 * This returns the filler character used to draw the histogram.
 */
void setCDKHistogramFillerChar (
		CDKHISTOGRAM *	/* histogram */,
		chtype		/* character */);

chtype getCDKHistogramFillerChar (
		CDKHISTOGRAM *	/* histogram */);

/*
 * This states where the statistics value is to be located on the histogram.
 */
void setCDKHistogramStatsPos (
		CDKHISTOGRAM *	/* histogram */,
		int		/* statsPos */);

int getCDKHistogramStatsPos (
		CDKHISTOGRAM *	/* histogram */);

/*
 * This sets the attribute of the statistics.
 */
void setCDKHistogramStatsAttr (
		CDKHISTOGRAM *	/* histogram */,
		chtype		/* statsAttr */);

chtype getCDKHistogramStatsAttr (
		CDKHISTOGRAM *	/* histogram */);

/*
 * This sets the box attribute of the widget.
 */
void setCDKHistogramBox (
		CDKHISTOGRAM *	/* histogram */,
		boolean		/* Box */);

boolean getCDKHistogramBox (
		CDKHISTOGRAM *	/* histogram */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDKHistogramULChar(w,c)         setULCharOf(w,c)
#define setCDKHistogramURChar(w,c)         setURCharOf(w,c)
#define setCDKHistogramLLChar(w,c)         setLLCharOf(w,c)
#define setCDKHistogramLRChar(w,c)         setLRCharOf(w,c)
#define setCDKHistogramVerticalChar(w,c)   setVTCharOf(w,c)
#define setCDKHistogramHorizontalChar(w,c) setHZCharOf(w,c)
#define setCDKHistogramBoxAttribute(w,c)   setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKHistogramBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKHistogramBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This draws the widget on the screen.
 */
#define drawCDKHistogram(obj,Box) drawCDKObject(obj,Box)

/*
 * This removes the widget from the screen.
 */
#define eraseCDKHistogram(obj) eraseCDKObject(obj)

/*
 * This moves the widget to the given location.
 */
#define moveCDKHistogram(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This allows the user to interactively position the widget on the screen.
 */
#define positionCDKHistogram(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the histogram and all related pointers.
 */
#define destroyCDKHistogram(obj) destroyCDKObject(obj)

#ifdef __cplusplus
}
#endif

#endif /* CDKHISTOGRAM_H */
#endif /* CDKINCLUDES */
