#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 00:40:47 $
 * $Revision: 1.69 $
 */

DeclareCDKObjects (BUTTONBOX, Buttonbox, setCdk, Int);

/*
 * This returns a CDK buttonbox widget pointer.
 */
CDKBUTTONBOX *newCDKButtonbox (CDKSCREEN *cdkscreen,
			       int xPos,
			       int yPos,
			       int height,
			       int width,
			       const char *title,
			       int rows,
			       int cols,
			       CDK_CSTRING2 buttons,
			       int buttonCount,
			       chtype highlight,
			       boolean Box,
			       boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKBUTTONBOX *buttonbox      = 0;
   int parentWidth              = getmaxx (cdkscreen->window);
   int parentHeight             = getmaxy (cdkscreen->window);
   int boxWidth                 = 0;
   int boxHeight                = 0;
   int maxColWidth              = INT_MIN;
   int colWidth                 = 0;
   int xpos                     = xPos;
   int ypos                     = yPos;
   int currentButton            = 0;
   int x, y, junk;

   if (buttonCount <= 0
       || (buttonbox = newCDKObject (CDKBUTTONBOX, &my_funcs)) == 0
       || (buttonbox->button = typeCallocN (chtype *, buttonCount + 1)) == 0
       || (buttonbox->buttonLen = typeCallocN (int, buttonCount + 1)) == 0
       || (buttonbox->buttonPos = typeCallocN (int, buttonCount + 1)) == 0
       || (buttonbox->columnWidths = typeCallocN (int, buttonCount + 1)) == 0)
   {
      destroyCDKObject (buttonbox);
      return (0);
   }

   setCDKButtonboxBox (buttonbox, Box);

   /* Set some default values for the widget. */
   buttonbox->rowAdjust = 0;
   buttonbox->colAdjust = 0;

   /*
    * If the height is a negative value, the height will
    * be ROWS-height, otherwise, the height will be the
    * given height.
    */
   boxHeight = setWidgetDimension (parentHeight, height, rows + 1);

   /*
    * If the width is a negative value, the width will
    * be COLS-width, otherwise, the width will be the
    * given width.
    */
   boxWidth = setWidgetDimension (parentWidth, width, 0);

   boxWidth = setCdkTitle (ObjOf (buttonbox), title, boxWidth);

   /* Translate the buttons char * to a chtype * */
   for (x = 0; x < buttonCount; x++)
   {
      buttonbox->button[x] = char2Chtype (buttons[x],
					  &buttonbox->buttonLen[x],
					  &junk);
   }

   /* Set the button positions. */
   for (x = 0; x < cols; x++)
   {
      maxColWidth = INT_MIN;

      /* Look for the widest item in this column. */
      for (y = 0; y < rows; y++)
      {
	 if (currentButton < buttonCount)
	 {
	    maxColWidth = MAXIMUM (buttonbox->buttonLen[currentButton], maxColWidth);
	    currentButton++;
	 }
      }

      /* Keep the maximum column width for this column. */
      buttonbox->columnWidths[x] = maxColWidth;
      colWidth += maxColWidth;
   }
   boxWidth++;

   /*
    * Make sure we didn't extend beyond the dimensions of the window.
    */
   boxWidth = (boxWidth > parentWidth ? parentWidth : boxWidth);
   boxHeight = (boxHeight > parentHeight ? parentHeight : boxHeight);

   /* Now we have to readjust the x and y positions. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* *INDENT-EQLS* Set up the buttonbox box attributes. */
   ScreenOf (buttonbox)            = cdkscreen;
   buttonbox->parent               = cdkscreen->window;
   buttonbox->win                  = newwin (boxHeight, boxWidth, ypos, xpos);
   buttonbox->shadowWin            = 0;
   buttonbox->buttonCount          = buttonCount;
   buttonbox->currentButton        = 0;
   buttonbox->rows                 = rows;
   buttonbox->cols                 = (buttonCount < cols ? buttonCount : cols);
   buttonbox->boxHeight            = boxHeight;
   buttonbox->boxWidth             = boxWidth;
   buttonbox->highlight            = highlight;
   initExitType (buttonbox);
   ObjOf (buttonbox)->acceptsFocus = TRUE;
   ObjOf (buttonbox)->inputWindow  = buttonbox->win;
   buttonbox->shadow               = shadow;
   buttonbox->ButtonAttrib         = A_NORMAL;

   /* Set up the row adjustment. */
   if (boxHeight - rows - TitleLinesOf (buttonbox) > 0)
   {
      buttonbox->rowAdjust = (int)((boxHeight
				    - rows
				    - TitleLinesOf (buttonbox)) / buttonbox->rows);
   }

   /* Set the col adjustment. */
   if (boxWidth - colWidth > 0)
   {
      buttonbox->colAdjust = (int)((boxWidth - colWidth)
				   / buttonbox->cols) - 1;
   }

   /* If we couldn't create the window, we should return a null value. */
   if (buttonbox->win == 0)
   {
      destroyCDKObject (buttonbox);
      return (0);
   }
   keypad (buttonbox->win, TRUE);

   /* Was there a shadow? */
   if (shadow)
   {
      buttonbox->shadowWin = newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   /* Register this baby. */
   registerCDKObject (cdkscreen, vBUTTONBOX, buttonbox);

   /* Return the buttonbox box pointer. */
   return (buttonbox);
}

/*
 * This activates the widget.
 */
int activateCDKButtonbox (CDKBUTTONBOX *buttonbox, chtype *actions)
{
   boolean functionKey;
   int ret;

   /* Draw the buttonbox box. */
   drawCDKButtonbox (buttonbox, ObjOf (buttonbox)->box);

   if (actions == 0)
   {
      for (;;)
      {
	 chtype input = (chtype)getchCDKObject (ObjOf (buttonbox), &functionKey);

	 /* Inject the character into the widget. */
	 ret = injectCDKButtonbox (buttonbox, input);
	 if (buttonbox->exitType != vEARLY_EXIT)
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
	 ret = injectCDKButtonbox (buttonbox, actions[x]);
	 if (buttonbox->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }

   /* Set the exit type and exit. */
   setExitType (buttonbox, 0);
   return -1;
}

/*
 * This injects a single character into the widget.
 */
static int _injectCDKButtonbox (CDKOBJS *object, chtype input)
{
   /* *INDENT-EQLS* */
   CDKBUTTONBOX *widget = (CDKBUTTONBOX *)object;
   int lastButton       = widget->buttonCount - 1;
   int ppReturn         = 1;
   int ret              = unknownInt;
   bool complete        = FALSE;

   /* Set the exit type. */
   setExitType (widget, 0);

   /* Check if there is a pre-process function to be called. */
   if (PreProcessFuncOf (widget) != 0)
   {
      ppReturn = PreProcessFuncOf (widget) (vBUTTONBOX,
					    widget,
					    PreProcessDataOf (widget),
					    input);
   }

   /* Should we continue? */
   if (ppReturn != 0)
   {
      /* Check for a key binding. */
      if (checkCDKObjectBind (vBUTTONBOX, widget, input) != 0)
      {
	 checkEarlyExit (widget);
	 complete = TRUE;
      }
      else
      {
	 int firstButton = 0;

	 switch (input)
	 {
	 case KEY_LEFT:
	 case KEY_BTAB:
	 case KEY_BACKSPACE:
	    if ((widget->currentButton - widget->rows) < firstButton)
	    {
	       widget->currentButton = lastButton;
	    }
	    else
	    {
	       widget->currentButton -= widget->rows;
	    }
	    break;

	 case KEY_RIGHT:
	 case KEY_TAB:
	 case SPACE:
	    if ((widget->currentButton + widget->rows) > lastButton)
	    {
	       widget->currentButton = firstButton;
	    }
	    else
	    {
	       widget->currentButton += widget->rows;
	    }
	    break;

	 case KEY_UP:
	    if ((widget->currentButton - 1) < firstButton)
	    {
	       widget->currentButton = lastButton;
	    }
	    else
	    {
	       widget->currentButton--;
	    }
	    break;

	 case KEY_DOWN:
	    if ((widget->currentButton + 1) > lastButton)
	    {
	       widget->currentButton = firstButton;
	    }
	    else
	    {
	       widget->currentButton++;
	    }
	    break;

	 case CDK_REFRESH:
	    eraseCDKScreen (ScreenOf (widget));
	    refreshCDKScreen (ScreenOf (widget));
	    break;

	 case KEY_ESC:
	    setExitType (widget, input);
	    complete = TRUE;
	    break;

	 case KEY_ERROR:
	    setExitType (widget, input);
	    complete = TRUE;
	    break;

	 case KEY_ENTER:
	    setExitType (widget, input);
	    ret = widget->currentButton;
	    complete = TRUE;
	    break;

	 default:
	    break;
	 }
      }

      /* Should we call a post-process? */
      if (!complete && (PostProcessFuncOf (widget) != 0))
      {
	 PostProcessFuncOf (widget) (vBUTTONBOX,
				     widget,
				     PostProcessDataOf (widget),
				     input);
      }
   }

   if (!complete)
   {
      drawCDKButtonboxButtons (widget);
      setExitType (widget, 0);
   }

   ResultOf (widget).valueInt = ret;
   return (ret != unknownInt);
}

/*
 * This sets multiple attributes of the widget.
 */
void setCDKButtonbox (CDKBUTTONBOX *buttonbox, chtype highlight, boolean Box)
{
   setCDKButtonboxHighlight (buttonbox, highlight);
   setCDKButtonboxBox (buttonbox, Box);
}

/*
 * This sets the highlight attribute for the buttonboxs.
 */
void setCDKButtonboxHighlight (CDKBUTTONBOX *buttonbox, chtype highlight)
{
   buttonbox->highlight = highlight;
}
chtype getCDKButtonboxHighlight (CDKBUTTONBOX *buttonbox)
{
   return (chtype)buttonbox->highlight;
}

/*
 * This sets the box attribute of the widget.
 */
void setCDKButtonboxBox (CDKBUTTONBOX *buttonbox, boolean Box)
{
   ObjOf (buttonbox)->box = Box;
   ObjOf (buttonbox)->borderSize = Box ? 1 : 0;
}
boolean getCDKButtonboxBox (CDKBUTTONBOX *buttonbox)
{
   return ObjOf (buttonbox)->box;
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattrButtonbox (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKBUTTONBOX *widget = (CDKBUTTONBOX *)object;

      wbkgd (widget->win, attrib);
   }
}

/*
 * This draws the buttonbox box widget.
 */
static void _drawCDKButtonbox (CDKOBJS *object, boolean Box)
{
   CDKBUTTONBOX *buttonbox = (CDKBUTTONBOX *)object;

   /* Is there a shadow? */
   if (buttonbox->shadowWin != 0)
   {
      drawShadow (buttonbox->shadowWin);
   }

   /* Box the widget if they asked. */
   if (Box)
   {
      drawObjBox (buttonbox->win, ObjOf (buttonbox));
   }

   /* Draw in the title if there is one. */
   drawCdkTitle (buttonbox->win, object);

   /* Draw in the buttons. */
   drawCDKButtonboxButtons (buttonbox);
}

/*
 * This draws the buttons on the button box widget.
 */
void drawCDKButtonboxButtons (CDKBUTTONBOX *buttonbox)
{
   /* *INDENT-EQLS* */
   int row;
   int col              = (int)(buttonbox->colAdjust / 2);
   int currentButton    = 0;
   int x, y;
   int cur_row          = -1;
   int cur_col          = -1;

   /* Draw the buttons. */
   while (currentButton < buttonbox->buttonCount)
   {
      for (x = 0; x < buttonbox->cols; x++)
      {
	 row = TitleLinesOf (buttonbox) + BorderOf (buttonbox);

	 for (y = 0; y < buttonbox->rows; y++)
	 {
	    chtype attr = buttonbox->ButtonAttrib;
	    if (currentButton == buttonbox->currentButton)
	    {
	       attr = buttonbox->highlight,
		  cur_row = row;
	       cur_col = col;
	    }
	    writeChtypeAttrib (buttonbox->win,
			       col, row,
			       buttonbox->button[currentButton],
			       attr,
			       HORIZONTAL, 0,
			       buttonbox->buttonLen[currentButton]);
	    row += (1 + buttonbox->rowAdjust);
	    currentButton++;
	 }
	 col += buttonbox->columnWidths[x] + buttonbox->colAdjust + BorderOf (buttonbox);
      }
   }
   if (cur_row >= 0 && cur_col >= 0)
      wmove (buttonbox->win, cur_row, cur_col);
   wrefresh (buttonbox->win);
}

/*
 * This erases the buttonbox box from the screen.
 */
static void _eraseCDKButtonbox (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKBUTTONBOX *buttonbox = (CDKBUTTONBOX *)object;

      eraseCursesWindow (buttonbox->win);
      eraseCursesWindow (buttonbox->shadowWin);
   }
}

/*
 * This moves the buttonbox box to a new screen location.
 */
static void _moveCDKButtonbox (CDKOBJS *object,
			       int xplace,
			       int yplace,
			       boolean relative,
			       boolean refresh_flag)
{
   CDKBUTTONBOX *buttonbox = (CDKBUTTONBOX *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx (buttonbox->win);
   int currentY = getbegy (buttonbox->win);
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
      xpos = getbegx (buttonbox->win) + xplace;
      ypos = getbegy (buttonbox->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (buttonbox), &xpos, &ypos, buttonbox->boxWidth, buttonbox->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (buttonbox->win, -xdiff, -ydiff);
   moveCursesWindow (buttonbox->shadowWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (buttonbox));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKButtonbox (buttonbox, ObjOf (buttonbox)->box);
   }
}

/*
 * This destroys the widget and all the memory associated with it.
 */
static void _destroyCDKButtonbox (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKBUTTONBOX *buttonbox = (CDKBUTTONBOX *)object;

      cleanCdkTitle (object);
      CDKfreeChtypes (buttonbox->button);
      freeChecked (buttonbox->buttonLen);
      freeChecked (buttonbox->buttonPos);
      freeChecked (buttonbox->columnWidths);

      /* Delete the windows. */
      deleteCursesWindow (buttonbox->shadowWin);
      deleteCursesWindow (buttonbox->win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vBUTTONBOX, buttonbox);

      /* Unregister this object. */
      unregisterCDKObject (vBUTTONBOX, buttonbox);
   }
}

/*
 *
 */
void setCDKButtonboxCurrentButton (CDKBUTTONBOX *buttonbox, int button)
{
   if ((button >= 0) && (button < buttonbox->buttonCount))
   {
      buttonbox->currentButton = button;
   }
}
int getCDKButtonboxCurrentButton (CDKBUTTONBOX *buttonbox)
{
   return buttonbox->currentButton;
}
int getCDKButtonboxButtonCount (CDKBUTTONBOX *buttonbox)
{
   return buttonbox->buttonCount;
}

static void _focusCDKButtonbox (CDKOBJS *object)
{
   CDKBUTTONBOX *widget = (CDKBUTTONBOX *)object;

   drawCDKButtonbox (widget, ObjOf (widget)->box);
}

static void _unfocusCDKButtonbox (CDKOBJS *object)
{
   CDKBUTTONBOX *widget = (CDKBUTTONBOX *)object;

   drawCDKButtonbox (widget, ObjOf (widget)->box);
}

dummyRefreshData (Buttonbox)

dummySaveData (Buttonbox)
