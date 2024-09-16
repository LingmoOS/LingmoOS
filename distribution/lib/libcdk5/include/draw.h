/*
 * $Id: draw.h,v 1.11 2004/10/05 00:05:32 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKDRAW_H
#define CDKDRAW_H	1

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
 * Copyright 1999-2003,2004 Thomas E. Dickey
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
 * This boxes a window.
 */
void boxWindow (
		WINDOW *	/* window */,
		chtype		/* attr */);

/*
 * This draws a single line with the character supplied by 'character'
 */
void drawLine (
		WINDOW *	/* window */,
		int		/* startx */,
		int		/* starty */,
		int		/* endx */,
		int		/* endy */,
		chtype		/* character */);

/*
 * This writes a blank string segment on the given window.
 */
void writeBlanks (
		WINDOW *	/* window */,
		int		/* xpos */,
		int		/* ypos */,
		int		/* align */,
		int		/* start */,
		int		/* end */);

/*
 * This writes a string segment on the given window.
 */
void writeChar (
		WINDOW *	/* window */,
		int		/* xpos */,
		int		/* ypos */,
		char *		/* string */,
		int		/* align */,
		int		/* start */,
		int		/* end */);

/*
 * This writes a string segment on the given window.
 */
void writeCharAttrib (
		WINDOW *	/* window */,
		int		/* xpos */,
		int		/* ypos */,
		char *		/* string */,
		chtype		/* attr */,
		int		/* align */,
		int		/* start */,
		int		/* end */);

/*
 * This writes a string segment on the given window.
 */
void writeChtype (
		WINDOW *	/* window */,
		int		/* xpos */,
		int		/* ypos */,
		chtype *	/* string */,
		int		/* align */,
		int		/* start */,
		int		/* end */);

/*
 * This writes a string segment on the given window.
 */
void writeChtypeAttrib (
		WINDOW *	/* window */,
		int		/* xpos */,
		int		/* ypos */,
		chtype *	/* string */,
		chtype		/* attr */,
		int		/* align */,
		int		/* start */,
		int		/* end */);

/*
 * This boxes a window using the given characters.
 */
void attrbox (
		WINDOW *	/* window */,
		chtype		/* tlc */,
		chtype		/* trc */,
		chtype		/* blc */,
		chtype		/* brc */,
		chtype		/* hor */,
		chtype		/* vert */,
		chtype		/* type */);

/*
 * This boxes a window using the object's characters.
 */
void drawObjBox (
   		WINDOW *	/* win */,
		CDKOBJS *	/* object */);

/*
 * This draws the shadow window of the widget.
 */
void drawShadow (
		WINDOW *	/* window */);

#ifdef __cplusplus
}
#endif

#endif /* CDKDRAW_H */
#endif /* CDKINCLUDES */
