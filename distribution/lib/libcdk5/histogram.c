#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2016/11/20 19:12:46 $
 * $Revision: 1.92 $
 */

DeclareCDKObjects (HISTOGRAM, Histogram, setCdk, Unknown);

/*
 * Create a histogram widget.
 */
CDKHISTOGRAM *newCDKHistogram (CDKSCREEN *cdkscreen,
			       int xplace,
			       int yplace,
			       int height,
			       int width,
			       int orient,
			       const char *title,
			       boolean Box,
			       boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKHISTOGRAM *widget = 0;
   int parentWidth      = getmaxx (cdkscreen->window);
   int parentHeight     = getmaxy (cdkscreen->window);
   int boxWidth;
   int boxHeight;
   int xpos             = xplace;
   int ypos             = yplace;
   int oldWidth         = 0;
   int oldHeight        = 0;

   if ((widget = newCDKObject (CDKHISTOGRAM, &my_funcs)) == 0)
        return (0);

   setCDKHistogramBox (widget, Box);

   boxHeight = setWidgetDimension (parentHeight, height, 2);
   oldHeight = boxHeight;

   boxWidth = setWidgetDimension (parentWidth, width, 0);
   oldWidth = boxWidth;

   boxWidth = setCdkTitle (ObjOf (widget), title, -(boxWidth + 1));

   /* Increment the height by the number of lines in the title. */
   boxHeight += TitleLinesOf (widget);

   /*
    * Make sure we didn't extend beyond the dimensions of the window.
    */
   boxWidth = (boxWidth > parentWidth ? oldWidth : boxWidth);
   boxHeight = (boxHeight > parentHeight ? oldHeight : boxHeight);

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* *INDENT-EQLS* Create the histogram pointer. */
   ScreenOf (widget)    = cdkscreen;
   widget->parent       = cdkscreen->window;
   widget->win          = newwin (boxHeight, boxWidth, ypos, xpos);
   widget->shadowWin    = 0;
   widget->boxWidth     = boxWidth;
   widget->boxHeight    = boxHeight;
   widget->fieldWidth   = boxWidth - 2 * BorderOf (widget);
   widget->fieldHeight  = boxHeight - TitleLinesOf (widget) - 2 * BorderOf (widget);
   widget->orient       = orient;
   widget->shadow       = shadow;

   /* Is the window null. */
   if (widget->win == 0)
   {
      destroyCDKObject (widget);
      return (0);
   }
   keypad (widget->win, TRUE);

   /* *INDENT-EQLS* Set up some default values. */
   widget->filler       = '#' | A_REVERSE;
   widget->statsAttr    = A_NORMAL;
   widget->statsPos     = TOP;
   widget->viewType     = vREAL;
   widget->high         = 0;
   widget->low          = 0;
   widget->value        = 0;
   widget->lowx         = 0;
   widget->lowy         = 0;
   widget->highx        = 0;
   widget->highy        = 0;
   widget->curx         = 0;
   widget->cury         = 0;
   widget->lowString    = 0;
   widget->highString   = 0;
   widget->curString    = 0;

   /* Do we want a shadow? */
   if (shadow)
   {
      widget->shadowWin = newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   registerCDKObject (cdkscreen, vHISTOGRAM, widget);

   return (widget);
}

/*
 * This was added for the builder.
 */
void activateCDKHistogram (CDKHISTOGRAM *widget, chtype *actions GCC_UNUSED)
{
   drawCDKHistogram (widget, ObjOf (widget)->box);
}

/*
 * Set various widget attributes.
 */
void setCDKHistogram (CDKHISTOGRAM *widget,
		      EHistogramDisplayType viewType,
		      int statsPos,
		      chtype statsAttr,
		      int low,
		      int high,
		      int value,
		      chtype filler,
		      boolean Box)
{
   setCDKHistogramDisplayType (widget, viewType);
   setCDKHistogramStatsPos (widget, statsPos);
   setCDKHistogramValue (widget, low, high, value);
   setCDKHistogramFillerChar (widget, filler);
   setCDKHistogramStatsAttr (widget, statsAttr);
   setCDKHistogramBox (widget, Box);
}

/*
 * Set the values for the widget.
 */
void setCDKHistogramValue (CDKHISTOGRAM *widget, int low, int high, int value)
{
   /* *INDENT-EQLS* We should error check the information we have. */
   widget->low          = (low <= high ? low : 0);
   widget->high         = (low <= high ? high : 0);
   widget->value        = (low <= value && value <= high ? value : 0);
   /* Determine the percentage of the given value. */
   widget->percent      = ((widget->high == 0)
			   ? 0
			   : ((float)widget->value / (float)widget->high));

   /* Determine the size of the histogram bar. */
   if (widget->orient == VERTICAL)
   {
      widget->barSize = (int)(widget->percent * (float)widget->fieldHeight);
   }
   else
   {
      widget->barSize = (int)(widget->percent * (float)widget->fieldWidth);
   }

   /*
    * We have a number of variables which determine the personality of the
    * histogram.  We have to go through each one methodically, and set them
    * correctly.  This section does this.
    */
   if (widget->viewType != vNONE)
   {
      char string[100];
      int len;

      if (widget->orient == VERTICAL)
      {
	 if (widget->statsPos == LEFT || widget->statsPos == BOTTOM)
	 {

	    /* Free the space used by the character strings. */
	    freeChar (widget->lowString);
	    freeChar (widget->highString);
	    freeChar (widget->curString);

	    /* *INDENT-EQLS* Set the low label attributes. */
	    sprintf (string, "%d", widget->low);
	    len                = (int)strlen (string);
	    widget->lowString  = copyChar (string);
	    widget->lowx       = 1;
	    widget->lowy       = widget->boxHeight - len - 1;

	    /* *INDENT-EQLS* Set the high label attributes. */
	    sprintf (string, "%d", widget->high);
	    widget->highString = copyChar (string);
	    widget->highx      = 1;
	    widget->highy      = TitleLinesOf (widget) + 1;

	    /* Set the current value attributes. */
	    if (widget->viewType == vPERCENT)
	    {
	       sprintf (string, "%3.1f%%", (float)(widget->percent * 100));
	    }
	    else if (widget->viewType == vFRACTION)
	    {
	       sprintf (string, "%d/%d", widget->value, widget->high);
	    }
	    else
	    {
	       sprintf (string, "%d", widget->value);
	    }
	    len = (int)strlen (string);
	    widget->curString = copyChar (string);
	    widget->curx = 1;
	    widget->cury = (((widget->fieldHeight - len) / 2)
			    + TitleLinesOf (widget) + 1);
	 }
	 else if (widget->statsPos == CENTER)
	 {
	    /* Set the character strings correctly. */
	    freeChar (widget->lowString);
	    freeChar (widget->highString);
	    freeChar (widget->curString);

	    /* *INDENT-EQLS* Set the low label attributes. */
	    sprintf (string, "%d", widget->low);
	    len                = (int)strlen (string);
	    widget->lowString  = copyChar (string);
	    widget->lowx       = (widget->fieldWidth / 2) + 1;
	    widget->lowy       = widget->boxHeight - len - 1;

	    /* *INDENT-EQLS* Set the high label attributes. */
	    sprintf (string, "%d", widget->high);
	    widget->highString = copyChar (string);
	    widget->highx      = (widget->fieldWidth / 2) + 1;
	    widget->highy      = TitleLinesOf (widget) + 1;

	    /* Set the stats label attributes. */
	    if (widget->viewType == vPERCENT)
	    {
	       sprintf (string, "%3.2f%%", (float)(widget->percent * 100));
	    }
	    else if (widget->viewType == vFRACTION)
	    {
	       sprintf (string, "%d/%d", widget->value, widget->high);
	    }
	    else
	    {
	       sprintf (string, "%d", widget->value);
	    }
	    /* *INDENT-EQLS* */
	    len                = (int)strlen (string);
	    widget->curString  = copyChar (string);
	    widget->curx       = (widget->fieldWidth / 2) + 1;
	    widget->cury       = (((widget->fieldHeight - len) / 2)
				                     + TitleLinesOf (widget)
				                     + 1);
	 }
	 else if (widget->statsPos == RIGHT || widget->statsPos == TOP)
	 {
	    /* Set the character strings correctly. */
	    freeChar (widget->lowString);
	    freeChar (widget->highString);
	    freeChar (widget->curString);

	    /* *INDENT-EQLS* Set the low label attributes. */
	    sprintf (string, "%d", widget->low);
	    len                = (int)strlen (string);
	    widget->lowString  = copyChar (string);
	    widget->lowx       = widget->fieldWidth;
	    widget->lowy       = widget->boxHeight - len - 1;

	    /* *INDENT-EQLS* Set the high label attributes. */
	    sprintf (string, "%d", widget->high);
	    widget->highString = copyChar (string);
	    widget->highx      = widget->fieldWidth;
	    widget->highy      = TitleLinesOf (widget) + 1;

	    /* Set the stats label attributes. */
	    if (widget->viewType == vPERCENT)
	    {
	       sprintf (string, "%3.2f%%", (float)(widget->percent * 100));
	    }
	    else if (widget->viewType == vFRACTION)
	    {
	       sprintf (string, "%d/%d", widget->value, widget->high);
	    }
	    else
	    {
	       sprintf (string, "%d", widget->value);
	    }
	    /* *INDENT-EQLS* */
	    len                = (int)strlen (string);
	    widget->curString  = copyChar (string);
	    widget->curx       = widget->fieldWidth;
	    widget->cury       = (((widget->fieldHeight - len) / 2)
				                     + TitleLinesOf (widget)
				                     + 1);
	 }
      }
      else
      {
	 /* Alignment is HORIZONTAL. */
	 if (widget->statsPos == TOP || widget->statsPos == RIGHT)
	 {
	    /* Set the character strings correctly. */
	    freeChar (widget->lowString);
	    freeChar (widget->highString);
	    freeChar (widget->curString);

	    /* *INDENT-EQLS* Set the low label attributes. */
	    sprintf (string, "%d", widget->low);
	    widget->lowString  = copyChar (string);
	    widget->lowx       = 1;
	    widget->lowy       = TitleLinesOf (widget) + 1;

	    /* *INDENT-EQLS* Set the high label attributes. */
	    sprintf (string, "%d", widget->high);
	    len                = (int)strlen (string);
	    widget->highString = copyChar (string);
	    widget->highx      = widget->boxWidth - len - 1;
	    widget->highy      = TitleLinesOf (widget) + 1;

	    /* Set the stats label attributes. */
	    if (widget->viewType == vPERCENT)
	    {
	       sprintf (string, "%3.1f%%", (float)(widget->percent * 100));
	    }
	    else if (widget->viewType == vFRACTION)
	    {
	       sprintf (string, "%d/%d", widget->value, widget->high);
	    }
	    else
	    {
	       sprintf (string, "%d", widget->value);
	    }
	    /* *INDENT-EQLS* */
	    len                = (int)strlen (string);
	    widget->curString  = copyChar (string);
	    widget->curx       = (widget->fieldWidth - len) / 2 + 1;
	    widget->cury       = TitleLinesOf (widget) + 1;
	 }
	 else if (widget->statsPos == CENTER)
	 {
	    /* Set the character strings correctly. */
	    freeChar (widget->lowString);
	    freeChar (widget->highString);
	    freeChar (widget->curString);

	    /* *INDENT-EQLS* Set the low label attributes. */
	    sprintf (string, "%d", widget->low);
	    widget->lowString  = copyChar (string);
	    widget->lowx       = 1;
	    widget->lowy       = ((widget->fieldHeight / 2)
				                     + TitleLinesOf (widget)
				                     + 1);

	    /* *INDENT-EQLS* Set the high label attributes. */
	    sprintf (string, "%d", widget->high);
	    len                = (int)strlen (string);
	    widget->highString = copyChar (string);
	    widget->highx      = widget->boxWidth - len - 1;
	    widget->highy      = ((widget->fieldHeight / 2)
				                     + TitleLinesOf (widget)
				                     + 1);

	    /* Set the stats label attributes. */
	    if (widget->viewType == vPERCENT)
	    {
	       sprintf (string, "%3.1f%%", (float)(widget->percent * 100));
	    }
	    else if (widget->viewType == vFRACTION)
	    {
	       sprintf (string, "%d/%d", widget->value, widget->high);
	    }
	    else
	    {
	       sprintf (string, "%d", widget->value);
	    }
	    /* *INDENT-EQLS* */
	    len                = (int)strlen (string);
	    widget->curString  = copyChar (string);
	    widget->curx       = (widget->fieldWidth - len) / 2 + 1;
	    widget->cury       = ((widget->fieldHeight / 2)
				                     + TitleLinesOf (widget)
				                     + 1);
	 }
	 else if (widget->statsPos == BOTTOM || widget->statsPos == LEFT)
	 {
	    /* Set the character strings correctly. */
	    freeChar (widget->lowString);
	    freeChar (widget->highString);
	    freeChar (widget->curString);

	    /* *INDENT-EQLS* Set the low label attributes. */
	    sprintf (string, "%d", widget->low);
	    widget->lowString  = copyChar (string);
	    widget->lowx       = 1;
	    widget->lowy       = widget->boxHeight - 2 * BorderOf (widget);

	    /* *INDENT-EQLS* Set the high label attributes. */
	    sprintf (string, "%d", widget->high);
	    len                = (int)strlen (string);
	    widget->highString = copyChar (string);
	    widget->highx      = widget->boxWidth - len - 1;
	    widget->highy      = widget->boxHeight - 2 * BorderOf (widget);

	    /* Set the stats label attributes. */
	    if (widget->viewType == vPERCENT)
	    {
	       sprintf (string, "%3.1f%%", (float)(widget->percent * 100));
	    }
	    else if (widget->viewType == vFRACTION)
	    {
	       sprintf (string, "%d/%d", widget->value, widget->high);
	    }
	    else
	    {
	       sprintf (string, "%d", widget->value);
	    }
	    /* *INDENT-EQLS* */
	    widget->curString  = copyChar (string);
	    widget->curx       = (widget->fieldWidth - len) / 2 + 1;
	    widget->cury       = widget->boxHeight - 2 * BorderOf (widget);
	 }
      }
   }
}
int getCDKHistogramValue (CDKHISTOGRAM *widget)
{
   return widget->value;
}
int getCDKHistogramLowValue (CDKHISTOGRAM *widget)
{
   return widget->low;
}
int getCDKHistogramHighValue (CDKHISTOGRAM *widget)
{
   return widget->high;
}

/*
 * Set the histogram display type.
 */
void setCDKHistogramDisplayType (CDKHISTOGRAM *widget, EHistogramDisplayType viewType)
{
   widget->viewType = viewType;
}
EHistogramDisplayType getCDKHistogramViewType (CDKHISTOGRAM *widget)
{
   return widget->viewType;
}

/*
 * Set the position of the statistics information.
 */
void setCDKHistogramStatsPos (CDKHISTOGRAM *widget, int statsPos)
{
   widget->statsPos = statsPos;
}
int getCDKHistogramStatsPos (CDKHISTOGRAM *widget)
{
   return widget->statsPos;
}

/*
 * Set the attribute of the statistics.
 */
void setCDKHistogramStatsAttr (CDKHISTOGRAM *widget, chtype statsAttr)
{
   widget->statsAttr = statsAttr;
}
chtype getCDKHistogramStatsAttr (CDKHISTOGRAM *widget)
{
   return widget->statsAttr;
}

/*
 * Set the character to use when drawing the widget.
 */
void setCDKHistogramFillerChar (CDKHISTOGRAM *widget, chtype character)
{
   widget->filler = character;
}
chtype getCDKHistogramFillerChar (CDKHISTOGRAM *widget)
{
   return widget->filler;
}

/*
 * Set the widget box attribute.
 */
void setCDKHistogramBox (CDKHISTOGRAM *widget, boolean Box)
{
   ObjOf (widget)->box = Box;
   ObjOf (widget)->borderSize = Box ? 1 : 0;
}
boolean getCDKHistogramBox (CDKHISTOGRAM *widget)
{
   return ObjOf (widget)->box;
}

/*
 * Set the background attribute of the widget.
 */
static void _setBKattrHistogram (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKHISTOGRAM *widget = (CDKHISTOGRAM *)object;
      wbkgd (widget->win, attrib);
   }
}

/*
 * Move the histogram field to the given location.
 */
static void _moveCDKHistogram (CDKOBJS *object,
			       int xplace,
			       int yplace,
			       boolean relative,
			       boolean refresh_flag)
{
   CDKHISTOGRAM *widget = (CDKHISTOGRAM *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx (widget->win);
   int currentY = getbegy (widget->win);
   int xpos     = xplace;
   int ypos     = yplace;
   int xdiff    = 0;
   int ydiff    = 0;

   /*
    * If this is a relative move, then we will adjust where we want
    * to move to.
    */
   if (relative)
   {
      xpos = getbegx (widget->win) + xplace;
      ypos = getbegy (widget->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (widget), &xpos, &ypos, widget->boxWidth, widget->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (widget->win, -xdiff, -ydiff);
   moveCursesWindow (widget->shadowWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (widget));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKHistogram (widget, ObjOf (widget)->box);
   }
}

/*
 * Draw the widget.
 */
static void _drawCDKHistogram (CDKOBJS *object, boolean Box)
{
   CDKHISTOGRAM *widget = (CDKHISTOGRAM *)object;
   /* *INDENT-EQLS* */
   chtype battr = 0;
   chtype bchar = 0;
   chtype fattr = widget->filler & A_ATTRIBUTES;
   int histX    = TitleLinesOf (widget) + 1;
   int histY    = widget->barSize;
   int x, y;

   /* Erase the window. */
   werase (widget->win);

   /* Box the widget if asked. */
   if (Box)
   {
      drawObjBox (widget->win, ObjOf (widget));
   }

   /* Do we have a shadow to draw? */
   if (widget->shadowWin != 0)
   {
      drawShadow (widget->shadowWin);
   }

   drawCdkTitle (widget->win, object);

   /* If the user asked for labels, draw them in. */
   if (widget->viewType != vNONE)
   {
      /* Draw in the low label. */
      if (widget->lowString != 0)
      {
	 int len = (int)strlen (widget->lowString);
	 writeCharAttrib (widget->win,
			  widget->lowx,
			  widget->lowy,
			  widget->lowString,
			  widget->statsAttr,
			  widget->orient,
			  0, len);
      }

      /* Draw in the current value label. */
      if (widget->curString != 0)
      {
	 int len = (int)strlen (widget->curString);
	 writeCharAttrib (widget->win,
			  widget->curx,
			  widget->cury,
			  widget->curString,
			  widget->statsAttr,
			  widget->orient,
			  0, len);
      }

      /* Draw in the high label. */
      if (widget->highString != 0)
      {
	 int len = (int)strlen (widget->highString);
	 writeCharAttrib (widget->win,
			  widget->highx,
			  widget->highy,
			  widget->highString,
			  widget->statsAttr,
			  widget->orient,
			  0, len);
      }
   }

   if (widget->orient == VERTICAL)
   {
      histX = widget->boxHeight - widget->barSize - 1;
      histY = widget->fieldWidth;
   }

   /* Draw the histogram bar. */
   for (x = histX; x < widget->boxHeight - 1; x++)
   {
      for (y = 1; y <= histY; y++)
      {
	 battr = mvwinch (widget->win, x, y);
	 bchar = CharOf (battr);

	 if (bchar == ' ')
	 {
	    (void)mvwaddch (widget->win, x, y, widget->filler);
	 }
	 else
	 {
	    (void)mvwaddch (widget->win, x, y, battr | fattr);
	 }
      }
   }

   /* Refresh the window. */
   wrefresh (widget->win);
}

/*
 * Destroy the widget.
 */
static void _destroyCDKHistogram (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKHISTOGRAM *widget = (CDKHISTOGRAM *)object;

      freeChar (widget->curString);
      freeChar (widget->lowString);
      freeChar (widget->highString);
      cleanCdkTitle (object);

      /* Clean up the windows. */
      deleteCursesWindow (widget->shadowWin);
      deleteCursesWindow (widget->win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vHISTOGRAM, widget);

      /* Unregister this object. */
      unregisterCDKObject (vHISTOGRAM, widget);
   }
}

/*
 * Erase the widget from the screen.
 */
static void _eraseCDKHistogram (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKHISTOGRAM *widget = (CDKHISTOGRAM *)object;

      eraseCursesWindow (widget->win);
      eraseCursesWindow (widget->shadowWin);
   }
}

dummyInject (Histogram)

dummyFocus (Histogram)

dummyUnfocus (Histogram)

dummyRefreshData (Histogram)

dummySaveData (Histogram)
