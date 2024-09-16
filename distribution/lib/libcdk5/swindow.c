#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 01:09:14 $
 * $Revision: 1.127 $
 */

/*
 * Declare file local prototypes.
 */
static int createList (CDKSWINDOW *swindow, int listSize);
static void drawCDKSwindowList (CDKSWINDOW *swindow, boolean Box);

DeclareCDKObjects (SWINDOW, Swindow, setCdk, Int);

/*
 * This function creates a scrolling window widget.
 */
CDKSWINDOW *newCDKSwindow (CDKSCREEN *cdkscreen,
			   int xplace,
			   int yplace,
			   int height,
			   int width,
			   const char *title,
			   int saveLines,
			   boolean Box,
			   boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKSWINDOW *swindow          = 0;
   int parentWidth              = getmaxx (cdkscreen->window);
   int parentHeight             = getmaxy (cdkscreen->window);
   int boxWidth;
   int boxHeight;
   int xpos                     = xplace;
   int ypos                     = yplace;
   int x;
   /* *INDENT-OFF* */
   static const struct { int from; int to; } bindings[] = {
	    { CDK_BACKCHAR,	KEY_PPAGE },
	    { 'b',		KEY_PPAGE },
	    { 'B',		KEY_PPAGE },
	    { CDK_FORCHAR,	KEY_NPAGE },
	    { SPACE,		KEY_NPAGE },
	    { 'f',		KEY_NPAGE },
	    { 'F',		KEY_NPAGE },
	    { '|',		KEY_HOME },
	    { '$',		KEY_END },
   };
   /* *INDENT-ON* */

   if ((swindow = newCDKObject (CDKSWINDOW, &my_funcs)) == 0)
        return (0);

   setCDKSwindowBox (swindow, Box);

   /*
    * If the height is a negative value, the height will
    * be ROWS-height, otherwise, the height will be the
    * given height.
    */
   boxHeight = setWidgetDimension (parentHeight, height, 0);

   /*
    * If the width is a negative value, the width will
    * be COLS-width, otherwise, the width will be the
    * given width.
    */
   boxWidth = setWidgetDimension (parentWidth, width, 0);

   boxWidth = setCdkTitle (ObjOf (swindow), title, boxWidth);

   /* Set the box height. */
   boxHeight += TitleLinesOf (swindow) + 1;

   /*
    * Make sure we didn't extend beyond the dimensions of the window.
    */
   boxWidth = (boxWidth > parentWidth ? parentWidth : boxWidth);
   boxHeight = (boxHeight > parentHeight ? parentHeight : boxHeight);

   /* Set the rest of the variables. */
   swindow->titleAdj = TitleLinesOf (swindow) + 1;

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* Make the scrolling window */
   swindow->win = newwin (boxHeight, boxWidth, ypos, xpos);
   if (swindow->win == 0)
   {
      destroyCDKObject (swindow);
      return (0);
   }
   keypad (swindow->win, TRUE);

   /* Make the field window. */
   swindow->fieldWin = subwin (swindow->win,
			       (boxHeight - TitleLinesOf (swindow) - 2),
			       boxWidth - 2,
			       ypos + TitleLinesOf (swindow) + 1,
			       xpos + 1);
   keypad (swindow->fieldWin, TRUE);

   /* *INDENT-EQLS* Set the rest of the variables */
   ScreenOf (swindow)           = cdkscreen;
   swindow->parent              = cdkscreen->window;
   swindow->shadowWin           = 0;
   swindow->boxHeight           = boxHeight;
   swindow->boxWidth            = boxWidth;
   swindow->viewSize            = boxHeight - TitleLinesOf (swindow) - 2;
   swindow->currentTop          = 0;
   swindow->maxTopLine          = 0;
   swindow->leftChar            = 0;
   swindow->maxLeftChar         = 0;
   swindow->listSize            = 0;
   swindow->widestLine          = -1;
   swindow->saveLines           = saveLines;
   initExitType (swindow);
   ObjOf (swindow)->acceptsFocus = TRUE;
   ObjOf (swindow)->inputWindow = swindow->win;
   swindow->shadow              = shadow;

   if (!createList (swindow, saveLines))
   {
      destroyCDKObject (swindow);
      return (0);
   }

   /* Do we need to create a shadow??? */
   if (shadow)
   {
      swindow->shadowWin = newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   /* Clean the key bindings. */
   for (x = 0; x < (int)SIZEOF (bindings); ++x)
      bindCDKObject (vSWINDOW,
		     swindow,
		     (chtype)bindings[x].from,
		     getcCDKBind,
		     (void *)(long)bindings[x].to);

   /* Register this baby. */
   registerCDKObject (cdkscreen, vSWINDOW, swindow);

   /* Return the scrolling window */
   return (swindow);
}

/*
 * This sets the lines and the box attribute of the scrolling window.
 */
void setCDKSwindow (CDKSWINDOW *swindow, CDK_CSTRING2 list, int lines, boolean Box)
{
   setCDKSwindowContents (swindow, list, lines);
   setCDKSwindowBox (swindow, Box);
}

static void setupLine (CDKSWINDOW *swindow, const char *list, int x)
{
   /* *INDENT-EQLS* */
   swindow->list[x]    = char2Chtype (list,
				      &swindow->listLen[x],
				      &swindow->listPos[x]);
   swindow->listPos[x] = justifyString (swindow->boxWidth,
					swindow->listLen[x],
					swindow->listPos[x]);
   swindow->widestLine = MAXIMUM (swindow->widestLine, swindow->listLen[x]);
}

/*
 * This sets all the lines inside the scrolling window.
 */
void setCDKSwindowContents (CDKSWINDOW *swindow, CDK_CSTRING2 list, int listSize)
{
   int x = 0;

   /* First lets clean all the lines in the window. */
   cleanCDKSwindow (swindow);
   createList (swindow, listSize);

   /* Now lets set all the lines inside the window. */
   for (x = 0; x < listSize; x++)
   {
      setupLine (swindow, list[x], x);
   }

   /* *INDENT-EQLS* Set some of the more important members of the scrolling window. */
   swindow->listSize    = listSize;
   swindow->maxTopLine  = swindow->listSize - swindow->viewSize;
   swindow->maxTopLine  = (swindow->maxTopLine < 0 ? 0 : swindow->maxTopLine);
   swindow->maxLeftChar = swindow->widestLine - (swindow->boxWidth - 2);
   swindow->currentTop  = 0;
   swindow->leftChar    = 0;
}
chtype **getCDKSwindowContents (CDKSWINDOW *swindow, int *size)
{
   (*size)              = swindow->listSize;
   return swindow->list;
}

/*
 * This sets the box attribute for the widget.
 */
void setCDKSwindowBox (CDKSWINDOW *swindow, boolean Box)
{
   ObjOf (swindow)->box = Box;
   ObjOf (swindow)->borderSize = Box ? 1 : 0;
}
boolean getCDKSwindowBox (CDKSWINDOW *swindow)
{
   return ObjOf (swindow)->box;
}

static void freeLine (CDKSWINDOW *swindow, int x)
{
   if (x < swindow->listSize)
   {
      freeChtype (swindow->list[x]);
      swindow->list[x] = 0;
   }
}

/*
 * This adds a line to the scrolling window.
 */
void addCDKSwindow (CDKSWINDOW *swindow, const char *list, int insertPos)
{
   int x = 0;

   /*
    * If we are at the maximum number of save lines. Erase
    * the first position and bump everything up one spot.
    */
   if (swindow->listSize == swindow->saveLines)
   {
      /* Free up the memory. */
      freeLine (swindow, 0);

      /* Bump everything up one spot. */
      for (x = 0; x < swindow->listSize; x++)
      {
	 /* *INDENT-EQLS* */
	 swindow->list[x]  = swindow->list[x + 1];
	 swindow->listPos[x] = swindow->listPos[x + 1];
	 swindow->listLen[x] = swindow->listLen[x + 1];
      }

      /* *INDENT-EQLS* Clean out the last position. */
      swindow->list[swindow->listSize]    = 0;
      swindow->listLen[swindow->listSize] = 0;
      swindow->listPos[swindow->listSize] = 0;
      swindow->listSize--;
   }

   /* Determine where the line is being added. */
   if (insertPos == TOP)
   {
      /* We need to 'bump' everything down one line... */
      for (x = swindow->listSize; x > 0; x--)
      {
	 /* *INDENT-EQLS* Copy in the new row. */
	 swindow->list[x]  = swindow->list[x - 1];
	 swindow->listPos[x] = swindow->listPos[x - 1];
	 swindow->listLen[x] = swindow->listLen[x - 1];
      }

      /* Add it into the scrolling window. */
      setupLine (swindow, list, 0);

      /* Set some variables. */
      swindow->currentTop = 0;
      if (swindow->listSize < swindow->saveLines)
      {
	 swindow->listSize++;
      }

      /* Set the maximum top line. */
      swindow->maxTopLine = swindow->listSize - swindow->viewSize;
      swindow->maxTopLine = (swindow->maxTopLine < 0 ? 0 : swindow->maxTopLine);

      swindow->maxLeftChar = swindow->widestLine - (swindow->boxWidth - 2);
   }
   else
   {
      /* Add to the bottom. */
      setupLine (swindow, list, swindow->listSize);

      swindow->maxLeftChar = swindow->widestLine - (swindow->boxWidth - 2);

      /* Increment the item count and zero out the next row. */
      if (swindow->listSize < swindow->saveLines)
      {
	 swindow->listSize++;
	 freeLine (swindow, swindow->listSize);
      }

      /* Set the maximum top line. */
      if (swindow->listSize <= swindow->viewSize)
      {
	 swindow->maxTopLine = 0;
	 swindow->currentTop = 0;
      }
      else
      {
	 swindow->maxTopLine = (swindow->listSize - swindow->viewSize);
	 swindow->currentTop = swindow->maxTopLine;
      }
   }

   /* Draw in the list. */
   drawCDKSwindowList (swindow, ObjOf (swindow)->box);
}

/*
 * This jumps to a given line.
 */
void jumpToLineCDKSwindow (CDKSWINDOW *swindow, int line)
{
   /*
    * Make sure the line is in bounds.
    */
   if (line == BOTTOM || line >= swindow->listSize)
   {
      /* We are moving to the last page. */
      swindow->currentTop = swindow->listSize - swindow->viewSize;
   }
   else if (line == TOP || line <= 0)
   {
      /* We are moving to the top of the page. */
      swindow->currentTop = 0;
   }
   else
   {
      /* We are moving in the middle somewhere. */
      if ((swindow->viewSize + line) < swindow->listSize)
      {
	 swindow->currentTop = line;
      }
      else
      {
	 swindow->currentTop = swindow->listSize - swindow->viewSize;
      }
   }

   /* A little sanity check to make we don't something silly. */
   if (swindow->currentTop < 0)
   {
      swindow->currentTop = 0;
   }

   /* Redraw the window. */
   drawCDKSwindow (swindow, ObjOf (swindow)->box);
}

/*
 * This removes all the lines inside the scrolling window.
 */
void cleanCDKSwindow (CDKSWINDOW *swindow)
{
   int x;

   /* Clean up the memory used ... */
   for (x = 0; x < swindow->listSize; x++)
   {
      freeLine (swindow, x);
   }

   /* *INDENT-EQLS* Reset some variables. */
   swindow->listSize    = 0;
   swindow->maxLeftChar = 0;
   swindow->widestLine  = 0;
   swindow->currentTop  = 0;
   swindow->maxTopLine  = 0;

   /* Redraw the window. */
   drawCDKSwindow (swindow, ObjOf (swindow)->box);
}

/*
 * This trims lines from the scrolling window.
 */
void trimCDKSwindow (CDKSWINDOW *swindow, int begin, int end)
{
   int start, finish, lines, x;

   /* 
    * Do nothing if the list is empty, the interval is empty,
    * or the entire interval lies outside of the list.
    */
   if ((swindow->listSize == 0) ||
       (begin > end) ||
       (begin < 0 && end < 0) ||
       (begin >= swindow->listSize && end >= swindow->listSize))
   {
      return;
   }

   /* Check the value of begin. */
   if (begin < 0)
   {
      start = 0;
   }
   else if (begin >= swindow->listSize)
   {
      start = swindow->listSize - 1;
   }
   else
   {
      start = begin;
   }

   /* Check the value of end. */
   if (end < 0)
   {
      finish = 0;
   }
   else if (end >= swindow->listSize)
   {
      finish = swindow->listSize - 1;
   }
   else
   {
      finish = end;
   }

   lines = finish - start + 1;

   /* Start nuking elements from the window. */
   for (x = start; x <= finish; x++)
   {
      freeLine (swindow, x);
   }

   /* Move the lines after the trimmed lines up. */
   for (x = start; x < swindow->listSize - lines; x++)
   {
      /* Move the line up. */
      swindow->list[x] = swindow->list[x + lines];
      swindow->listPos[x] = swindow->listPos[x + lines];
      swindow->listLen[x] = swindow->listLen[x + lines];

      /* Zero out the moved line's original entries. */
      swindow->list[x + lines] = 0;
      swindow->listPos[x + lines] = 0;
      swindow->listLen[x + lines] = 0;
   }

   /* Adjust the item count correctly. */
   swindow->listSize = swindow->listSize - lines;

   /* Set the maximum top line. */
   if (swindow->listSize <= swindow->viewSize)
   {
      swindow->maxTopLine = 0;
   }
   else
   {
      swindow->maxTopLine = (swindow->listSize - swindow->viewSize);
   }

   /* Set the current top line, but only if it is no longer valid. */
   if (swindow->currentTop > swindow->maxTopLine)
   {
      swindow->currentTop = swindow->maxTopLine;
   }

   /* Redraw the list. */
   drawCDKSwindowList (swindow, ObjOf (swindow)->box);
}

/*
 * This allows the user to play inside the scrolling window.
 */
void activateCDKSwindow (CDKSWINDOW *swindow, chtype *actions)
{
   /* Draw the scrolling list */
   drawCDKSwindow (swindow, ObjOf (swindow)->box);

   if (actions == 0)
   {
      boolean functionKey;

      for (;;)
      {
	 chtype input = (chtype)getchCDKObject (ObjOf (swindow), &functionKey);

	 /* Inject the character into the widget. */
	 (void)injectCDKSwindow (swindow, input);
	 if (swindow->exitType != vEARLY_EXIT)
	 {
	    return;
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
	 (void)injectCDKSwindow (swindow, actions[x]);
	 if (swindow->exitType != vEARLY_EXIT)
	 {
	    return;
	 }
      }
   }

   /* Set the exit type and return. */
   setExitType (swindow, 0);
   return;
}

/*
 * This injects a single character into the widget.
 */
static int _injectCDKSwindow (CDKOBJS *object, chtype input)
{
   /* *INDENT-EQLS* */
   CDKSWINDOW *widget = (CDKSWINDOW *)object;
   int ppReturn       = 1;
   int ret            = unknownInt;
   bool complete      = FALSE;

   /* Set the exit type. */
   setExitType (widget, 0);

   /* Draw the window.... */
   drawCDKSwindow (widget, ObjOf (widget)->box);

   /* Check if there is a pre-process function to be called. */
   if (PreProcessFuncOf (widget) != 0)
   {
      /* Call the pre-process function. */
      ppReturn = PreProcessFuncOf (widget) (vSWINDOW,
					    widget,
					    PreProcessDataOf (widget),
					    input);
   }

   /* Should we continue? */
   if (ppReturn != 0)
   {
      /* Check for a key binding. */
      if (checkCDKObjectBind (vSWINDOW, widget, input) != 0)
      {
	 checkEarlyExit (widget);
	 complete = TRUE;
      }
      else
      {
	 switch (input)
	 {
	 case KEY_UP:
	    if (widget->currentTop > 0)
	    {
	       widget->currentTop--;
	    }
	    else
	    {
	       Beep ();
	    }
	    break;

	 case KEY_DOWN:
	    if (widget->currentTop >= 0 && widget->currentTop < widget->maxTopLine)
	    {
	       widget->currentTop++;
	    }
	    else
	    {
	       Beep ();
	    }
	    break;

	 case KEY_RIGHT:
	    if (widget->leftChar < widget->maxLeftChar)
	    {
	       widget->leftChar++;
	    }
	    else
	    {
	       Beep ();
	    }
	    break;

	 case KEY_LEFT:
	    if (widget->leftChar > 0)
	    {
	       widget->leftChar--;
	    }
	    else
	    {
	       Beep ();
	    }
	    break;

	 case KEY_PPAGE:
	    if (widget->currentTop != 0)
	    {
	       if (widget->currentTop >= widget->viewSize)
	       {
		  widget->currentTop = (widget->currentTop
					- (widget->viewSize - 1));
	       }
	       else
	       {
		  widget->currentTop = 0;
	       }
	    }
	    else
	    {
	       Beep ();
	    }
	    break;

	 case KEY_NPAGE:
	    if (widget->currentTop != widget->maxTopLine)
	    {
	       if ((widget->currentTop + widget->viewSize) < widget->maxTopLine)
	       {
		  widget->currentTop = (widget->currentTop
					+ (widget->viewSize - 1));
	       }
	       else
	       {
		  widget->currentTop = widget->maxTopLine;
	       }
	    }
	    else
	    {
	       Beep ();
	    }
	    break;

	 case KEY_HOME:
	    widget->leftChar = 0;
	    break;

	 case KEY_END:
	    widget->leftChar = widget->maxLeftChar + 1;
	    break;

	 case 'g':
	 case '1':
	 case '<':
	    widget->currentTop = 0;
	    break;

	 case 'G':
	 case '>':
	    widget->currentTop = widget->maxTopLine;
	    break;

	 case 'l':
	 case 'L':
	    loadCDKSwindowInformation (widget);
	    break;

	 case 's':
	 case 'S':
	    saveCDKSwindowInformation (widget);
	    break;

	 case KEY_TAB:
	 case KEY_ENTER:
	    setExitType (widget, input);
	    ret = 1;
	    complete = TRUE;
	    break;

	 case KEY_ESC:
	    setExitType (widget, input);
	    complete = TRUE;
	    break;

	 case KEY_ERROR:
	    setExitType (widget, input);
	    complete = TRUE;
	    break;

	 case CDK_REFRESH:
	    eraseCDKScreen (ScreenOf (widget));
	    refreshCDKScreen (ScreenOf (widget));
	    break;

	 default:
	    break;
	 }
      }

      /* Should we call a post-process? */
      if (!complete && (PostProcessFuncOf (widget) != 0))
      {
	 PostProcessFuncOf (widget) (vSWINDOW,
				     widget,
				     PostProcessDataOf (widget),
				     input);
      }
   }

   if (!complete)
   {
      drawCDKSwindowList (widget, ObjOf (widget)->box);
      setExitType (widget, 0);
   }

   ResultOf (widget).valueInt = ret;
   return (ret != unknownInt);
}

/*
 * This moves the swindow field to the given location.
 */
static void _moveCDKSwindow (CDKOBJS *object,
			     int xplace,
			     int yplace,
			     boolean relative,
			     boolean refresh_flag)
{
   CDKSWINDOW *swindow = (CDKSWINDOW *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx (swindow->win);
   int currentY = getbegy (swindow->win);
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
      xpos = getbegx (swindow->win) + xplace;
      ypos = getbegy (swindow->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (swindow), &xpos, &ypos, swindow->boxWidth, swindow->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (swindow->win, -xdiff, -ydiff);
   moveCursesWindow (swindow->shadowWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (swindow));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKSwindow (swindow, ObjOf (swindow)->box);
   }
}

/*
 * This function draws the swindow window widget.
 */
static void _drawCDKSwindow (CDKOBJS *object, boolean Box)
{
   CDKSWINDOW *swindow = (CDKSWINDOW *)object;

   /* Do we need to draw in the shadow. */
   if (swindow->shadowWin != 0)
   {
      drawShadow (swindow->shadowWin);
   }

   /* Box the widget if needed */
   if (Box)
   {
      drawObjBox (swindow->win, ObjOf (swindow));
   }

   drawCdkTitle (swindow->win, object);

   wrefresh (swindow->win);

   /* Draw in the list. */
   drawCDKSwindowList (swindow, Box);
}

/*
 * This draws in the contents of the scrolling window.
 */
static void drawCDKSwindowList (CDKSWINDOW *swindow, boolean Box GCC_UNUSED)
{
   int lastLine, x;

   /* Determine the last line to draw. */
   if (swindow->listSize < swindow->viewSize)
   {
      lastLine = swindow->listSize;
   }
   else
   {
      lastLine = swindow->viewSize;
   }

   /* Erase the scrolling window. */
   werase (swindow->fieldWin);

   /* Start drawing in each line. */
   for (x = 0; x < lastLine; x++)
   {
      int screenPos;

      if ((x + swindow->currentTop) >= swindow->listSize)
	 break;

      screenPos = swindow->listPos[x + swindow->currentTop] - swindow->leftChar;

      /* Write in the correct line. */
      if (screenPos >= 0)
      {
	 writeChtype (swindow->fieldWin, screenPos, x,
		      swindow->list[x + swindow->currentTop],
		      HORIZONTAL, 0,
		      swindow->listLen[x + swindow->currentTop]);
      }
      else
      {
	 writeChtype (swindow->fieldWin, 0, x,
		      swindow->list[x + swindow->currentTop],
		      HORIZONTAL,
		      swindow->leftChar - swindow->listPos[x + swindow->currentTop],
		      swindow->listLen[x + swindow->currentTop]);
      }
   }

   wrefresh (swindow->fieldWin);
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattrSwindow (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKSWINDOW *widget = (CDKSWINDOW *)object;

      wbkgd (widget->win, attrib);
      wbkgd (widget->fieldWin, attrib);
   }
}

/*
 * Free any storage associated with the info-list.
 */
static void destroyInfo (CDKSWINDOW *swindow)
{
   CDKfreeChtypes (swindow->list);
   freeChecked (swindow->listPos);
   freeChecked (swindow->listLen);

   swindow->list = 0;
   swindow->listPos = 0;
   swindow->listLen = 0;
}

/*
 * This function destroys the scrolling window widget.
 */
static void _destroyCDKSwindow (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKSWINDOW *swindow = (CDKSWINDOW *)object;

      destroyInfo (swindow);

      cleanCdkTitle (object);

      /* Delete the windows. */
      deleteCursesWindow (swindow->shadowWin);
      deleteCursesWindow (swindow->fieldWin);
      deleteCursesWindow (swindow->win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vSWINDOW, swindow);

      /* Unregister this object. */
      unregisterCDKObject (vSWINDOW, swindow);
   }
}

/*
 * This function erases the scrolling window widget.
 */
static void _eraseCDKSwindow (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKSWINDOW *swindow = (CDKSWINDOW *)object;

      eraseCursesWindow (swindow->win);
      eraseCursesWindow (swindow->shadowWin);
   }
}

/*
 * This exec's a command and redirects the output to the scrolling window.
 */
int execCDKSwindow (CDKSWINDOW *swindow, const char *command, int insertPos)
{
   FILE *ps;
   int count = -1;

   endwin ();
   /* Try to open the command. */
   if ((ps = popen (command, "r")) != 0)
   {
      char temp[BUFSIZ];

      /* Start reading. */
      while (fgets (temp, sizeof (temp), ps) != 0)
      {
	 size_t len = strlen (temp);
	 if (len != 0 && temp[len - 1] == '\n')
	    temp[--len] = '\0';
	 /* Add the line to the scrolling window. */
	 addCDKSwindow (swindow, temp, insertPos);
	 count++;
      }

      /* Close the pipe. */
      pclose (ps);
   }
   return count;
}

static void showMessage2 (CDKSWINDOW *swindow,
			  const char *msg,
			  const char *msg2,
			  const char *filename)
{
   char *mesg[10];
   char *temp = (char *)malloc (80 + strlen (filename));
   int n = 0;

   mesg[n++] = copyChar (msg);
   mesg[n++] = copyChar (msg2);
   sprintf (temp, "<C>(%s)", filename);
   mesg[n++] = copyChar (temp);
   mesg[n++] = copyChar (" ");
   mesg[n++] = copyChar ("<C>Press any key to continue.");
   popupLabel (ScreenOf (swindow), (CDK_CSTRING2)mesg, n);
   freeCharList (mesg, (unsigned)n);
   free (temp);
}

/*
 * This function allows the user to dump the information from the
 * scrolling window to a file.
 */
void saveCDKSwindowInformation (CDKSWINDOW *swindow)
{
   CDKENTRY *entry = 0;
   char *filename = 0;
   int linesSaved;

   /* Create the entry field to get the filename. */
   entry = newCDKEntry (ScreenOf (swindow), CENTER, CENTER,
			"<C></B/5>Enter the filename of the save file.",
			"Filename: ",
			A_NORMAL, '_', vMIXED,
			20, 1, 256,
			TRUE, FALSE);

   /* Get the filename. */
   filename = activateCDKEntry (entry, 0);

   /* Did they hit escape? */
   if (entry->exitType == vESCAPE_HIT)
   {
      const char *mesg[10];

      /* Popup a message. */
      mesg[0] = "<C></B/5>Save Canceled.";
      mesg[1] = "<C>Escape hit. Scrolling window information not saved.";
      mesg[2] = " ";
      mesg[3] = "<C>Press any key to continue.";
      popupLabel (ScreenOf (swindow), (CDK_CSTRING2)mesg, 4);

      /* Clean up and exit. */
      destroyCDKEntry (entry);
      return;
   }

   /* Write the contents of the scrolling window to the file. */
   linesSaved = dumpCDKSwindow (swindow, filename);

   /* Was the save successful? */
   if (linesSaved == -1)
   {
      /* Nope, tell 'em. */
      showMessage2 (swindow,
		    "<C></B/16>Error",
		    "<C>Could not save to the file.",
		    filename);
   }
   else
   {
      char temp[256];

      /* Yep, let them know how many lines were saved. */
      sprintf (temp, "<C>There were %d lines saved to the file", linesSaved);
      showMessage2 (swindow,
		    "<C></B/5>Save Successful",
		    temp,
		    filename);
   }

   /* Clean up and exit. */
   destroyCDKEntry (entry);
   eraseCDKScreen (ScreenOf (swindow));
   drawCDKScreen (ScreenOf (swindow));
}

/*
 * This function allows the user to load new information into the scrolling
 * window.
 */
void loadCDKSwindowInformation (CDKSWINDOW *swindow)
{
   /* *INDENT-EQLS* */
   CDKFSELECT *fselect  = 0;
   char *filename       = 0;
   const char *mesg[15];
   char **fileInfo      = 0;
   int lines;

   /* Create the file selector to choose the file. */
   fselect = newCDKFselect (ScreenOf (swindow), CENTER, CENTER, 20, 55,
			    "<C>Load Which File",
			    "Filename",
			    A_NORMAL, '.',
			    A_REVERSE,
			    "</5>", "</48>", "</N>", "</N>",
			    TRUE, FALSE);

   /* Get the filename to load. */
   (void)activateCDKFselect (fselect, 0);

   /* Make sure they selected a file. */
   if (fselect->exitType == vESCAPE_HIT)
   {
      /* Popup a message. */
      mesg[0] = "<C></B/5>Load Canceled.";
      mesg[1] = " ";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (ScreenOf (swindow), (CDK_CSTRING2)mesg, 3);

      /* Clean up and exit. */
      destroyCDKFselect (fselect);
      return;
   }

   /* Copy the filename and destroy the file selector. */
   filename = copyChar (fselect->pathname);
   destroyCDKFselect (fselect);

   /*
    * Maye we should check before nuking all the information
    * in the scrolling window...
    */
   if (swindow->listSize > 0)
   {
      CDKDIALOG *dialog = 0;
      const char *button[5];
      int answer;

      /* Create the dialog message. */
      mesg[0] = "<C></B/5>Save Information First";
      mesg[1] = "<C>There is information in the scrolling window.";
      mesg[2] = "<C>Do you want to save it to a file first?";
      button[0] = "(Yes)";
      button[1] = "(No)";

      /* Create the dialog widget. */
      dialog = newCDKDialog (ScreenOf (swindow), CENTER, CENTER,
			     (CDK_CSTRING2)mesg, 3,
			     (CDK_CSTRING2)button, 2,
			     COLOR_PAIR (2) | A_REVERSE,
			     TRUE, TRUE, FALSE);

      /* Activate the widget. */
      answer = activateCDKDialog (dialog, 0);
      destroyCDKDialog (dialog);

      /* Check the answer. */
      if (answer == -1 || answer == 0)
      {
	 /* Save the information. */
	 saveCDKSwindowInformation (swindow);
      }
   }

   /* Open the file and read it in. */
   lines = CDKreadFile (filename, &fileInfo);
   if (lines == -1)
   {
      /* The file read didn't work. */
      showMessage2 (swindow,
		    "<C></B/16>Error",
		    "<C>Could not read the file",
		    filename);
      freeChar (filename);
      return;
   }

   /* Clean out the scrolling window. */
   cleanCDKSwindow (swindow);

   /* Set the new information in the scrolling window. */
   setCDKSwindow (swindow, (CDK_CSTRING2)fileInfo, lines, ObjOf (swindow)->box);

   /* Clean up. */
   CDKfreeStrings (fileInfo);
   freeChar (filename);
}

/*
 * This actually dumps the information from the scrolling window to a
 * file.
 */
int dumpCDKSwindow (CDKSWINDOW *swindow, const char *filename)
{
   FILE *outputFile = 0;
   int x;

   /* Try to open the file. */
   if ((outputFile = fopen (filename, "w")) == 0)
   {
      return -1;
   }

   /* Start writing out the file. */
   for (x = 0; x < swindow->listSize; x++)
   {
      char *rawLine = chtype2Char (swindow->list[x]);
      fprintf (outputFile, "%s\n", rawLine);
      freeChar (rawLine);
   }

   /* Close the file and return the number of lines written. */
   fclose (outputFile);
   return swindow->listSize;
}

static void _focusCDKSwindow (CDKOBJS *object)
{
   CDKSWINDOW *widget = (CDKSWINDOW *)object;

   drawCDKSwindow (widget, ObjOf (widget)->box);
}

static void _unfocusCDKSwindow (CDKOBJS *object)
{
   CDKSWINDOW *widget = (CDKSWINDOW *)object;

   drawCDKSwindow (widget, ObjOf (widget)->box);
}

static int createList (CDKSWINDOW *swindow, int listSize)
{
   int status = 0;

   if (listSize >= 0)
   {
      chtype **newList = typeCallocN (chtype *, listSize + 1);
      int *newPos = typeCallocN (int, listSize + 1);
      int *newLen = typeCallocN (int, listSize + 1);

      if (newList != 0
	  && newPos != 0
	  && newLen != 0)
      {
	 status = 1;
	 destroyInfo (swindow);

	 /* *INDENT-EQLS* */
	 swindow->list  = newList;
	 swindow->listPos = newPos;
	 swindow->listLen = newLen;
      }
      if (!status)
      {
	 CDKfreeChtypes (newList);
	 freeChecked (newPos);
	 freeChecked (newLen);
      }
   }
   else
   {
      destroyInfo (swindow);
      status         = 1;
   }
   return status;
}

dummyRefreshData (Swindow)

dummySaveData (Swindow)
