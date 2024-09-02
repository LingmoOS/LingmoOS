#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 01:08:29 $
 * $Revision: 1.170 $
 */

/*
 * Declare file local prototypes.
 */
static void CDKMentryCallBack (CDKMENTRY *mentry, chtype character);

DeclareCDKObjects (MENTRY, Mentry, setCdk, String);

/*
 * This creates a pointer to a multiple line entry widget.
 */
CDKMENTRY *newCDKMentry (CDKSCREEN *cdkscreen,
			 int xplace,
			 int yplace,
			 const char *title,
			 const char *label,
			 chtype fieldAttr,
			 chtype filler,
			 EDisplayType dispType,
			 int fWidth,
			 int fRows,
			 int logicalRows,
			 int min,
			 boolean Box,
			 boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKMENTRY *mentry    = 0;
   int parentWidth      = getmaxx (cdkscreen->window);
   int parentHeight     = getmaxy (cdkscreen->window);
   int fieldWidth       = fWidth;
   int fieldRows        = fRows;
   int boxWidth         = 0;
   int boxHeight        = 0;
   int horizontalAdjust, oldWidth;
   int xpos             = xplace;
   int ypos             = yplace;
   int junk;

   if ((mentry = newCDKObject (CDKMENTRY, &my_funcs)) == 0)
        return (0);

   setCDKMentryBox (mentry, Box);

   /*
    * If the fieldWidth is a negative value, the fieldWidth will
    * be COLS-fieldWidth, otherwise, the fieldWidth will be the
    * given width.
    */
   fieldWidth = setWidgetDimension (parentWidth, fieldWidth, 0);

   /*
    * If the fieldRows is a negative value, the fieldRows will
    * be ROWS-fieldRows, otherwise, the fieldRows will be the
    * given height.
    */
   fieldRows = setWidgetDimension (parentWidth, fieldRows, 0);
   boxHeight = fieldRows + 2;

   /* *INDENT-EQLS* Set some basic values of the mentry field. */
   mentry->label        = 0;
   mentry->labelLen     = 0;
   mentry->labelWin     = 0;

   /* We need to translate the char * label to a chtype * */
   if (label != 0)
   {
      mentry->label = char2Chtype (label, &mentry->labelLen, &junk);
   }
   boxWidth = mentry->labelLen + fieldWidth + 2;

   oldWidth = boxWidth;
   boxWidth = setCdkTitle (ObjOf (mentry), title, boxWidth);
   horizontalAdjust = (boxWidth - oldWidth) / 2;

   boxHeight += TitleLinesOf (mentry);

   /*
    * Make sure we didn't extend beyond the parent window.
    */
   boxWidth = (boxWidth > parentWidth ? parentWidth : boxWidth);
   boxHeight = (boxHeight > parentHeight ? parentHeight : boxHeight);
   fieldWidth = (fieldWidth > (boxWidth - mentry->labelLen - 2)
		 ? (boxWidth - mentry->labelLen - 2)
		 : fieldWidth);
   fieldRows = (fieldRows > (boxHeight - TitleLinesOf (mentry) - 2)
		? (boxHeight - TitleLinesOf (mentry) - 2)
		: fieldRows);

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* Make the label window. */
   mentry->win = newwin (boxHeight, boxWidth, ypos, xpos);

   /* Is the window null??? */
   if (mentry->win == 0)
   {
      destroyCDKObject (mentry);
      return (0);
   }

   /* Create the label window. */
   if (mentry->label != 0)
   {
      mentry->labelWin = subwin (mentry->win, fieldRows,
				 mentry->labelLen + 2,
				 ypos + TitleLinesOf (mentry) + 1,
				 xpos + horizontalAdjust + 1);
   }

   /* Make the field window. */
   mentry->fieldWin = subwin (mentry->win, fieldRows, fieldWidth,
			      ypos + TitleLinesOf (mentry) + 1,
			      xpos + mentry->labelLen + horizontalAdjust + 1);

   /* Turn on the keypad. */
   keypad (mentry->fieldWin, TRUE);
   keypad (mentry->win, TRUE);

   /* *INDENT-EQLS* Set up the rest of the structure. */
   mentry->parent       = cdkscreen->window;
   mentry->totalWidth   = (fieldWidth * logicalRows) + 1;

   /* Create the info char * pointer. */
   mentry->info = typeMallocN (char, mentry->totalWidth + 3);
   cleanChar (mentry->info, mentry->totalWidth + 3, '\0');

   /* *INDENT-EQLS* Set up the rest of the widget information. */
   ScreenOf (mentry)            = cdkscreen;
   mentry->shadowWin            = 0;
   mentry->fieldAttr            = fieldAttr;
   mentry->fieldWidth           = fieldWidth;
   mentry->rows                 = fieldRows;
   mentry->boxHeight            = boxHeight;
   mentry->boxWidth             = boxWidth;
   mentry->filler               = filler;
   mentry->hidden               = filler;
   ObjOf (mentry)->inputWindow  = mentry->win;
   ObjOf (mentry)->acceptsFocus = TRUE;
   mentry->currentRow           = 0;
   mentry->currentCol           = 0;
   mentry->topRow               = 0;
   mentry->shadow               = shadow;
   mentry->dispType             = dispType;
   mentry->min                  = min;
   mentry->logicalRows          = logicalRows;
   initExitType (mentry);
   mentry->callbackfn           = CDKMentryCallBack;

   /* Do we need to create a shadow. */
   if (shadow)
   {
      mentry->shadowWin = newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   /* Register this baby. */
   registerCDKObject (cdkscreen, vMENTRY, mentry);

   /* Return the pointer to the structure. */
   return (mentry);
}

/*
 * This actually manages the mentry widget...
 */
char *activateCDKMentry (CDKMENTRY *mentry, chtype *actions)
{
   boolean functionKey;
   char *ret = 0;

   /* Draw the mentry widget. */
   drawCDKMentry (mentry, ObjOf (mentry)->box);

   if (actions == 0)
   {
      for (;;)
      {
	 chtype input = (chtype)getchCDKObject (ObjOf (mentry), &functionKey);

	 /* Inject this character into the widget. */
	 ret = injectCDKMentry (mentry, input);
	 if (mentry->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }
   else
   {
      int length = chlen (actions);
      int x;

      /* Inject each character one at a time. */
      for (x = 0; x < length; x++)
      {
	 ret = injectCDKMentry (mentry, actions[x]);
	 if (mentry->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }

   /* Set the exit type and exit. */
   setExitType (mentry, 0);
   return 0;
}

static bool setTopRow (CDKMENTRY *widget, int row)
{
   if (widget->topRow != row)
   {
      widget->topRow = row;
      return TRUE;
   }
   return FALSE;
}

static bool setCurPos (CDKMENTRY *widget, int row, int col)
{
   if (widget->currentRow != row ||
       widget->currentCol != col)
   {
      widget->currentRow = row;
      widget->currentCol = col;
      return TRUE;
   }
   return FALSE;
}

static bool handle_KEY_LEFT (CDKMENTRY *mentry, bool *moved, bool *redraw)
{
   bool result = TRUE;

   if (mentry->currentCol != 0)
   {
      *moved = setCurPos (mentry,
			  mentry->currentRow,
			  mentry->currentCol - 1);
   }
   else if (mentry->currentRow == 0)
   {
      if (mentry->topRow != 0)
      {
	 *moved = setCurPos (mentry,
			     mentry->currentRow,
			     mentry->fieldWidth - 1);
	 *redraw = setTopRow (mentry, mentry->topRow - 1);
      }
   }
   else
   {
      *moved = setCurPos (mentry,
			  mentry->currentRow - 1,
			  mentry->fieldWidth - 1);
   }

   if (!*moved && !*redraw)
   {
      Beep ();
      result = FALSE;
   }
   return result;
}

static int getCursorPos (CDKMENTRY *mentry)
{
   return (((mentry->currentRow + mentry->topRow) * mentry->fieldWidth) +
	   mentry->currentCol);
}

/*
 * This injects a character into the widget.
 */
static int _injectCDKMentry (CDKOBJS *object, chtype input)
{
   CDKMENTRY *widget = (CDKMENTRY *)object;
   /* *INDENT-EQLS */
   int cursorPos = getCursorPos (widget);
   int ppReturn = 1;
   char *ret = unknownString;
   bool complete = FALSE;

   /* Set the exit type. */
   setExitType (widget, 0);

   /* Refresh the field. */
   drawCDKMentryField (widget);

   /* Check if there is a pre-process function to be called. */
   if (PreProcessFuncOf (widget) != 0)
   {
      /* Call the pre-process function. */
      ppReturn = PreProcessFuncOf (widget) (vMENTRY,
					    widget,
					    PreProcessDataOf (widget),
					    input);
   }

   /* Should we continue? */
   if (ppReturn != 0)
   {
      /* Check for a key binding... */
      if (checkCDKObjectBind (vMENTRY, widget, input) != 0)
      {
	 checkEarlyExit (widget);
	 complete = TRUE;
      }
      else
      {
	 bool moved = FALSE;
	 bool redraw = FALSE;
	 int infoLength = (int)strlen (widget->info);
	 int fieldCharacters;

	 switch (input)
	 {
	 case KEY_HOME:
	    moved = setCurPos (widget, 0, 0);
	    redraw = setTopRow (widget, 0);
	    break;

	 case KEY_END:
	    fieldCharacters = widget->rows * widget->fieldWidth;
	    if (infoLength < fieldCharacters)
	    {
	       redraw = setTopRow (widget, 0);
	       moved = setCurPos (widget,
				  infoLength / widget->fieldWidth,
				  infoLength % widget->fieldWidth);
	    }
	    else
	    {
	       redraw = setTopRow (widget,
				   (infoLength / widget->fieldWidth) -
				   widget->rows + 1);
	       moved = setCurPos (widget,
				  widget->rows - 1,
				  infoLength % widget->fieldWidth);
	    }
	    break;

	 case KEY_LEFT:
	    handle_KEY_LEFT (widget, &moved, &redraw);
	    break;

	 case KEY_RIGHT:
	    if (widget->currentCol < (widget->fieldWidth - 1))
	    {
	       if ((getCursorPos (widget) + 1) <= infoLength)
	       {
		  moved = setCurPos (widget,
				     widget->currentRow,
				     widget->currentCol + 1);
	       }
	    }
	    else if (widget->currentRow == widget->rows - 1)
	    {
	       if ((widget->topRow + widget->currentRow + 1) <= widget->logicalRows)
	       {
		  moved = setCurPos (widget, widget->currentRow, 0);
		  redraw = setTopRow (widget, widget->topRow + 1);
	       }
	    }
	    else
	    {
	       moved = setCurPos (widget, widget->currentRow + 1, 0);
	    }
	    if (!moved && !redraw)
	       Beep ();
	    break;

	 case KEY_DOWN:
	    if (widget->currentRow != (widget->rows - 1))
	    {
	       if ((getCursorPos (widget) + widget->fieldWidth + 1)
		   <= infoLength)
	       {
		  moved = setCurPos (widget, widget->currentRow + 1, widget->currentCol);
	       }
	    }
	    else if (widget->topRow < widget->logicalRows - widget->rows)
	    {
	       if (((widget->topRow + widget->currentRow + 1) *
		    widget->fieldWidth) <= infoLength)
	       {
		  redraw = setTopRow (widget, widget->topRow + 1);
	       }
	    }
	    if (!moved && !redraw)
	       Beep ();
	    break;

	 case KEY_UP:
	    if (widget->currentRow != 0)
	    {
	       moved = setCurPos (widget, widget->currentRow - 1, widget->currentCol);
	    }
	    else if (widget->topRow != 0)
	    {
	       redraw = setTopRow (widget, widget->topRow - 1);
	    }
	    if (!moved && !redraw)
	       Beep ();
	    break;

	 case KEY_BACKSPACE:
	 case KEY_DC:
	    if (widget->dispType == vVIEWONLY)
	    {
	       Beep ();
	    }
	    else if (infoLength == 0)
	    {
	       Beep ();
	    }
	    else if (input == KEY_DC
		     || handle_KEY_LEFT (widget, &moved, &redraw))
	    {
	       cursorPos = getCursorPos (widget);
	       if (widget->info[cursorPos] != '\0')
	       {
		  int x;

		  for (x = cursorPos; x < infoLength; x++)
		  {
		     widget->info[x] = widget->info[x + 1];
		  }
		  widget->info[--infoLength] = '\0';

		  drawCDKMentryField (widget);
	       }
	       else
	       {
		  Beep ();
	       }
	    }
	    break;

	 case CDK_TRANSPOSE:
	    if (cursorPos >= infoLength - 1)
	    {
	       Beep ();
	    }
	    else
	    {
	       char holder = widget->info[cursorPos];
	       widget->info[cursorPos] = widget->info[cursorPos + 1];
	       widget->info[cursorPos + 1] = holder;
	       drawCDKMentryField (widget);
	    }
	    break;

	 case CDK_ERASE:
	    if (infoLength != 0)
	    {
	       cleanCDKMentry (widget);
	       drawCDKMentryField (widget);
	    }
	    break;

	 case CDK_CUT:
	    if (infoLength == 0)
	    {
	       Beep ();
	    }
	    else
	    {
	       freeChar (GPasteBuffer);
	       GPasteBuffer = copyChar (widget->info);
	       cleanCDKMentry (widget);
	       drawCDKMentryField (widget);
	    }
	    break;

	 case CDK_COPY:
	    if (infoLength == 0)
	    {
	       Beep ();
	    }
	    else
	    {
	       freeChar (GPasteBuffer);
	       GPasteBuffer = copyChar (widget->info);
	    }
	    break;

	 case CDK_PASTE:
	    if (GPasteBuffer == 0)
	    {
	       Beep ();
	    }
	    else
	    {
	       setCDKMentryValue (widget, GPasteBuffer);
	       drawCDKMentry (widget, ObjOf (widget)->box);
	    }
	    break;

	 case KEY_TAB:
	 case KEY_ENTER:
	    if (infoLength < widget->min + 1)
	    {
	       Beep ();
	    }
	    else
	    {
	       setExitType (widget, input);
	       ret = (widget->info);
	       complete = TRUE;
	    }
	    break;

	 case KEY_ERROR:
	    setExitType (widget, input);
	    complete = TRUE;
	    break;

	 case KEY_ESC:
	    setExitType (widget, input);
	    complete = TRUE;
	    break;

	 case CDK_REFRESH:
	    eraseCDKScreen (ScreenOf (widget));
	    refreshCDKScreen (ScreenOf (widget));
	    break;

	 default:
	    if (widget->dispType == vVIEWONLY
		|| infoLength >= widget->totalWidth)
	    {
	       Beep ();
	    }
	    else
	    {
	       (widget->callbackfn) (widget, input);
	    }
	    break;
	 }

	 if (redraw)
	 {
	    drawCDKMentryField (widget);
	 }
	 else if (moved)
	 {
	    wmove (widget->fieldWin, widget->currentRow, widget->currentCol);
	    wrefresh (widget->fieldWin);
	 }
      }

      /* Should we do a post-process? */
      if (!complete && (PostProcessFuncOf (widget) != 0))
      {
	 PostProcessFuncOf (widget) (vMENTRY,
				     widget,
				     PostProcessDataOf (widget),
				     input);
      }
   }

   if (!complete)
   {
      setExitType (widget, 0);
   }

   ResultOf (widget).valueString = ret;
   return (ret != unknownString);
}

/*
 * This moves the mentry field to the given location.
 */
static void _moveCDKMentry (CDKOBJS *object,
			    int xplace,
			    int yplace,
			    boolean relative,
			    boolean refresh_flag)
{
   CDKMENTRY *mentry = (CDKMENTRY *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx (mentry->win);
   int currentY = getbegy (mentry->win);
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
      xpos = getbegx (mentry->win) + xplace;
      ypos = getbegy (mentry->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (mentry), &xpos, &ypos, mentry->boxWidth, mentry->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (mentry->win, -xdiff, -ydiff);
   moveCursesWindow (mentry->fieldWin, -xdiff, -ydiff);
   moveCursesWindow (mentry->labelWin, -xdiff, -ydiff);
   moveCursesWindow (mentry->shadowWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (mentry));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKMentry (mentry, ObjOf (mentry)->box);
   }
}

/*
 * This function redraws the multiple line entry field.
 */
void drawCDKMentryField (CDKMENTRY *mentry)
{
   /* *INDENT-EQLS* */
   int currchar         = (mentry->fieldWidth * mentry->topRow);
   int length           = 0;
   int lastpos          = 0;
   int x, y;

   /* Check the value of info. */
   if (mentry->info == 0)
   {
      return;
   }

   drawCdkTitle (mentry->win, ObjOf (mentry));

   wrefresh (mentry->win);

   /* The information isn't null, redraw the field. */
   length = (int)strlen (mentry->info);
   lastpos = ((chtype)mentry->info[length] == (chtype)mentry->filler
	      ? length - 1
	      : length);

   /* Set background color and attributes of the entry field */
   wbkgd (mentry->fieldWin, mentry->fieldAttr);

   /* Start redrawing the fields. */
   for (x = 0; x < mentry->rows; x++)
   {
      for (y = 0; y < mentry->fieldWidth; y++)
      {
	 if (currchar < lastpos)
	 {
	    if (isHiddenDisplayType (mentry->dispType))
	    {
	       (void)mvwaddch (mentry->fieldWin, x, y, mentry->hidden | mentry->fieldAttr);
	    }
	    else
	    {
	       (void)mvwaddch (mentry->fieldWin, x, y,
			       CharOf (mentry->info[currchar++]) | mentry->fieldAttr);
	    }
	 }
	 else
	 {
	    (void)mvwhline (mentry->fieldWin, x, y, mentry->filler |
			    mentry->fieldAttr, mentry->fieldWidth - y);
	    break;
	 }
      }
   }

   /* Refresh the screen. */
   wmove (mentry->fieldWin, mentry->currentRow, mentry->currentCol);
   wrefresh (mentry->fieldWin);
}

/*
 * This is a generic character parser for the mentry field. It is used as a
 * callback function, so any personal modifications can be made by creating
 * a new function and calling that one the mentry activation.
 */
static void CDKMentryCallBack (CDKMENTRY *mentry, chtype character)
{
   /* *INDENT-EQLS* */
   int cursorPos  = getCursorPos (mentry);
   int infoLength = (int)strlen (mentry->info);
   char newchar   = (char)filterByDisplayType (mentry->dispType, character);

   if (newchar == ERR)
   {
      Beep ();
   }
   else
   {
      int x;

      for (x = infoLength + 1; x > cursorPos; x--)
      {
	 mentry->info[x] = mentry->info[x - 1];
      }
      mentry->info[cursorPos] = newchar;
      mentry->currentCol++;

      drawCDKMentryField (mentry);

      /* Have we gone out of bounds. */
      if (mentry->currentCol >= mentry->fieldWidth)
      {
	 /* Update the row and col values. */
	 mentry->currentCol = 0;
	 mentry->currentRow++;

	 /*
	  * If we have gone outside of the visual boundaries, we
	  * need to scroll the window.
	  */
	 if (mentry->currentRow == mentry->rows)
	 {
	    /* We have to redraw the screen. */
	    mentry->currentRow--;
	    mentry->topRow++;
	    drawCDKMentryField (mentry);
	 }
	 wmove (mentry->fieldWin, mentry->currentRow, mentry->currentCol);
	 wrefresh (mentry->fieldWin);
      }
   }
}

/*
 * This function draws the multiple line entry field.
 */
static void _drawCDKMentry (CDKOBJS *object, boolean Box)
{
   CDKMENTRY *mentry = (CDKMENTRY *)object;

   /* Box the widget if asked. */
   if (Box)
   {
      drawObjBox (mentry->win, ObjOf (mentry));
      wrefresh (mentry->win);
   }

   /* Do we need to draw in the shadow??? */
   if (mentry->shadowWin != 0)
   {
      drawShadow (mentry->shadowWin);
   }

   /* Draw in the label to the widget. */
   if (mentry->labelWin != 0)
   {
      writeChtype (mentry->labelWin, 0, 0,
		   mentry->label,
		   HORIZONTAL, 0,
		   mentry->labelLen);
      wrefresh (mentry->labelWin);
   }

   /* Draw the mentry field. */
   drawCDKMentryField (mentry);
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattrMentry (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKMENTRY *widget = (CDKMENTRY *)object;

      wbkgd (widget->win, attrib);
      wbkgd (widget->fieldWin, attrib);
      if (widget->labelWin != 0)
      {
	 wbkgd (widget->labelWin, attrib);
      }
   }
}

/*
 * This function erases the multiple line entry field from the screen.
 */
static void _eraseCDKMentry (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKMENTRY *mentry = (CDKMENTRY *)object;

      eraseCursesWindow (mentry->fieldWin);
      eraseCursesWindow (mentry->labelWin);
      eraseCursesWindow (mentry->win);
      eraseCursesWindow (mentry->shadowWin);
   }
}

/*
 * This function destroys a multiple line entry field widget.
 */
static void _destroyCDKMentry (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKMENTRY *mentry = (CDKMENTRY *)object;

      cleanCdkTitle (object);
      freeChtype (mentry->label);
      freeChar (mentry->info);

      /* Clean up the windows. */
      deleteCursesWindow (mentry->fieldWin);
      deleteCursesWindow (mentry->labelWin);
      deleteCursesWindow (mentry->shadowWin);
      deleteCursesWindow (mentry->win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vMENTRY, mentry);

      /* Unregister this object. */
      unregisterCDKObject (vMENTRY, mentry);
   }
}

/*
 * This sets multiple attributes of the widget.
 */
void setCDKMentry (CDKMENTRY *mentry, const char *value, int min, boolean Box)
{
   setCDKMentryValue (mentry, value);
   setCDKMentryMin (mentry, min);
   setCDKMentryBox (mentry, Box);
}

/*
 * This removes the old information in the entry field and keeps the
 * new information given.
 */
void setCDKMentryValue (CDKMENTRY *mentry, const char *newValue)
{
   /* *INDENT-EQLS* */
   int fieldCharacters  = mentry->rows * mentry->fieldWidth;
   int len              = 0;
   int copychars        = 0;

   /* Just to be sure, if lets make sure the new value isn't null. */
   if (newValue == 0)
   {
      /* Then we want to just erase the old value. */
      cleanChar (mentry->info, mentry->totalWidth, '\0');
      return;
   }

   /* Determine how many characters we need to copy. */
   len = (int)strlen (newValue);
   copychars = (len < mentry->totalWidth ? len : mentry->totalWidth);

   /* OK, erase the old value, and copy in the new value. */
   cleanChar (mentry->info, mentry->totalWidth, '\0');
   strncpy (mentry->info, newValue, (size_t)copychars);

   /* Set the cursor/row info. */
   if (len < fieldCharacters)
   {
      mentry->topRow = 0;
      mentry->currentRow = len / mentry->fieldWidth;
      mentry->currentCol = len % mentry->fieldWidth;
   }
   else
   {
      /* *INDENT-EQLS* */
      int rowsUsed              = len / mentry->fieldWidth;
      mentry->topRow            = rowsUsed - mentry->rows + 1;
      mentry->currentRow        = mentry->rows - 1;
      mentry->currentCol        = len % mentry->fieldWidth;
   }

   /* Redraw the widget. */
   drawCDKMentryField (mentry);
}
char *getCDKMentryValue (CDKMENTRY *mentry)
{
   return mentry->info;
}

/*
 * This sets the filler character to use when drawing the widget.
 */
void setCDKMentryFillerChar (CDKMENTRY *mentry, chtype filler)
{
   mentry->filler = filler;
}
chtype getCDKMentryFillerChar (CDKMENTRY *mentry)
{
   return mentry->filler;
}

/*
 * This sets the character to use when a hidden character type is used.
 */
void setCDKMentryHiddenChar (CDKMENTRY *mentry, chtype character)
{
   mentry->hidden = character;
}
chtype getCDKMentryHiddenChar (CDKMENTRY *mentry)
{
   return mentry->hidden;
}

/*
 * This sets the minimum length of the widget.
 */
void setCDKMentryMin (CDKMENTRY *mentry, int min)
{
   mentry->min = min;
}
int getCDKMentryMin (CDKMENTRY *mentry)
{
   return mentry->min;
}

/*
 * This sets the widgets box attribute.
 */
void setCDKMentryBox (CDKMENTRY *mentry, boolean Box)
{
   ObjOf (mentry)->box = Box;
   ObjOf (mentry)->borderSize = Box ? 1 : 0;
}
boolean getCDKMentryBox (CDKMENTRY *mentry)
{
   return ObjOf (mentry)->box;
}

/*
 * This erases the information in the multiple line entry widget.
 */
void cleanCDKMentry (CDKMENTRY *mentry)
{
   cleanChar (mentry->info, mentry->totalWidth, '\0');
   /* *INDENT-EQLS* */
   mentry->currentRow   = 0;
   mentry->currentCol   = 0;
   mentry->topRow       = 0;
}

/*
 * This sets the callback function.
 */
void setCDKMentryCB (CDKMENTRY *mentry, MENTRYCB callback)
{
   mentry->callbackfn = callback;
}

static void _focusCDKMentry (CDKOBJS *object)
{
   CDKMENTRY *mentry = (CDKMENTRY *)object;

   wmove (mentry->fieldWin, 0, mentry->currentCol);
   wrefresh (mentry->fieldWin);
}

static void _unfocusCDKMentry (CDKOBJS *object)
{
   CDKMENTRY *mentry = (CDKMENTRY *)object;

   wrefresh (mentry->fieldWin);
}

dummyRefreshData (Mentry)

dummySaveData (Mentry)
