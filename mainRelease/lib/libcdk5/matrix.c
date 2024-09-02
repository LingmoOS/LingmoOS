#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 01:09:14 $
 * $Revision: 1.200 $
 */

/*
 * Declare file local prototypes.
 */
static void highlightCDKMatrixCell (CDKMATRIX *matrix);
static void CDKMatrixCallBack (CDKMATRIX *matrix, chtype input);
static void drawCDKMatrixCell (CDKMATRIX *matrix,
			       int srow, int scol,
			       int vrow, int vcol,
			       boolean Box);
static void drawCurCDKMatrixCell (CDKMATRIX *matrix);
static void drawEachCDKMatrixCell (CDKMATRIX *matrix);
static void drawEachColTitle (CDKMATRIX *matrix);
static void drawEachRowTitle (CDKMATRIX *matrix);
static void drawOldCDKMatrixCell (CDKMATRIX *matrix);
static void redrawTitles (CDKMATRIX *matrix, int row, int col);

#define emptyString(s) ((s) == 0 || *(s) == '\0')

#define CurMatrixCell(matrix) \
	    MATRIX_CELL (matrix, matrix->crow, matrix->ccol)

#define CurMatrixInfo(matrix) \
	    MATRIX_INFO (matrix, \
	    matrix->trow + matrix->crow - 1, \
	    matrix->lcol + matrix->ccol - 1)

DeclareCDKObjects (MATRIX, Matrix, setCdk, Int);

#define WHOLE_BOX ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER

#define TOP_C_BOX ACS_ULCORNER, ACS_URCORNER, ACS_LTEE,     ACS_RTEE
#define MID_C_BOX ACS_LTEE,     ACS_RTEE,     ACS_LTEE,     ACS_RTEE
#define BOT_C_BOX ACS_LTEE,     ACS_RTEE,     ACS_LLCORNER, ACS_LRCORNER

#define LFT_R_BOX ACS_ULCORNER, ACS_TTEE,     ACS_LLCORNER, ACS_BTEE
#define MID_R_BOX ACS_TTEE,     ACS_TTEE,     ACS_BTEE,     ACS_BTEE
#define RGT_R_BOX ACS_TTEE,     ACS_URCORNER, ACS_BTEE,     ACS_LRCORNER

#define LFT_T_BOX ACS_ULCORNER, ACS_TTEE,     ACS_LTEE,     ACS_PLUS
#define MID_T_BOX ACS_TTEE,     ACS_TTEE,     ACS_PLUS,     ACS_PLUS
#define RGT_T_BOX ACS_TTEE,     ACS_URCORNER, ACS_PLUS,     ACS_RTEE

#define LFT_M_BOX ACS_LTEE,     ACS_PLUS,     ACS_LTEE,     ACS_PLUS
#define MID_M_BOX ACS_PLUS,     ACS_PLUS,     ACS_PLUS,     ACS_PLUS
#define RGT_M_BOX ACS_PLUS,     ACS_RTEE,     ACS_PLUS,     ACS_RTEE

#define LFT_B_BOX ACS_LTEE,     ACS_PLUS,     ACS_LLCORNER, ACS_BTEE
#define MID_B_BOX ACS_PLUS,     ACS_PLUS,     ACS_BTEE,     ACS_BTEE
#define RGT_B_BOX ACS_PLUS,     ACS_RTEE,     ACS_BTEE,     ACS_LRCORNER

#define MyBox(cell,what,attr) attrbox(cell, what, ACS_HLINE, ACS_VLINE, attr)

/*
 * This function creates the matrix widget.
 */
CDKMATRIX *newCDKMatrix (CDKSCREEN *cdkscreen,
			 int xplace,
			 int yplace,
			 int rows,
			 int cols,
			 int vrows,
			 int vcols,
			 const char *title,
			 CDK_CSTRING2 rowtitles,
			 CDK_CSTRING2 coltitles,
			 int *colwidths,
			 int *colvalues,
			 int rspace,
			 int cspace,
			 chtype filler,
			 int dominant,
			 boolean Box,
			 boolean boxCell,
			 boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKMATRIX *matrix    = 0;
   int parentWidth      = getmaxx (cdkscreen->window);
   int parentHeight     = getmaxy (cdkscreen->window);
   int boxHeight        = 0;
   int boxWidth         = 0;
   int xpos             = xplace;
   int ypos             = yplace;
   int maxWidth;
   int maxRowTitleWidth = 0;
   int rowSpace         = MAXIMUM (0, rspace);
   int colSpace         = MAXIMUM (0, cspace);
   int begx             = 0;
   int begy             = 0;
   int cellWidth        = 0;
   char **temp          = 0;
   int x, y;
   int borderw          = 0;
   bool have_rowtitles  = FALSE;
   bool have_coltitles  = FALSE;
   /* *INDENT-OFF* */
   static const struct { int from; int to; } bindings[] = {
      { CDK_FORCHAR,	KEY_NPAGE },
      { CDK_BACKCHAR,	KEY_PPAGE },
   };
   /* *INDENT-ON* */

   if ((matrix = newCDKObject (CDKMATRIX, &my_funcs)) == 0)
   {
      return (0);
   }

   setCDKMatrixBox (matrix, Box);
   borderw = (ObjOf (matrix)->box) ? 1 : 0;

   /* Make sure that the number of rows/cols/vrows/vcols is not zero. */
   if (rows <= 0 || cols <= 0 || vrows <= 0 || vcols <= 0)
   {
      destroyCDKObject (matrix);
      return (0);
   }
#if NEW_CDKMATRIX
   matrix->cell = typeCallocN (WINDOW *, (rows + 1) * (cols + 1));
   matrix->info = typeCallocN (char *, (rows + 1) * (cols + 1));
#endif

   /*
    * Make sure the number of virtual cells is not larger than
    * the physical size.
    */
   vrows = (vrows > rows ? rows : vrows);
   vcols = (vcols > cols ? cols : vcols);

   /* Set these early, since they are used in matrix index computations */
   /* *INDENT-EQLS* */
   matrix->rows        = rows;
   matrix->cols        = cols;
   /* columns */
   matrix->colwidths   = typeCallocN (int, cols + 1);
   matrix->colvalues   = typeCallocN (int, cols + 1);
   matrix->coltitle    = typeCallocN (chtype *, cols + 1);
   matrix->coltitleLen = typeCallocN (int, cols + 1);
   matrix->coltitlePos = typeCallocN (int, cols + 1);
   /* titles */
   matrix->rowtitle    = typeCallocN (chtype *, rows + 1);
   matrix->rowtitleLen = typeCallocN (int, rows + 1);
   matrix->rowtitlePos = typeCallocN (int, rows + 1);

   /*
    * Count the number of lines in the title (see setCdkTitle).
    */
   temp = CDKsplitString (title, '\n');
   TitleLinesOf (matrix) = (int)CDKcountStrings ((CDK_CSTRING2)temp);
   CDKfreeStrings (temp);

   /* Determine the height of the box. */
   if (vrows == 1)
   {
      boxHeight = 6 + TitleLinesOf (matrix);
   }
   else
   {
      if (rowSpace == 0)
      {
	 boxHeight = (6 + TitleLinesOf (matrix) +
		      ((vrows - 1) * 2));
      }
      else
      {
	 boxHeight = (3 + TitleLinesOf (matrix) +
		      (vrows * 3) + ((vrows - 1) * (rowSpace - 1)));
      }
   }

   /* Determine the maximum row title width */
   for (x = 1; x <= rows; x++)
   {
      if (rowtitles && !emptyString (rowtitles[x]))	/*VR */
	 have_rowtitles = TRUE;
      matrix->rowtitle[x] = char2Chtype (rowtitles ? rowtitles[x] : 0,	/*VR */
					 &matrix->rowtitleLen[x],
					 &matrix->rowtitlePos[x]);
      maxRowTitleWidth = MAXIMUM (maxRowTitleWidth, matrix->rowtitleLen[x]);
   }

   if (have_rowtitles)
   {
      matrix->maxrt = maxRowTitleWidth + 2;

      /* We need to rejustify the row title cell info. */
      for (x = 1; x <= rows; x++)
      {
	 matrix->rowtitlePos[x] = justifyString (matrix->maxrt,
						 matrix->rowtitleLen[x],
						 matrix->rowtitlePos[x]);
      }
   }
   else
   {
      matrix->maxrt = 0;
   }

   /* Determine the width of the matrix. */
   maxWidth = 2 + matrix->maxrt;
   for (x = 1; x <= vcols; x++)
   {
      maxWidth += colwidths[x] + 2 + colSpace;
   }
   maxWidth -= (colSpace - 1);
   boxWidth = MAXIMUM (maxWidth, boxWidth);

   boxWidth = setCdkTitle (ObjOf (matrix), title, boxWidth);

   /*
    * Make sure the dimensions of the window didn't
    * extend beyond the dimensions of the parent window.
    */
   boxWidth = (boxWidth > parentWidth ? parentWidth : boxWidth);
   boxHeight = (boxHeight > parentHeight ? parentHeight : boxHeight);

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* Make the pop-up window. */
   matrix->win = newwin (boxHeight, boxWidth, ypos, xpos);

   if (matrix->win == 0)
   {
      destroyCDKObject (matrix);
      return (0);
   }

   /* Make the subwindows in the pop-up. */
   begx = xpos;
   begy = ypos + borderw + TitleLinesOf (matrix);

   /* Make the 'empty' 0x0 cell. */
   MATRIX_CELL (matrix, 0, 0) = subwin (matrix->win, 3, matrix->maxrt, begy, begx);
   begx += matrix->maxrt + 1;

   /* Copy the titles into the structure. */
   for (x = 1; x <= cols; x++)
   {
      if (coltitles && !emptyString (coltitles[x]))	/*VR */
	 have_coltitles = TRUE;
      matrix->coltitle[x] = char2Chtype (coltitles ? coltitles[x] : 0,	/*VR */
					 &matrix->coltitleLen[x],
					 &matrix->coltitlePos[x]);
      matrix->coltitlePos[x] = (BorderOf (matrix) +
				justifyString (colwidths[x],
					       matrix->coltitleLen[x],
					       matrix->coltitlePos[x]));
      matrix->colwidths[x] = colwidths[x];
   }

   if (have_coltitles)
   {
      /* Make the column titles. */
      for (x = 1; x <= vcols; x++)
      {
	 cellWidth = colwidths[x] + 3;
	 MATRIX_CELL (matrix, 0, x) = subwin (matrix->win,
					      borderw,
					      cellWidth,
					      begy,
					      begx);

	 if (MATRIX_CELL (matrix, 0, x) == 0)
	 {
	    destroyCDKObject (matrix);
	    return (0);
	 }
	 begx += cellWidth + colSpace - 1;
      }
      begy++;
   }

   /* Make the main cell body */
   for (x = 1; x <= vrows; x++)
   {
      if (have_rowtitles)
      {
	 /* Make the row titles */
	 MATRIX_CELL (matrix, x, 0) = subwin (matrix->win,
					      3, matrix->maxrt,
					      begy, xpos + borderw);

	 if (MATRIX_CELL (matrix, x, 0) == 0)
	 {
	    destroyCDKObject (matrix);
	    return (0);
	 }
      }

      /* Set the start of the x position. */
      begx = xpos + matrix->maxrt + borderw;

      /* Make the cells */
      for (y = 1; y <= vcols; y++)
      {
	 cellWidth = colwidths[y] + 3;
	 MATRIX_CELL (matrix, x, y) = subwin (matrix->win,
					      3, cellWidth,
					      begy, begx);

	 if (MATRIX_CELL (matrix, x, y) == 0)
	 {
	    destroyCDKObject (matrix);
	    return (0);
	 }
	 begx += cellWidth + colSpace - 1;
	 keypad (MATRIX_CELL (matrix, x, y), TRUE);
      }
      begy += rowSpace + 2;
   }
   keypad (matrix->win, TRUE);

   /* *INDENT-EQLS* Keep the rest of the info. */
   ScreenOf (matrix)            = cdkscreen;
   ObjOf (matrix)->acceptsFocus = TRUE;
   ObjOf (matrix)->inputWindow  = matrix->win;
   matrix->parent               = cdkscreen->window;
   matrix->vrows                = vrows;
   matrix->vcols                = vcols;
   matrix->boxWidth             = boxWidth;
   matrix->boxHeight            = boxHeight;
   matrix->rowSpace             = rowSpace;
   matrix->colSpace             = colSpace;
   matrix->filler               = filler;
   matrix->dominant             = dominant;
   matrix->row                  = 1;
   matrix->col                  = 1;
   matrix->crow                 = 1;
   matrix->ccol                 = 1;
   matrix->trow                 = 1;
   matrix->lcol                 = 1;
   matrix->oldcrow              = 1;
   matrix->oldccol              = 1;
   matrix->oldvrow              = 1;
   matrix->oldvcol              = 1;
   initExitType (matrix);
   matrix->boxCell              = boxCell;
   matrix->shadow               = shadow;
   matrix->highlight            = A_REVERSE;
   matrix->callbackfn           = CDKMatrixCallBack;

   /* Make room for the cell information. */
   for (x = 1; x <= rows; x++)
   {
      for (y = 1; y <= cols; y++)
      {
	 MATRIX_INFO (matrix, x, y) = typeCallocN (char, (colwidths[y] + 1));
	 matrix->colvalues[y] = colvalues[y];
	 matrix->colwidths[y] = colwidths[y];
      }
   }

   /* Do we want a shadow??? */
   if (shadow)
   {
      matrix->shadowWin = newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   /* Setup the key bindings. */
   for (x = 0; x < (int)SIZEOF (bindings); ++x)
      bindCDKObject (vMATRIX, matrix,
		     (chtype)bindings[x].from,
		     getcCDKBind,
		     (void *)(long)bindings[x].to);

   /* Register this baby. */
   registerCDKObject (cdkscreen, vMATRIX, matrix);

   /* Return the matrix pointer */
   return (matrix);
}

/*
 * This activates the matrix.
 */
int activateCDKMatrix (CDKMATRIX *matrix, chtype *actions)
{
   int ret;

   /* Draw the matrix */
   drawCDKMatrix (matrix, ObjOf (matrix)->box);

   if (actions == 0)
   {
      boolean functionKey;

      for (;;)
      {
	 chtype input;

	 ObjOf (matrix)->inputWindow = CurMatrixCell (matrix);
	 keypad (ObjOf (matrix)->inputWindow, TRUE);
	 input = (chtype)getchCDKObject (ObjOf (matrix), &functionKey);

	 /* Inject the character into the widget. */
	 ret = injectCDKMatrix (matrix, input);
	 if (matrix->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }
   else
   {
      int length = chlen (actions);
      int x = 0;

      /* Inject each character one at a time. */
      for (x = 0; x < length; x++)
      {
	 ret = injectCDKMatrix (matrix, actions[x]);
	 if (matrix->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }

   /* Set the exit type and exit. */
   setExitType (matrix, 0);
   return -1;
}

/*
 * This injects a single character into the matrix widget.
 */
static int _injectCDKMatrix (CDKOBJS *object, chtype input)
{
   /* *INDENT-EQLS* */
   CDKMATRIX *widget = (CDKMATRIX *)object;
   int charcount     = (int)strlen (MATRIX_INFO (widget, widget->row, widget->col));
   int ppReturn      = 1;
   int ret           = unknownInt;
   bool complete     = FALSE;

   /* Set the exit type. */
   setExitType (widget, 0);

   /* Move the cursor to the correct position within the cell. */
   if (widget->colwidths[widget->ccol] == 1)
   {
      wmove (CurMatrixCell (widget), 1, 1);
   }
   else
   {
      wmove (CurMatrixCell (widget),
	     1,
	     (int)strlen (MATRIX_INFO (widget, widget->row, widget->col)) + 1);
   }

   /* Put the focus on the current cell */
   MyBox (CurMatrixCell (widget), WHOLE_BOX, A_BOLD);
   wrefresh (CurMatrixCell (widget));
   highlightCDKMatrixCell (widget);

   /* Check if there is a pre-process function to be called. */
   if (PreProcessFuncOf (widget) != 0)
   {
      /* Call the pre-process function. */
      ppReturn = PreProcessFuncOf (widget) (vMATRIX,
					    widget,
					    PreProcessDataOf (widget),
					    input);
   }

   /* Should we continue? */
   if (ppReturn != 0)
   {
      /* *INDENT-EQLS* */
      int refreshCells  = FALSE;
      int movedCell     = FALSE;

      /* Check the key bindings. */
      if (checkCDKObjectBind (vMATRIX, widget, input) != 0)
      {
	 checkEarlyExit (widget);
	 complete = TRUE;
      }
      else
      {
	 switch (input)
	 {
	 case CDK_TRANSPOSE:
	    break;

	 case KEY_HOME:
	    break;

	 case KEY_END:
	    break;

	 case KEY_BACKSPACE:
	 case KEY_DC:
	    if (widget->colvalues[widget->col] == vVIEWONLY || charcount <= 0)
	    {
	       Beep ();
	    }
	    else
	    {
	       charcount--;
	       mvwdelch (CurMatrixCell (widget), 1, charcount + 1);
	       mvwinsch (CurMatrixCell (widget), 1, charcount + 1, widget->filler);
	       wrefresh (CurMatrixCell (widget));
	       MATRIX_INFO (widget, widget->row, widget->col)[charcount] = '\0';
	    }
	    break;

	 case KEY_RIGHT:
	 case KEY_TAB:
	    if (widget->ccol != widget->vcols)
	    {
	       /* We are moving to the right... */
	       widget->col++;
	       widget->ccol++;
	       movedCell = TRUE;
	    }
	    else
	    {
	       /* We have to shift the columns to the right. */
	       if (widget->col != widget->cols)
	       {
		  widget->lcol++;
		  widget->col++;

		  /* Redraw the column titles. */
		  if (widget->rows > widget->vrows)
		  {
		     redrawTitles (widget, FALSE, TRUE);
		  }
		  refreshCells = TRUE;
		  movedCell = TRUE;
	       }
	       else
	       {
		  /* We are at the far right column, we need  */
		  /* shift down one row, if we can. */
		  if (widget->row == widget->rows)
		  {
		     Beep ();
		  }
		  else
		  {
		     /* Set up the columns info. */
		     widget->col = 1;
		     widget->lcol = 1;
		     widget->ccol = 1;

		     /* Shift the rows... */
		     if (widget->crow != widget->vrows)
		     {
			widget->row++;
			widget->crow++;
		     }
		     else
		     {
			widget->row++;
			widget->trow++;
		     }
		     redrawTitles (widget, TRUE, TRUE);
		     refreshCells = TRUE;
		     movedCell = TRUE;
		  }
	       }
	    }
	    break;

	 case KEY_LEFT:
	 case KEY_BTAB:
	    if (widget->ccol != 1)
	    {
	       /* We are moving to the left... */
	       widget->col--;
	       widget->ccol--;
	       movedCell = TRUE;
	    }
	    else
	    {
	       /* Are we at the far left??? */
	       if (widget->lcol != 1)
	       {
		  widget->lcol--;
		  widget->col--;

		  /* Redraw the column titles. */
		  if (widget->cols > widget->vcols)
		  {
		     redrawTitles (widget, FALSE, TRUE);
		  }
		  refreshCells = TRUE;
		  movedCell = TRUE;
	       }
	       else
	       {
		  /* Shift up one line if we can... */
		  if (widget->row == 1)
		  {
		     Beep ();
		  }
		  else
		  {
		     /* Set up the columns info. */
		     widget->col = widget->cols;
		     widget->lcol = widget->cols - widget->vcols + 1;
		     widget->ccol = widget->vcols;

		     /* Shift the rows... */
		     if (widget->crow != 1)
		     {
			widget->row--;
			widget->crow--;
		     }
		     else
		     {
			widget->row--;
			widget->trow--;
		     }
		     redrawTitles (widget, TRUE, TRUE);
		     refreshCells = TRUE;
		     movedCell = TRUE;
		  }
	       }
	    }
	    break;

	 case KEY_UP:
	    if (widget->crow != 1)
	    {
	       widget->row--;
	       widget->crow--;
	       movedCell = TRUE;
	    }
	    else
	    {
	       if (widget->trow != 1)
	       {
		  widget->trow--;
		  widget->row--;

		  /* Redraw the row titles. */
		  if (widget->rows > widget->vrows)
		  {
		     redrawTitles (widget, TRUE, FALSE);
		  }
		  refreshCells = TRUE;
		  movedCell = TRUE;
	       }
	       else
	       {
		  Beep ();
	       }
	    }
	    break;

	 case KEY_DOWN:
	    if (widget->crow != widget->vrows)
	    {
	       widget->row++;
	       widget->crow++;
	       movedCell = TRUE;
	    }
	    else
	    {
	       if ((widget->trow + widget->vrows - 1) != widget->rows)
	       {
		  widget->trow++;
		  widget->row++;

		  /* Redraw the titles. */
		  if (widget->rows > widget->vrows)
		  {
		     redrawTitles (widget, TRUE, FALSE);
		  }
		  refreshCells = TRUE;
		  movedCell = TRUE;
	       }
	       else
	       {
		  Beep ();
	       }
	    }
	    break;

	 case KEY_NPAGE:
	    if (widget->rows > widget->vrows)
	    {
	       if ((widget->trow + ((widget->vrows - 1) * 2)) <= widget->rows)
	       {
		  widget->trow += widget->vrows - 1;
		  widget->row += widget->vrows - 1;
		  redrawTitles (widget, TRUE, FALSE);
		  refreshCells = TRUE;
		  movedCell = TRUE;
	       }
	       else
	       {
		  Beep ();
	       }
	    }
	    else
	    {
	       Beep ();
	    }
	    break;

	 case KEY_PPAGE:
	    if (widget->rows > widget->vrows)
	    {
	       if ((widget->trow - ((widget->vrows - 1) * 2)) >= 1)
	       {
		  widget->trow -= widget->vrows - 1;
		  widget->row -= widget->vrows - 1;
		  redrawTitles (widget, TRUE, FALSE);
		  refreshCells = TRUE;
		  movedCell = TRUE;
	       }
	       else
	       {
		  Beep ();
	       }
	    }
	    else
	    {
	       Beep ();
	    }
	    break;

	 case CTRL ('G'):
	    jumpToCell (widget, -1, -1);
	    drawCDKMatrix (widget, ObjOf (widget)->box);
	    break;

	 case CDK_PASTE:
	    if (GPasteBuffer == 0 ||
		(int)strlen (GPasteBuffer) > widget->colwidths[widget->ccol])
	    {
	       Beep ();
	    }
	    else
	    {
	       strcpy (CurMatrixInfo (widget), GPasteBuffer);
	       drawCurCDKMatrixCell (widget);
	    }
	    break;

	 case CDK_COPY:
	    freeChar (GPasteBuffer);
	    GPasteBuffer = copyChar (CurMatrixInfo (widget));
	    break;

	 case CDK_CUT:
	    freeChar (GPasteBuffer);
	    GPasteBuffer = copyChar (CurMatrixInfo (widget));
	    cleanCDKMatrixCell (widget,
				widget->trow + widget->crow - 1,
				widget->lcol + widget->ccol - 1);
	    drawCurCDKMatrixCell (widget);
	    break;

	 case CDK_ERASE:
	    cleanCDKMatrixCell (widget,
				widget->trow + widget->crow - 1,
				widget->lcol + widget->ccol - 1);
	    drawCurCDKMatrixCell (widget);
	    break;

	 case KEY_ENTER:
	    if (!widget->boxCell)
	    {
	       attrbox (MATRIX_CELL (widget, widget->oldcrow, widget->oldccol),
			' ', ' ',
			' ', ' ',
			' ', ' ',
			A_NORMAL);
	    }
	    else
	    {
	       drawOldCDKMatrixCell (widget);
	    }
	    wrefresh (CurMatrixCell (widget));
	    setExitType (widget, input);
	    ret = 1;
	    complete = TRUE;
	    break;

	 case KEY_ERROR:
	    setExitType (widget, input);
	    complete = TRUE;
	    break;

	 case KEY_ESC:
	    if (!widget->boxCell)
	    {
	       attrbox (MATRIX_CELL (widget, widget->oldcrow, widget->oldccol),
			' ', ' ',
			' ', ' ',
			' ', ' ',
			A_NORMAL);
	    }
	    else
	    {
	       drawOldCDKMatrixCell (widget);
	    }
	    wrefresh (CurMatrixCell (widget));
	    setExitType (widget, input);
	    complete = TRUE;
	    break;

	 case CDK_REFRESH:
	    eraseCDKScreen (ScreenOf (widget));
	    refreshCDKScreen (ScreenOf (widget));
	    break;

	 default:
	    (widget->callbackfn) (widget, input);
	    break;
	 }
      }

      if (!complete)
      {
	 /* Did we change cells? */
	 if (movedCell)
	 {
	    /* un-highlight the old box  */
	    if (!widget->boxCell)
	    {
	       attrbox (MATRIX_CELL (widget, widget->oldcrow, widget->oldccol),
			' ', ' ',
			' ', ' ',
			' ', ' ',
			A_NORMAL);
	    }
	    else
	    {
	       drawOldCDKMatrixCell (widget);
	    }
	    wrefresh (MATRIX_CELL (widget, widget->oldcrow, widget->oldccol));

	    /* Highlight the new cell. */
	    MyBox (CurMatrixCell (widget), WHOLE_BOX, A_BOLD);
	    wrefresh (CurMatrixCell (widget));
	    highlightCDKMatrixCell (widget);
	 }

	 /* Redraw each cell. */
	 if (refreshCells)
	 {
	    drawEachCDKMatrixCell (widget);

	    /* Highlight the current cell. */
	    MyBox (CurMatrixCell (widget), WHOLE_BOX, A_BOLD);
	    wrefresh (CurMatrixCell (widget));
	    highlightCDKMatrixCell (widget);
	 }

	 /* Move to the correct position in the cell. */
	 if (refreshCells || movedCell)
	 {
	    if (widget->colwidths[widget->ccol] == 1)
	    {
	       wmove (CurMatrixCell (widget), 1, 1);
	    }
	    else
	    {
	       int infolen = (int)strlen (CurMatrixInfo (widget));
	       wmove (CurMatrixCell (widget), 1, infolen + 1);
	    }
	    wrefresh (CurMatrixCell (widget));
	 }

	 /* Should we call a post-process? */
	 if (PostProcessFuncOf (widget) != 0)
	 {
	    PostProcessFuncOf (widget) (vMATRIX,
					widget,
					PostProcessDataOf (widget),
					input);
	 }
      }
   }

   if (!complete)
   {
      /* Set the variables we need. */
      widget->oldcrow = widget->crow;
      widget->oldccol = widget->ccol;
      widget->oldvrow = widget->row;
      widget->oldvcol = widget->col;

      /* Set the exit type and exit. */
      setExitType (widget, 0);
   }

   ResultOf (widget).valueInt = ret;
   return (ret != unknownInt);
}

/*
 * This allows the programmer to define their own key mappings.
 */
static void CDKMatrixCallBack (CDKMATRIX *matrix, chtype input)
{
   EDisplayType disptype = (EDisplayType) matrix->colvalues[matrix->col];
   int plainchar = filterByDisplayType (disptype, input);
   int charcount = (int)strlen (MATRIX_INFO (matrix, matrix->row, matrix->col));

   if (plainchar == ERR)
   {
      Beep ();
   }
   else if (charcount == matrix->colwidths[matrix->col])
   {
      Beep ();
   }
   else
   {
      /* Update the screen. */
      wmove (CurMatrixCell (matrix),
	     1,
	     (int)strlen (MATRIX_INFO (matrix, matrix->row, matrix->col)) + 1);
      waddch (CurMatrixCell (matrix),
	      (chtype)((isHiddenDisplayType (disptype))
		       ? (int)matrix->filler
		       : plainchar));
      wrefresh (CurMatrixCell (matrix));

      /* Update the character pointer. */
      MATRIX_INFO (matrix, matrix->row, matrix->col)[charcount++] = (char)plainchar;
      MATRIX_INFO (matrix, matrix->row, matrix->col)[charcount] = '\0';
   }
}

/*
 * Highlight the new field.
 */
static void highlightCDKMatrixCell (CDKMATRIX *matrix)
{
   /* *INDENT-EQLS* */
   EDisplayType disptype = (EDisplayType) matrix->colvalues[matrix->col];
   chtype highlight      = matrix->highlight;
   int x                 = 0;
   int infolen           = (int)strlen (MATRIX_INFO (matrix, matrix->row, matrix->col));

   /*
    * Given the dominance of the colors/attributes, we need to set the
    * current cell attribute.
    */
   if (matrix->dominant == ROW)
   {
      highlight = matrix->rowtitle[matrix->crow][0] & A_ATTRIBUTES;
   }
   else if (matrix->dominant == COL)
   {
      highlight = matrix->coltitle[matrix->ccol][0] & A_ATTRIBUTES;
   }

   /* If the column is only one char. */
   for (x = 1; x <= matrix->colwidths[matrix->ccol]; x++)
   {
      chtype ch = (((x <= infolen) && !isHiddenDisplayType (disptype))
		   ? CharOf (MATRIX_INFO (matrix,
					  matrix->row,
					  matrix->col)[x - 1])
		   : matrix->filler);

      (void)mvwaddch (CurMatrixCell (matrix), 1, x, ch | highlight);
   }
   wmove (CurMatrixCell (matrix), 1, infolen + 1);
   wrefresh (CurMatrixCell (matrix));
}

/*
 * This moves the matrix field to the given location.
 */
static void _moveCDKMatrix (CDKOBJS *object,
			    int xplace,
			    int yplace,
			    boolean relative,
			    boolean refresh_flag)
{
   /* *INDENT-EQLS* */
   CDKMATRIX *matrix = (CDKMATRIX *)object;
   int currentX      = getbegx (matrix->win);
   int currentY      = getbegy (matrix->win);
   int xpos          = xplace;
   int ypos          = yplace;
   int xdiff         = 0;
   int ydiff         = 0;
   int x, y;

   /*
    * If this is a relative move, then we will adjust where we want
    * to move to.
    */
   if (relative)
   {
      xpos = getbegx (matrix->win) + xplace;
      ypos = getbegy (matrix->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (matrix), &xpos, &ypos, matrix->boxWidth, matrix->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (matrix->win, -xdiff, -ydiff);

   for (x = 0; x <= matrix->vrows; x++)
   {
      for (y = 0; y <= matrix->vcols; y++)
      {
	 moveCursesWindow (MATRIX_CELL (matrix, x, y), -xdiff, -ydiff);
      }
   }

   moveCursesWindow (matrix->shadowWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (matrix));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKMatrix (matrix, ObjOf (matrix)->box);
   }
}

/*
 * This draws a cell within a matrix.
 */
static void drawCDKMatrixCell (CDKMATRIX *matrix,
			       int row,
			       int col,
			       int vrow,
			       int vcol,
			       boolean Box)
{
   /* *INDENT-EQLS* */
   WINDOW *cell         = MATRIX_CELL (matrix, row, col);
   EDisplayType disptype = (EDisplayType) matrix->colvalues[matrix->col];
   chtype highlight     = matrix->filler & A_ATTRIBUTES;
   int rows             = matrix->vrows;
   int cols             = matrix->vcols;
   int infolen          = (int)strlen (MATRIX_INFO (matrix, vrow, vcol));
   chtype attr          = A_NORMAL;
   int x;

   /*
    * Given the dominance of the colors/attributes, we need to set the
    * current cell attribute.
    */
   if (matrix->dominant == ROW)
   {
      highlight = matrix->rowtitle[row][0] & A_ATTRIBUTES;
   }
   else if (matrix->dominant == COL)
   {
      highlight = matrix->coltitle[col][0] & A_ATTRIBUTES;
   }

   /* Draw in the cell info. */
   for (x = 1; x <= matrix->colwidths[col]; x++)
   {
      chtype ch = (((x <= infolen) && !isHiddenDisplayType (disptype))
		   ? (CharOf (MATRIX_INFO (matrix, vrow, vcol)[x - 1]) | highlight)
		   : matrix->filler);

      (void)mvwaddch (cell, 1, x, ch | highlight);
   }
   wmove (cell, 1, infolen + 1);
   wrefresh (cell);

   /* Only draw the box iff the user asked for a box. */
   if (!Box)
   {
      return;
   }

   /*
    * If the value of the column spacing is greater than 0 then these
    * are independent boxes.
    */
   if (matrix->colSpace != 0)
   {
      if (matrix->rowSpace != 0)
      {
	 MyBox (cell, WHOLE_BOX, attr);
      }
      else
      {
	 if (row == 1)
	 {
	    MyBox (cell, TOP_C_BOX, attr);
	 }
	 else if (row > 1 && row < rows)
	 {
	    MyBox (cell, MID_C_BOX, attr);
	 }
	 else if (row == rows)
	 {
	    MyBox (cell, BOT_C_BOX, attr);
	 }
      }
   }
   else if (matrix->rowSpace != 0)
   {
      if (col == 1)
      {
	 MyBox (cell, LFT_R_BOX, attr);
      }
      else if (col > 1 && col < cols)
      {
	 MyBox (cell, MID_R_BOX, attr);
      }
      else if (col == cols)
      {
	 MyBox (cell, RGT_R_BOX, attr);
      }
   }
   else
   {
      if (row == 1)
      {
	 if (col == 1)
	 {
	    MyBox (cell, LFT_T_BOX, attr);	/* top left corner */
	 }
	 else if (col > 1 && col < cols)
	 {
	    MyBox (cell, MID_T_BOX, attr);	/* top middle */
	 }
	 else if (col == cols)
	 {
	    MyBox (cell, RGT_T_BOX, attr);	/* top right corner */
	 }
      }
      else if (row > 1 && row < rows)
      {
	 if (col == 1)
	 {
	    MyBox (cell, LFT_M_BOX, attr);	/* middle left */
	 }
	 else if (col > 1 && col < cols)
	 {
	    MyBox (cell, MID_M_BOX, attr);	/* middle */
	 }
	 else if (col == cols)
	 {
	    MyBox (cell, RGT_M_BOX, attr);	/* middle right */
	 }
      }
      else if (row == rows)
      {
	 if (col == 1)
	 {
	    MyBox (cell, LFT_B_BOX, attr);	/* bottom left corner */
	 }
	 else if (col > 1 && col < cols)
	 {
	    MyBox (cell, MID_B_BOX, attr);	/* bottom middle */
	 }
	 else if (col == cols)
	 {
	    MyBox (cell, RGT_B_BOX, attr);	/* bottom right corner */
	 }
      }
   }

   /* Highlight the current cell. */
   MyBox (CurMatrixCell (matrix), WHOLE_BOX, A_BOLD);
   wrefresh (CurMatrixCell (matrix));
   highlightCDKMatrixCell (matrix);
}

static void drawEachColTitle (CDKMATRIX *matrix)
{
   int x;

   for (x = 1; x <= matrix->vcols; x++)
   {
      if (MATRIX_CELL (matrix, 0, x))
      {
	 werase (MATRIX_CELL (matrix, 0, x));	/*VR */
	 writeChtype (MATRIX_CELL (matrix, 0, x),
		      matrix->coltitlePos[matrix->lcol + x - 1], 0,
		      matrix->coltitle[matrix->lcol + x - 1],
		      HORIZONTAL,
		      0, matrix->coltitleLen[matrix->lcol + x - 1]);
	 wrefresh (MATRIX_CELL (matrix, 0, x));
      }
   }
}

static void drawEachRowTitle (CDKMATRIX *matrix)
{
   int x;

   for (x = 1; x <= matrix->vrows; x++)
   {
      if (MATRIX_CELL (matrix, x, 0))
      {
	 werase (MATRIX_CELL (matrix, x, 0));
	 writeChtype (MATRIX_CELL (matrix, x, 0),
		      matrix->rowtitlePos[matrix->trow + x - 1], 1,
		      matrix->rowtitle[matrix->trow + x - 1],
		      HORIZONTAL,
		      0, matrix->rowtitleLen[matrix->trow + x - 1]);
	 wrefresh (MATRIX_CELL (matrix, x, 0));
      }
   }
}

static void drawEachCDKMatrixCell (CDKMATRIX *matrix)
{
   int y, x;

   /* Fill in the cells. */
   for (x = 1; x <= matrix->vrows; x++)
   {
      for (y = 1; y <= matrix->vcols; y++)
      {
	 drawCDKMatrixCell (matrix, x, y,
			    matrix->trow + x - 1,
			    matrix->lcol + y - 1,
			    matrix->boxCell);
      }
   }
}

static void drawCurCDKMatrixCell (CDKMATRIX *matrix)
{
   drawCDKMatrixCell (matrix,
		      matrix->crow,
		      matrix->ccol,
		      matrix->row,
		      matrix->col,
		      matrix->boxCell);
}

static void drawOldCDKMatrixCell (CDKMATRIX *matrix)
{
   drawCDKMatrixCell (matrix,
		      matrix->oldcrow,
		      matrix->oldccol,
		      matrix->oldvrow,
		      matrix->oldvcol,
		      matrix->boxCell);
}

/*
 * This function draws the matrix widget.
 */
static void _drawCDKMatrix (CDKOBJS *object, boolean Box)
{
   CDKMATRIX *matrix = (CDKMATRIX *)object;

   /* Did we ask for a shadow??? */
   if (matrix->shadowWin != 0)
   {
      drawShadow (matrix->shadowWin);
   }

   /* Should we box the matrix??? */
   if (Box)
   {
      drawObjBox (matrix->win, ObjOf (matrix));
   }

   drawCdkTitle (matrix->win, object);

   wrefresh (matrix->win);

   drawEachColTitle (matrix);
   drawEachRowTitle (matrix);
   drawEachCDKMatrixCell (matrix);

   /* Highlight the current cell. */
   MyBox (CurMatrixCell (matrix), WHOLE_BOX, A_BOLD);
   wrefresh (CurMatrixCell (matrix));
   highlightCDKMatrixCell (matrix);
}

/*
 * This function destroys the matrix widget.
 */
static void _destroyCDKMatrix (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKMATRIX *matrix = (CDKMATRIX *)object;
      int x = 0;
      int y = 0;

      cleanCdkTitle (object);

      /* Clear out the col titles. */
      for (x = 1; x <= matrix->cols; x++)
      {
	 freeChtype (matrix->coltitle[x]);
      }

      /* Clear out the row titles. */
      for (x = 1; x <= matrix->rows; x++)
      {
	 freeChtype (matrix->rowtitle[x]);
      }

      /* Clear out the matrix cells. */
      for (x = 1; x <= matrix->rows; x++)
      {
	 for (y = 1; y <= matrix->cols; y++)
	 {
	    freeChar (MATRIX_INFO (matrix, x, y));
	 }
      }

      /* Clear the matrix windows. */
      deleteCursesWindow (MATRIX_CELL (matrix, 0, 0));
      for (x = 1; x <= matrix->vrows; x++)
      {
	 deleteCursesWindow (MATRIX_CELL (matrix, x, 0));
      }
      for (x = 1; x <= matrix->vcols; x++)
      {
	 deleteCursesWindow (MATRIX_CELL (matrix, 0, x));
      }
      for (x = 1; x <= matrix->vrows; x++)
      {
	 for (y = 1; y <= matrix->vcols; y++)
	 {
	    deleteCursesWindow (MATRIX_CELL (matrix, x, y));
	 }
      }

#if NEW_CDKMATRIX
      freeChecked (matrix->cell);
      freeChecked (matrix->info);
#endif

      freeChecked (matrix->colwidths);
      freeChecked (matrix->colvalues);

      freeChecked (matrix->coltitle);
      freeChecked (matrix->coltitleLen);
      freeChecked (matrix->coltitlePos);

      freeChecked (matrix->rowtitle);
      freeChecked (matrix->rowtitleLen);
      freeChecked (matrix->rowtitlePos);

      deleteCursesWindow (matrix->shadowWin);
      deleteCursesWindow (matrix->win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vMATRIX, matrix);

      /* Unregister this object. */
      unregisterCDKObject (vMATRIX, matrix);
   }
}

/*
 * This function erases the matrix widget from the screen.
 */
static void _eraseCDKMatrix (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKMATRIX *matrix = (CDKMATRIX *)object;
      int x = 0;
      int y = 0;

      /* Clear the matrix cells. */
      eraseCursesWindow (MATRIX_CELL (matrix, 0, 0));
      for (x = 1; x <= matrix->vrows; x++)
      {
	 eraseCursesWindow (MATRIX_CELL (matrix, x, 0));
      }
      for (x = 1; x <= matrix->vcols; x++)
      {
	 eraseCursesWindow (MATRIX_CELL (matrix, 0, x));
      }
      for (x = 1; x <= matrix->vrows; x++)
      {
	 for (y = 1; y <= matrix->vcols; y++)
	 {
	    eraseCursesWindow (MATRIX_CELL (matrix, x, y));
	 }
      }
      eraseCursesWindow (matrix->shadowWin);
      eraseCursesWindow (matrix->win);
   }
}

/*
 * Set the callback-function.
 */
void setCDKMatrixCB (CDKMATRIX *widget, MATRIXCB callback)
{
   widget->callbackfn = callback;
}

/*
 * This function sets the values of the matrix widget.
 */
void setCDKMatrixCells (CDKMATRIX *matrix,
			CDK_CSTRING2 info,
			int rows,
			int maxcols,
			int *subSize)
{
   int x = 0;
   int y = 0;

   if (rows > matrix->rows)
      rows = matrix->rows;

   /* Copy in the new info. */
   for (x = 1; x <= rows; x++)
   {
      for (y = 1; y <= matrix->cols; y++)
      {
	 if (x <= rows && y <= subSize[x])
	 {
	    const char *source = info[(x * maxcols) + y];

	    /* Copy in the new information. */
	    if (source != 0)
	    {
	       char *target = MATRIX_INFO (matrix, x, y);

	       if (target == 0)	/* this should not happen... */
	       {
		  target = typeCallocN (char, matrix->colwidths[y] + 1);
		  MATRIX_INFO (matrix, x, y) = target;
	       }
	       strncpy (MATRIX_INFO (matrix, x, y),
			source,
			(size_t)matrix->colwidths[y]);
	    }
	 }
	 else
	    cleanCDKMatrixCell (matrix, x, y);
      }
   }
}

/*
 * This sets the widget's box attribute.
 */
void setCDKMatrixBox (CDKMATRIX *matrix, boolean Box)
{
   ObjOf (matrix)->box = Box;
   ObjOf (matrix)->borderSize = Box ? 1 : 0;
}
boolean getCDKMatrixBox (CDKMATRIX *matrix)
{
   return ObjOf (matrix)->box;
}

/*
 * This cleans out the information cells in the matrix widget.
 */
void cleanCDKMatrix (CDKMATRIX *matrix)
{
   int x = 0;
   int y = 0;

   for (x = 1; x <= matrix->rows; x++)
   {
      for (y = 1; y <= matrix->cols; y++)
      {
	 cleanCDKMatrixCell (matrix, x, y);
      }
   }
}

/*
 * This cleans one cell in the matrix widget.
 */
void cleanCDKMatrixCell (CDKMATRIX *matrix, int row, int col)
{
   if (row > 0 && row <= matrix->rows &&
       col > 0 && col <= matrix->cols)
      cleanChar (MATRIX_INFO (matrix, row, col), matrix->colwidths[col], '\0');
}

/*
 * This allows us to hyper-warp to a cell.
 */
int jumpToCell (CDKMATRIX *matrix, int row, int col)
{
   CDKSCALE *scale = 0;
   int newRow = row;
   int newCol = col;

   /*
    * Only create the row scale if needed.
    */
   if ((row == -1) || (row > matrix->rows))
   {
      /* Create the row scale widget. */
      scale = newCDKScale (ScreenOf (matrix),
			   CENTER, CENTER,
			   "<C>Jump to which row.",
			   "</5/B>Row: ", A_NORMAL, 5,
			   1, 1, matrix->rows, 1, 1, TRUE, FALSE);

      /* Activate the scale and get the row. */
      newRow = activateCDKScale (scale, 0);
      destroyCDKScale (scale);
   }

   /*
    * Only create the column scale if needed.
    */
   if ((col == -1) || (col > matrix->cols))
   {
      /* Create the column scale widget. */
      scale = newCDKScale (ScreenOf (matrix),
			   CENTER, CENTER,
			   "<C>Jump to which column",
			   "</5/B>Col: ", A_NORMAL, 5,
			   1, 1, matrix->cols, 1, 1, TRUE, FALSE);

      /* Activate the scale and get the column. */
      newCol = activateCDKScale (scale, 0);
      destroyCDKScale (scale);
   }

   /* Hyper-warp.... */
   if (newRow != matrix->row || newCol != matrix->col)
   {
      return (moveToCDKMatrixCell (matrix, newRow, newCol));
   }
   else
   {
      return 1;
   }
}

/*
 * This allows us to move to a given cell.
 */
int moveToCDKMatrixCell (CDKMATRIX *matrix, int newrow, int newcol)
{
   int rowShift = newrow - matrix->row;
   int colShift = newcol - matrix->col;

   /* Make sure we arent asking to move out of the matrix. */
   if (newrow > matrix->rows ||
       newcol > matrix->cols ||
       newrow <= 0 ||
       newcol <= 0)
   {
      return 0;
   }

   /* Did we move up/down???? */
   if (rowShift > 0)
   {
      /* We are moving down. */
      if (matrix->vrows == matrix->cols)
      {
	 matrix->trow = 1;
	 matrix->crow = newrow;
	 matrix->row = newrow;
      }
      else
      {
	 if ((rowShift + matrix->vrows) < matrix->rows)
	 {
	    /* Just shift down by rowShift... */
	    matrix->trow += rowShift;
	    matrix->crow = 1;
	    matrix->row += rowShift;
	 }
	 else
	 {
	    /* We need to munge with the values... */
	    matrix->trow = matrix->rows - matrix->vrows + 1;
	    matrix->crow = ((rowShift + matrix->vrows) - matrix->rows) + 1;
	    matrix->row = newrow;
	 }
      }
   }
   else if (rowShift < 0)
   {
      /* We are moving up. */
      if (matrix->vrows == matrix->rows)
      {
	 matrix->trow = 1;
	 matrix->row = newrow;
	 matrix->crow = newrow;
      }
      else
      {
	 if ((rowShift + matrix->vrows) > 1)
	 {
	    /* Just shift up by rowShift... */
	    matrix->trow += rowShift;
	    matrix->row += rowShift;
	    matrix->crow = 1;
	 }
	 else
	 {
	    /* We need to munge with the values... */
	    matrix->trow = 1;
	    matrix->crow = 1;
	    matrix->row = 1;
	 }
      }
   }

   /* Did we move left/right ???? */
   if (colShift > 0)
   {
      /* We are moving right. */
      if (matrix->vcols == matrix->cols)
      {
	 matrix->lcol = 1;
	 matrix->ccol = newcol;
	 matrix->col = newcol;
      }
      else
      {
	 if ((colShift + matrix->vcols) < matrix->cols)
	 {
	    matrix->lcol += colShift;
	    matrix->ccol = 1;
	    matrix->col += colShift;
	 }
	 else
	 {
	    /* We need to munge with the values... */
	    matrix->lcol = matrix->cols - matrix->vcols + 1;
	    matrix->ccol = ((colShift + matrix->vcols) - matrix->cols) + 1;
	    matrix->col = newcol;
	 }
      }
   }
   else if (colShift < 0)
   {
      /* We are moving left. */
      if (matrix->vcols == matrix->cols)
      {
	 matrix->lcol = 1;
	 matrix->col = newcol;
	 matrix->ccol = newcol;
      }
      else
      {
	 if ((colShift + matrix->vcols) > 1)
	 {
	    /* Just shift left by colShift... */
	    matrix->lcol += colShift;
	    matrix->col += colShift;
	    matrix->ccol = 1;
	 }
	 else
	 {
	    matrix->lcol = 1;
	    matrix->col = 1;
	    matrix->ccol = 1;
	 }
      }
   }

   /* Keep the 'old' values around for redrawing sake. */
   matrix->oldcrow = matrix->crow;
   matrix->oldccol = matrix->ccol;
   matrix->oldvrow = matrix->row;
   matrix->oldvcol = matrix->col;

   /* Lets ... */
   return 1;
}

/*
 * This redraws the titles indicated...
 */
static void redrawTitles (CDKMATRIX *matrix, int rowTitles, int colTitles)
{
   /* Redraw the row titles. */
   if (rowTitles)
   {
      drawEachRowTitle (matrix);
   }

   /* Redraw the column titles. */
   if (colTitles)
   {
      drawEachColTitle (matrix);
   }
}

/*
 * This sets the value of a matrix cell.
 */
int setCDKMatrixCell (CDKMATRIX *matrix, int row, int col, const char *value)
{
   /* Make sure the row/col combination is within the matrix. */
   if (row > matrix->rows || col > matrix->cols || row <= 0 || col <= 0)
   {
      return -1;
   }

   cleanCDKMatrixCell (matrix, row, col);
   strncpy (MATRIX_INFO (matrix, row, col),
	    value,
	    (size_t)matrix->colwidths[col]);
   return 1;
}

/*
 * This gets the value of a matrix cell.
 */
char *getCDKMatrixCell (CDKMATRIX *matrix, int row, int col)
{
   /* Make sure the row/col combination is within the matrix. */
   if (row > matrix->rows || col > matrix->cols || row <= 0 || col <= 0)
   {
      return 0;
   }
   return MATRIX_INFO (matrix, row, col);
}

/*
 * This returns the current row/col cell.
 */
int getCDKMatrixCol (CDKMATRIX *matrix)
{
   return matrix->col;
}
int getCDKMatrixRow (CDKMATRIX *matrix)
{
   return matrix->row;
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattrMatrix (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKMATRIX *widget = (CDKMATRIX *)object;
      int x, y;

      wbkgd (widget->win, attrib);
      for (x = 0; x <= widget->vrows; x++)
      {
	 for (y = 0; y <= widget->vcols; y++)
	 {
	    wbkgd (MATRIX_CELL (widget, x, y), attrib);
	 }
      }
   }
}

static void _focusCDKMatrix (CDKOBJS *object)
{
   CDKMATRIX *widget = (CDKMATRIX *)object;

   drawCDKMatrix (widget, ObjOf (widget)->box);
}

static void _unfocusCDKMatrix (CDKOBJS *object)
{
   CDKMATRIX *widget = (CDKMATRIX *)object;

   drawCDKMatrix (widget, ObjOf (widget)->box);
}

dummyRefreshData (Matrix)

dummySaveData (Matrix)
