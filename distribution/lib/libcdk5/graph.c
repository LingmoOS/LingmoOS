#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2013/06/16 15:05:27 $
 * $Revision: 1.86 $
 */

DeclareCDKObjects (GRAPH, Graph, setCdk, Unknown);

#define TITLE_LM 3

/*
 * Create a graph widget.
 */
CDKGRAPH *newCDKGraph (CDKSCREEN *cdkscreen,
		       int xplace,
		       int yplace,
		       int height,
		       int width,
		       const char *title,
		       const char *xtitle,
		       const char *ytitle)
{
   /* *INDENT-EQLS* */
   CDKGRAPH *widget     = 0;
   int parentWidth      = getmaxx (cdkscreen->window);
   int parentHeight     = getmaxy (cdkscreen->window);
   int boxWidth;
   int boxHeight;
   int xpos             = xplace;
   int ypos             = yplace;

   if ((widget = newCDKObject (CDKGRAPH, &my_funcs)) == 0)
        return (0);

   setCDKGraphBox (widget, FALSE);

   /* *INDENT-EQLS* */
   boxHeight = setWidgetDimension (parentHeight, height, 3);
   boxWidth  = setWidgetDimension (parentWidth, width, 0);
   boxWidth  = setCdkTitle (ObjOf (widget), title, boxWidth);
   boxHeight += TitleLinesOf (widget);
   boxWidth  = MINIMUM (boxWidth, parentWidth);
   boxHeight = MINIMUM (boxHeight, parentHeight);

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* *INDENT-EQLS* Create the widget pointer. */
   ScreenOf (widget)    = cdkscreen;
   widget->parent       = cdkscreen->window;
   widget->win          = newwin (boxHeight, boxWidth, ypos, xpos);
   widget->boxHeight    = boxHeight;
   widget->boxWidth     = boxWidth;
   widget->minx         = 0;
   widget->maxx         = 0;
   widget->xscale       = 0;
   widget->yscale       = 0;
   widget->count        = 0;
   widget->displayType  = vLINE;

   if (widget->win == 0)
   {
      destroyCDKObject (widget);
      return (0);
   }
   keypad (widget->win, TRUE);

   /* Translate the X Axis title char * to a chtype * */
   if (xtitle != 0)
   {
      widget->xtitle = char2Chtype (xtitle, &widget->xtitleLen, &widget->xtitlePos);
      widget->xtitlePos = justifyString (widget->boxHeight,
					 widget->xtitleLen,
					 widget->xtitlePos);
   }
   else
   {
      widget->xtitle = char2Chtype ("<C></5>X Axis", &widget->xtitleLen, &widget->xtitlePos);
      widget->xtitlePos = justifyString (widget->boxHeight,
					 widget->xtitleLen,
					 widget->xtitlePos);
   }

   /* Translate the Y Axis title char * to a chtype * */
   if (ytitle != 0)
   {
      widget->ytitle = char2Chtype (ytitle, &widget->ytitleLen, &widget->ytitlePos);
      widget->ytitlePos = justifyString (widget->boxWidth,
					 widget->ytitleLen,
					 widget->ytitlePos);
   }
   else
   {
      widget->ytitle = char2Chtype ("<C></5>Y Axis", &widget->ytitleLen, &widget->ytitlePos);
      widget->ytitlePos = justifyString (widget->boxWidth,
					 widget->ytitleLen,
					 widget->ytitlePos);
   }

   widget->graphChar = 0;

   registerCDKObject (cdkscreen, vGRAPH, widget);

   return (widget);
}

/*
 * This was added for the builder.
 */
void activateCDKGraph (CDKGRAPH *widget, chtype *actions GCC_UNUSED)
{
   drawCDKGraph (widget, ObjOf (widget)->box);
}

/*
 * Set multiple attributes of the widget.
 */
int setCDKGraph (CDKGRAPH *widget,
		 int *values,
		 int count,
		 const char *graphChar,
		 boolean startAtZero,
		 EGraphDisplayType displayType)
{
   int ret;

   ret = setCDKGraphValues (widget, values, count, startAtZero);
   setCDKGraphCharacters (widget, graphChar);
   setCDKGraphDisplayType (widget, displayType);
   return ret;
}

/*
 * Set the scale factors for the graph after we have loaded new values.
 */
static void setScales (CDKGRAPH *widget)
{
   widget->xscale = ((widget->maxx - widget->minx)
		     / MAXIMUM (1, (widget->boxHeight
				    - TitleLinesOf (widget)
				    - 5)));
   if (widget->xscale <= 0)
      widget->xscale = 1;

   widget->yscale = ((widget->boxWidth - 4) / MAXIMUM (1, widget->count));
   if (widget->yscale <= 0)
      widget->yscale = 1;
}

/*
 * Set the values of the graph.
 */
int setCDKGraphValues (CDKGRAPH *widget, int *values, int count, boolean startAtZero)
{
   int min = INT_MAX;
   int max = INT_MIN;
   int x;

   /* Make sure everything is happy. */
   if (count < 0)
      return (FALSE);

   if (widget->values != 0)
   {
      free (widget->values);
      widget->values = 0;
      widget->count = 0;
   }
   if ((widget->values = typeCallocN (int, count + 1)) == 0)
        return FALSE;

   /* Copy the X values. */
   for (x = 0; x < count; x++)
   {
      /* Determine the min/max values of the graph. */
      min = MINIMUM (values[x], widget->minx);
      max = MAXIMUM (values[x], widget->maxx);

      /* Copy the value. */
      widget->values[x] = values[x];
   }

   /* Keep the count and min/max values. */
   widget->count = count;
   widget->minx = min;
   widget->maxx = max;

   /* Check the start at zero status. */
   if (startAtZero)
   {
      widget->minx = 0;
   }

   setScales (widget);

   return (TRUE);
}
int *getCDKGraphValues (CDKGRAPH *widget, int *size)
{
   (*size) = widget->count;
   return widget->values;
}

/*
 * Set the value of the graph at the given index.
 */
int setCDKGraphValue (CDKGRAPH *widget, int Index, int value, boolean startAtZero)
{
   /* Make sure the index is within range. */
   if (Index < 0 || Index >= widget->count)
   {
      return (FALSE);
   }

   /* Set the min, max, and value for the graph. */
   widget->minx = MINIMUM (value, widget->minx);
   widget->maxx = MAXIMUM (value, widget->maxx);
   widget->values[Index] = value;

   /* Check the start at zero status. */
   if (startAtZero)
   {
      widget->minx = 0;
   }

   setScales (widget);

   return (TRUE);
}
int getCDKGraphValue (CDKGRAPH *widget, int Index)
{
   return Index >= 0 && Index < widget->count ? widget->values[Index] : 0;
}

/*
 * Set the characters of the graph widget.
 */
int setCDKGraphCharacters (CDKGRAPH *widget, const char *characters)
{
   chtype *newTokens = 0;
   int charCount, junk;

   newTokens = char2Chtype (characters, &charCount, &junk);

   if (charCount != widget->count)
   {
      freeChtype (newTokens);
      return (FALSE);
   }

   freeChtype (widget->graphChar);
   widget->graphChar = newTokens;
   return (TRUE);
}
chtype *getCDKGraphCharacters (CDKGRAPH *widget)
{
   return widget->graphChar;
}

/*
 * Set the character of the graph widget of the given index.
 */
int setCDKGraphCharacter (CDKGRAPH *widget, int Index, const char *character)
{
   chtype *newTokens = 0;
   int charCount, junk;

   /* Make sure the index is within range. */
   if (Index < 0 || Index > widget->count)
   {
      return (FALSE);
   }

   /* Convert the string given to us. */
   newTokens = char2Chtype (character, &charCount, &junk);

   /*
    * Check if the number of characters back is the same as the number
    * of elements in the list.
    */
   if (charCount != widget->count)
   {
      freeChtype (newTokens);
      return (FALSE);
   }

   /* Everything OK so far.  Set the value of the array.  */
   widget->graphChar[Index] = newTokens[0];
   freeChtype (newTokens);
   return (TRUE);
}
chtype getCDKGraphCharacter (CDKGRAPH *widget, int Index)
{
   return widget->graphChar[Index];
}

/*
 * Set the display type of the graph.
 */
void setCDKGraphDisplayType (CDKGRAPH *widget, EGraphDisplayType type)
{
   widget->displayType = type;
}
EGraphDisplayType getCDKGraphDisplayType (CDKGRAPH *widget)
{
   return widget->displayType;
}

/*
 * Set the background attribute of the widget.
 */
static void _setBKattrGraph (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKGRAPH *widget = (CDKGRAPH *)object;

      wbkgd (widget->win, attrib);
   }
}

/*
 * Move the graph field to the given location.
 */
static void _moveCDKGraph (CDKOBJS *object,
			   int xplace,
			   int yplace,
			   boolean relative,
			   boolean refresh_flag)
{
   CDKGRAPH *widget = (CDKGRAPH *)object;
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
      drawCDKGraph (widget, ObjOf (widget)->box);
   }
}

/*
 * Set whether or not the graph will be boxed.
 */
void setCDKGraphBox (CDKGRAPH *widget, boolean Box)
{
   ObjOf (widget)->box = Box;
   ObjOf (widget)->borderSize = Box ? 1 : 0;
}
boolean getCDKGraphBox (CDKGRAPH *widget)
{
   return ObjOf (widget)->box;
}

/*
 * Draw the graph widget.
 */
static void _drawCDKGraph (CDKOBJS *object, boolean Box)
{
   /* *INDENT-EQLS* */
   CDKGRAPH *widget     = (CDKGRAPH *)object;
   int adj              = 2 + (widget->xtitle == 0 ? 0 : 1);
   int spacing          = 0;
   chtype attrib        = ' ' | A_REVERSE;
   char temp[100];
   int x, y, xpos, ypos, len;

   /* Box it if needed. */
   if (Box)
   {
      drawObjBox (widget->win, ObjOf (widget));
   }

   /* Draw in the vertical axis. */
   drawLine (widget->win,
	     2,
	     TitleLinesOf (widget) + 1,
	     2,
	     widget->boxHeight - 3,
	     ACS_VLINE);

   /* Draw in the horizontal axis. */
   drawLine (widget->win,
	     3,
	     widget->boxHeight - 3,
	     widget->boxWidth,
	     widget->boxHeight - 3,
	     ACS_HLINE);

   drawCdkTitle (widget->win, object);

   /* Draw in the X axis title. */
   if (widget->xtitle != 0)
   {
      writeChtype (widget->win,
		   0,
		   widget->xtitlePos,
		   widget->xtitle,
		   VERTICAL,
		   0,
		   widget->xtitleLen);
      attrib = widget->xtitle[0] & A_ATTRIBUTES;
   }

   /* Draw in the X axis high value. */
   sprintf (temp, "%d", widget->maxx);
   len = (int)strlen (temp);
   writeCharAttrib (widget->win,
		    1,
		    TitleLinesOf (widget) + 1,
		    temp,
		    attrib,
		    VERTICAL,
		    0,
		    len);

   /* Draw in the X axis low value. */
   sprintf (temp, "%d", widget->minx);
   len = (int)strlen (temp);
   writeCharAttrib (widget->win,
		    1,
		    widget->boxHeight - 2 - len,
		    temp,
		    attrib,
		    VERTICAL,
		    0,
		    len);

   /* Draw in the Y axis title. */
   if (widget->ytitle != 0)
   {
      writeChtype (widget->win,
		   widget->ytitlePos,
		   widget->boxHeight - 1,
		   widget->ytitle,
		   HORIZONTAL,
		   0,
		   widget->ytitleLen);
      attrib = widget->ytitle[0] & A_ATTRIBUTES;
   }

   /* Draw in the Y axis high value. */
   sprintf (temp, "%d", widget->count);
   len = (int)strlen (temp);
   writeCharAttrib (widget->win,
		    widget->boxWidth - len - adj,
		    widget->boxHeight - 2,
		    temp,
		    attrib,
		    HORIZONTAL,
		    0,
		    len);

   /* Draw in the Y axis low value. */
   sprintf (temp, "0");
   writeCharAttrib (widget->win,
		    3,
		    widget->boxHeight - 2,
		    temp,
		    attrib,
		    HORIZONTAL,
		    0,
		    (int)strlen (temp));

   /* If the count is zero, then there aren't any points. */
   if (widget->count == 0)
   {
      wrefresh (widget->win);
      return;
   }
   spacing = (widget->boxWidth - TITLE_LM) / widget->count;

   /* Draw in the graph line/plot points. */
   for (y = 0; y < widget->count; y++)
   {
      int colheight = (widget->values[y] / widget->xscale) - 1;
      /* Add the marker on the Y axis. */
      (void)mvwaddch (widget->win,
		      widget->boxHeight - 3,
		      (y + 1) * spacing + adj,
		      ACS_TTEE);

      /* If this is a plot graph, all we do is draw a dot. */
      if (widget->displayType == vPLOT)
      {
	 xpos = widget->boxHeight - 4 - colheight;
	 ypos = (y + 1) * spacing + adj;
	 (void)mvwaddch (widget->win, xpos, ypos, widget->graphChar[y]);
      }
      else
      {
	 for (x = 0; x <= widget->yscale; x++)
	 {
	    xpos = widget->boxHeight - 3;
	    ypos = (y + 1) * spacing + adj;
	    drawLine (widget->win,
		      ypos,
		      xpos - colheight,
		      ypos,
		      xpos,
		      widget->graphChar[y]);
	 }
      }
   }

   /* Draw in the axis corners. */
   (void)mvwaddch (widget->win, TitleLinesOf (widget), 2, ACS_URCORNER);
   (void)mvwaddch (widget->win, widget->boxHeight - 3, 2, ACS_LLCORNER);
   (void)mvwaddch (widget->win, widget->boxHeight - 3, widget->boxWidth, ACS_URCORNER);

   /* Refresh and lets see 'er. */
   wrefresh (widget->win);
}

/*
 * Destroy the graph widget.
 */
static void _destroyCDKGraph (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKGRAPH *widget = (CDKGRAPH *)object;

      cleanCdkTitle (object);

      freeChtype (widget->xtitle);
      freeChtype (widget->ytitle);
      freeChtype (widget->graphChar);

      freeChecked (widget->values);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vGRAPH, widget);

      /* Unregister this object. */
      unregisterCDKObject (vGRAPH, widget);

      /* Clean up the windows. */
      deleteCursesWindow (widget->win);
   }
}

/*
 * Erase the graph widget from the screen.
 */
static void _eraseCDKGraph (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKGRAPH *widget = (CDKGRAPH *)object;

      eraseCursesWindow (widget->win);
   }
}

dummyInject (Graph)

dummyFocus (Graph)

dummyUnfocus (Graph)

dummyRefreshData (Graph)

dummySaveData (Graph)
