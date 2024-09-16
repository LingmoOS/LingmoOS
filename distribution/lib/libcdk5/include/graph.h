/*
 * $Id: graph.h,v 1.23 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKGRAPH_H
#define CDKGRAPH_H	1

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
 * Copyright 2000-2004,2012 Thomas E. Dickey
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
 * Define the CDK graph structure.
 */
struct SGraph {
   CDKOBJS		obj;
   WINDOW *		parent;
   WINDOW *		win;
   WINDOW *		shadowWin;
   int			titleAdj;
   chtype *		graphChar;
   boolean		shadow;
   int			boxHeight;
   int			boxWidth;
   chtype *		xtitle;
   int			xtitlePos;
   int			xtitleLen;
   int *		values;
   int			count;
   int			minx;
   int			maxx;
   int			xscale;
   int			txpos;
   chtype *		ytitle;
   int			ytitlePos;
   int			ytitleLen;
   int			yscale;
   int			typos;
   EGraphDisplayType	displayType;
};
typedef struct SGraph CDKGRAPH;

/*
 * This creates a new CDK graph pointer.
 */
CDKGRAPH *newCDKGraph (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		int		/* height */,
		int		/* width */,
		const char *	/* title */,
		const char *	/* xtitle */,
		const char *	/* ytitle */);

/*
 * This was added to make the builder simpler. All this will
 * do is call drawCDKGraph.
 */
void activateCDKGraph (
		CDKGRAPH *	/* graph */,
		chtype *	/* actions */);

/*
 * This sets multiple attributes of the widget.
 */
int setCDKGraph (
		CDKGRAPH *	/* graph */,
		int *		/* values */,
		int		/* count */,
		const char *	/* graphChar */,
		boolean		/* startAtZero */,
		EGraphDisplayType  /* displayType */);

/*
 * This sets the values of the graph widget.
 */
int setCDKGraphValues (
		CDKGRAPH *	/* graph */,
		int *		/* values */,
		int		/* count */,
		boolean		/* startAtZero */);

int setCDKGraphValue (
		CDKGRAPH *	/* graph */,
		int		/* index */,
		int		/* value */,
		boolean		/* startAtZero */);

int *getCDKGraphValues (
		CDKGRAPH *	/* graph */,
		int *		/* size */);

int getCDKGraphValue (
		CDKGRAPH *	/* graph */,
		int		/* index */);

/*
 * This sets the characters of the graph widget.
 */
int setCDKGraphCharacters (
		CDKGRAPH *	/* graph */,
		const char *	/* characters */);

int setCDKGraphCharacter (
		CDKGRAPH *	/* graph */,
		int		/* index */,
		const char *	/* character */);

chtype *getCDKGraphCharacters (
		CDKGRAPH *	/* graph */);

chtype getCDKGraphCharacter (
		CDKGRAPH *	/* graph */,
		int		/* index */);

/*
 * This sets the display type of the graph.
 */
void setCDKGraphDisplayType (
		CDKGRAPH *	/* graph */,
		EGraphDisplayType  /* type */);

EGraphDisplayType getCDKGraphDisplayType (
		CDKGRAPH *	/* graph */);

/*
 * This sets the box attribute of the widget.
 */
void setCDKGraphBox (
		CDKGRAPH *	/* graph */,
		boolean		/* Box */);

boolean getCDKGraphBox (
		CDKGRAPH *	/* graph */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDKGraphULChar(w,c)             setULCharOf(w,c)
#define setCDKGraphURChar(w,c)             setURCharOf(w,c)
#define setCDKGraphLLChar(w,c)             setLLCharOf(w,c)
#define setCDKGraphLRChar(w,c)             setLRCharOf(w,c)
#define setCDKGraphVerticalChar(w,c)       setVTCharOf(w,c)
#define setCDKGraphHorizontalChar(w,c)     setHZCharOf(w,c)
#define setCDKGraphBoxAttribute(w,c)       setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKGraphBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDKGraphBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This draws the graph on the screen.
 */
#define drawCDKGraph(obj,Box) drawCDKObject(obj,Box)

/*
 * This removes the graph from the screen.
 */
#define eraseCDKGraph(obj) eraseCDKObject(obj)

/*
 * This moves the widget to the given location.
 */
#define moveCDKGraph(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This allows the user to interactively move the widget.
 */
#define positionCDKGraph(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the graph and all the associated memory.
 */
#define destroyCDKGraph(obj) destroyCDKObject(obj)

#ifdef __cplusplus
}
#endif

#endif /* CDKGRAPH_H */
#endif /* CDKINCLUDES */
