/*
 * $Id: matrix.h,v 1.31 2012/03/21 21:15:30 tom Exp $
 */

#ifndef CDKINCLUDES
#ifndef CDKMATRIX_H
#define CDKMATRIX_H	1

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
 * Copyright 1999-2008,2012 Thomas E. Dickey
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
 * Declare some matrix definitions.
 */
#define MAX_MATRIX_ROWS 1000
#define MAX_MATRIX_COLS 1000

/*
 * Define the CDK matrix widget structure.
 */
typedef struct SMatrix CDKMATRIX;
typedef void (*MATRIXCB) (CDKMATRIX *matrix, chtype input);

#define CELL_LIMIT                  MAX_MATRIX_ROWS][MAX_MATRIX_COLS

#define NEW_CDKMATRIX 1

#if NEW_CDKMATRIX
#define CELL_INDEX(matrix, row,col) (((row) * ((matrix)->cols + 1)) + (col))
#else
#define CELL_INDEX(matrix, row,col) (row)][(col)
#endif

#define MATRIX_CELL(matrix,row,col) ((matrix)->cell[CELL_INDEX(matrix, row, col)])
#define MATRIX_INFO(matrix,row,col) ((matrix)->info[CELL_INDEX(matrix, row, col)])

struct SMatrix {
   CDKOBJS	obj;
   WINDOW *	parent;
   WINDOW *	win;
   WINDOW *	shadowWin;
#if NEW_CDKMATRIX
   WINDOW **	cell;
   char **	info;
#else
   WINDOW *	cell[CELL_LIMIT];
   char *	info[CELL_LIMIT];
#endif
   int		titleAdj;
   int		rows;
   int		cols;
   int		vrows;
   int		vcols;
   int *	colwidths;
   int *	colvalues;
   chtype **	coltitle;
   int *	coltitleLen;
   int *	coltitlePos;
   int		maxct;
   chtype **	rowtitle;
   int *	rowtitleLen;
   int *	rowtitlePos;
   int		maxrt;
   int		boxHeight;
   int		boxWidth;
   int		rowSpace;
   int		colSpace;
   int		row;
   int		col;
   int		crow;		/* current row */
   int		ccol;		/* current column */
   int		trow;		/* topmost row shown in screen */
   int		lcol;		/* leftmost column shown in screen */
   int		oldcrow;
   int		oldccol;
   int		oldvrow;
   int		oldvcol;
   EExitType	exitType;
   boolean	boxCell;
   boolean	shadow;
   chtype	highlight;
   int		dominant;
   chtype	filler;
   MATRIXCB	callbackfn;
};

/*
 * This creates a new pointer to a matrix widget.
 */
CDKMATRIX *newCDKMatrix (
		CDKSCREEN *	/* cdkscreen */,
		int		/* xpos */,
		int		/* ypos */,
		int		/* rows */,
		int		/* cols */,
		int		/* vrows */,
		int		/* vcols */,
		const char *	/* title */,
		CDK_CSTRING2	/* rowtitles */,
		CDK_CSTRING2	/* coltitles */,
		int *		/* colwidths */,
		int *		/* coltypes */,
		int		/* rowspace */,
		int		/* colspace */,
		chtype		/* filler */,
		int		/* dominantAttrib */,
		boolean		/* boxMatrix */,
		boolean		/* boxCell */,
		boolean		/* shadow */);

/*
 * This activates the matrix.
 */
int activateCDKMatrix (
		CDKMATRIX *	/* matrix */,
		chtype *	/* actions */);

/*
 * This injects a single character into the matrix widget.
 */
#define injectCDKMatrix(obj,input) injectCDKObject(obj,input,Int)

/*
 * This sets the contents of the matrix widget from a fixed-size 2d array.
 * The predefined array limits are very large.
 * Use setCDKMatrixCells() instead.
 */
#define setCDKMatrix(matrix, info, rows, subSize) \
	setCDKMatrixCells(matrix, &info[0][0], rows, MAX_MATRIX_COLS, subSize)

/*
 * This sets the contents of the matrix widget from an array defined by the
 * caller.  It may be any size.  For compatibility with setCDKMatrix(), the
 * info[][] array's subscripts start at 1.
 */
void setCDKMatrixCells (
		CDKMATRIX *	/* matrix */,
		CDK_CSTRING2	/* info */,
		int		/* rows */,
		int		/* cols */,
		int *		/* subSize */);

/*
 * This sets the value of a given cell.
 */
int setCDKMatrixCell (
		CDKMATRIX *	/* matrix */,
		int		/* row */,
		int		/* col */,
		const char *	/* value */);

char *getCDKMatrixCell (
		CDKMATRIX *	/* matrix */,
		int		/* row */,
		int		/* col */);

/*
 * This returns the row/col of the matrix.
 */
int getCDKMatrixCol (
		CDKMATRIX *	/* matrix */);

int getCDKMatrixRow (
		CDKMATRIX *	/* matrix */);

/*
 * These set the drawing characters of the widget.
 */
#define setCDKMatrixULChar(w,c)            setULCharOf(w,c)
#define setCDKMatrixURChar(w,c)            setURCharOf(w,c)
#define setCDKMatrixLLChar(w,c)            setLLCharOf(w,c)
#define setCDKMatrixLRChar(w,c)            setLRCharOf(w,c)
#define setCDKMatrixVerticalChar(w,c)      setVTCharOf(w,c)
#define setCDKMatrixHorizontalChar(w,c)    setHZCharOf(w,c)
#define setCDKMatrixBoxAttribute(w,c)      setBXAttrOf(w,c)

/*
 * This sets the background color of the widget.
 */
#define setCDKMatrixBackgroundColor(w,c) setCDKObjectBackgroundColor(ObjOf(w),c)

/*
 * This sets the background attribute of the widget.
 */
#define setCDKMatrixBackgroundAttrib(w,c) setBKAttrOf(w,c)

/*
 * This draws the matrix on the screen.
 */
#define drawCDKMatrix(obj,Box) drawCDKObject(obj,Box)

/*
 * This removes the matrix from the screen.
 */
#define eraseCDKMatrix(obj) eraseCDKObject(obj)

/*
 * This cleans out all the cells from the matrix.
 */
void cleanCDKMatrix (
		CDKMATRIX *	/* matrix */);

/*
 * This cleans one cell in the matrix.
 */
void cleanCDKMatrixCell (
		CDKMATRIX *	/* matrix */,
		int		/* row */,
		int		/* col */);

/*
 * This sets the main callback in the matrix.
 */
void setCDKMatrixCB (
		CDKMATRIX *	/* matrix */,
		MATRIXCB	/* callback */);

/*
 * This moves the matrix to the given cell.
 */
int moveToCDKMatrixCell (
		CDKMATRIX *	/* matrix */,
		int		/* newrow */,
		int		/* newcol */);

/*
 * This sets the box attribute of the matrix widget.
 */
void setCDKMatrixBox (
		CDKMATRIX *	/* matrix */,
		boolean		/* Box */);

boolean getCDKMatrixBox (
		CDKMATRIX *	/* matrix */);

/*
 * This moves the matrix on the screen to the given location.
 */
#define moveCDKMatrix(obj,xpos,ypos,relative,refresh) moveCDKObject(obj,xpos,ypos,relative,refresh)

/*
 * This allows the user to interactively position the matrix.
 */
#define positionCDKMatrix(widget) positionCDKObject(ObjOf(widget),widget->win)

/*
 * This destroys the matrix widget and associated memory.
 */
#define destroyCDKMatrix(obj) destroyCDKObject(obj)

/*
 * This jumps to the given matrix cell. You can pass in
 * -1 for both the row/col if you want to interactively
 * pick the cell.
 */
int jumpToCell (
		CDKMATRIX *	/* matrix */,
		int		/* row */,
		int		/* col */);

/*
 * These set the pre/post process callback functions.
 */
#define setCDKMatrixPreProcess(w,f,d)  setCDKObjectPreProcess(ObjOf(w),f,d)
#define setCDKMatrixPostProcess(w,f,d) setCDKObjectPostProcess(ObjOf(w),f,d)

#ifdef __cplusplus
}
#endif

#endif /* CDKMATRIX_H */
#endif /* CDKINCLUDES */
