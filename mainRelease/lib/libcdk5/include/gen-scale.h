/*
 * $Id: gen-scale.h,v 1.9 2012/03/20 21:59:39 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDK<UPPER>_H
#define CDK<UPPER>_H	1

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
 * Copyright 2004,2012 Thomas E. Dickey
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
 * Define the CDK widget <MIXED> structure.
 */
struct S<MIXED> {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
   WINDOW *	fieldWin;
   WINDOW *	labelWin;
   int		titleAdj;
   chtype *	label;
   int		labelLen;
   int		boxHeight;
   int		boxWidth;
   int		fieldWidth;
   int		fieldEdit;	/* offset from right-margin of field */
   chtype	fieldAttr;
   <CTYPE>	low;
   <CTYPE>	high;
   <CTYPE>	inc;
   <CTYPE>	fastinc;
   <CTYPE>	current;
#if <FLOAT>
   int		digits;
#endif <FLOAT>
   EExitType	exitType;
   boolean	shadow;
};
typedef struct S<MIXED> CDK<UPPER>;

/*
 * This creates a new pointer to a CDK <CTYPE>-<MIXED> widget.
 */
CDK<UPPER> *newCDK<MIXED> (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		const char *	/* title */,
		const char *	/* label */,
		chtype		/* fieldAttr */,
		int		/* fieldWidth */,
		<CTYPE>		/* start */,
		<CTYPE>		/* low */,
		<CTYPE>		/* high */,
		<CTYPE>		/* inc */,
		<CTYPE>		/* fastInc */,
#if <FLOAT>
		int		/* digits */,
#endif <FLOAT>
		boolean		/* Box */,
		boolean		/* shadow */);

/*
 * This activates the widget.
 */
<CTYPE>	activateCDK<MIXED> (
		CDK<UPPER> *	/* widget */,
		chtype *	/* actions */);

/*
 * This injects a single character into the widget.
 */
#define injectCDK<MIXED>(obj,input) injectCDKObject(obj,input,<DTYPE>)

/*
 * This sets various attributes of the widget.
 */
void setCDK<MIXED> (
		CDK<UPPER> *	/* widget */,
		<CTYPE>		/* low */,
		<CTYPE>		/* high */,
		<CTYPE>		/* value */,
		boolean		/* Box */);

/*
 * These set/get the low and high values.
 */
void setCDK<MIXED>LowHigh (
		CDK<UPPER> *	/* widget */,
		<CTYPE>		/* low */,
		<CTYPE>		/* high */);

<CTYPE>	getCDK<MIXED>LowValue (
		CDK<UPPER> *	/* widget */);

<CTYPE>	getCDK<MIXED>HighValue (
		CDK<UPPER> *	/* widget */);

/*
 * These set/get the digits.
 */
#if <FLOAT>
void setCDK<MIXED>Digits (
		CDK<UPPER> *	/* widget */,
		int		/* digits */);

int getCDK<MIXED>Digits (
		CDK<UPPER> *	/* widget */);
#endif <FLOAT>

/*
 * These set/get the current value.
 */
void setCDK<MIXED>Value (
		CDK<UPPER> *	/* widget */,
		<CTYPE>		/* value */);

<CTYPE>	getCDK<MIXED>Value (
		CDK<UPPER> *	/* widget */);

/*
 * This sets the box attribute of the widget.
 */
void setCDK<MIXED>Box (
		CDK<UPPER> *	/* widget */,
		boolean		/* Box */);

boolean getCDK<MIXED>Box (
		CDK<UPPER> *	/* widget */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDK<MIXED>ULChar(w,c)             setULCharOf(w,c)
#define setCDK<MIXED>URChar(w,c)             setURCharOf(w,c)
#define setCDK<MIXED>LLChar(w,c)             setLLCharOf(w,c)
#define setCDK<MIXED>LRChar(w,c)             setLRCharOf(w,c)
#define setCDK<MIXED>VerticalChar(w,c)       setVTCharOf(w,c)
#define setCDK<MIXED>HorizontalChar(w,c)     setHZCharOf(w,c)
#define setCDK<MIXED>BoxAttribute(w,c)       setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDK<MIXED>BackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */ 
#define setCDK<MIXED>BackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This draws the widget on the screen.
 */
#define drawCDK<MIXED>(obj,Box) drawCDKObject(obj,Box)

/*
 * This erases the widget from the screen.
 */
#define eraseCDK<MIXED>(obj) eraseCDKObject(obj)

/*
 * This moves the widget to the given location on the screen.
 */
#define moveCDK<MIXED>(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This allows the user to interactively position the widget on the screen.
 */
#define positionCDK<MIXED>(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the widget and associated memory.
 */
#define destroyCDK<MIXED>(obj) destroyCDKObject(obj)

/*
 * These set the pre/post process callback functions.
 */
#define setCDK<MIXED>PreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDK<MIXED>PostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

#ifdef __cplusplus
}
#endif

#endif /* CDK<UPPER>_H */
#endif /* CDKINCLUDES */
