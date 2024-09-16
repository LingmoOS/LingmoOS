#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 00:31:08 $
 * $Revision: 1.103 $
 */

DeclareCDKObjects (DIALOG, Dialog, setCdk, Int);

/*
 * This function creates a dialog widget.
 */
CDKDIALOG *newCDKDialog (CDKSCREEN *cdkscreen,
			 int xplace,
			 int yplace,
			 CDK_CSTRING2 mesg,
			 int rows,
			 CDK_CSTRING2 buttonLabel,
			 int buttonCount,
			 chtype highlight,
			 boolean separator,
			 boolean Box,
			 boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKDIALOG *dialog    = 0;
   int boxHeight;
   int boxWidth         = MIN_DIALOG_WIDTH;
   int maxmessagewidth  = -1;
   int buttonwidth      = 0;
   int xpos             = xplace;
   int ypos             = yplace;
   int temp             = 0;
   int buttonadj        = 0;
   int x                = 0;

   if (rows <= 0
       || buttonCount <= 0
       || (dialog = newCDKObject (CDKDIALOG, &my_funcs)) == 0
       || (dialog->info = typeCallocN (chtype *, rows + 1)) == 0
       || (dialog->infoLen = typeCallocN (int, rows + 1)) == 0
       || (dialog->infoPos = typeCallocN (int, rows + 1)) == 0
       || (dialog->buttonLabel = typeCallocN (chtype *, buttonCount + 1)) == 0
       || (dialog->buttonLen = typeCallocN (int, buttonCount + 1)) == 0
       || (dialog->buttonPos = typeCallocN (int, buttonCount + 1)) == 0)
   {
      destroyCDKObject (dialog);
      return (0);
   }

   setCDKDialogBox (dialog, Box);
   boxHeight = rows + 2 * BorderOf (dialog) + separator + 1;

   /* Translate the char * message to a chtype * */
   for (x = 0; x < rows; x++)
   {
      dialog->info[x] = char2Chtype (mesg[x],
				     &dialog->infoLen[x],
				     &dialog->infoPos[x]);
      maxmessagewidth = MAXIMUM (maxmessagewidth, dialog->infoLen[x]);
   }

   /* Translate the button label char * to a chtype * */
   for (x = 0; x < buttonCount; x++)
   {
      /* *INDENT-EQLS* */
      dialog->buttonLabel[x] = char2Chtype (buttonLabel[x],
					    &dialog->buttonLen[x],
					    &temp);
      buttonwidth            += dialog->buttonLen[x] + 1;
   }
   buttonwidth--;

   /* Determine the final dimensions of the box. */
   boxWidth = MAXIMUM (boxWidth, maxmessagewidth);
   boxWidth = MAXIMUM (boxWidth, buttonwidth);
   boxWidth = boxWidth + 2 + 2 * BorderOf (dialog);

   /* Now we have to readjust the x and y positions. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* *INDENT-EQLS* Set up the dialog box attributes. */
   ScreenOf (dialog)            = cdkscreen;
   dialog->parent               = cdkscreen->window;
   dialog->win                  = newwin (boxHeight, boxWidth, ypos, xpos);
   dialog->shadowWin            = 0;
   dialog->buttonCount          = buttonCount;
   dialog->currentButton        = 0;
   dialog->messageRows          = rows;
   dialog->boxHeight            = boxHeight;
   dialog->boxWidth             = boxWidth;
   dialog->highlight            = highlight;
   dialog->separator            = separator;
   initExitType (dialog);
   ObjOf (dialog)->acceptsFocus = TRUE;
   ObjOf (dialog)->inputWindow  = dialog->win;
   dialog->shadow               = shadow;

   /* If we couldn't create the window, we should return a null value. */
   if (dialog->win == 0)
   {
      destroyCDKObject (dialog);
      return (0);
   }
   keypad (dialog->win, TRUE);

   /* Find the button positions. */
   buttonadj = ((int)((boxWidth - buttonwidth) / 2));
   for (x = 0; x < buttonCount; x++)
   {
      dialog->buttonPos[x] = buttonadj;
      buttonadj = buttonadj + dialog->buttonLen[x] + BorderOf (dialog);
   }

   /* Create the string alignments. */
   for (x = 0; x < rows; x++)
   {
      dialog->infoPos[x] = justifyString (boxWidth - 2 * BorderOf (dialog),
					  dialog->infoLen[x],
					  dialog->infoPos[x]);
   }

   /* Was there a shadow? */
   if (shadow)
   {
      dialog->shadowWin = newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   /* Register this baby. */
   registerCDKObject (cdkscreen, vDIALOG, dialog);

   /* Return the dialog box pointer. */
   return (dialog);
}

/*
 * This lets the user select the button.
 */
int activateCDKDialog (CDKDIALOG *dialog, chtype *actions)
{
   boolean functionKey;
   int ret;

   /* Draw the dialog box. */
   drawCDKDialog (dialog, ObjOf (dialog)->box);

   /* Lets move to the first button. */
   writeChtypeAttrib (dialog->win,
		      dialog->buttonPos[dialog->currentButton],
		      dialog->boxHeight - 1 - BorderOf (dialog),
		      dialog->buttonLabel[dialog->currentButton],
		      dialog->highlight,
		      HORIZONTAL,
		      0, dialog->buttonLen[dialog->currentButton]);
   wrefresh (dialog->win);

   if (actions == 0)
   {
      for (;;)
      {
	 chtype input = (chtype)getchCDKObject (ObjOf (dialog), &functionKey);

	 /* Inject the character into the widget. */
	 ret = injectCDKDialog (dialog, input);
	 if (dialog->exitType != vEARLY_EXIT)
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
	 ret = injectCDKDialog (dialog, actions[x]);
	 if (dialog->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }

   /* Set the exit type and exit. */
   setExitType (dialog, 0);
   return -1;
}

/*
 * This injects a single character into the dialog widget.
 */
static int _injectCDKDialog (CDKOBJS *object, chtype input)
{
   /* *INDENT-EQLS* */
   CDKDIALOG *widget = (CDKDIALOG *)object;
   int lastButton    = widget->buttonCount - 1;
   int ppReturn      = 1;
   int ret           = unknownInt;
   bool complete     = FALSE;

   /* Set the exit type. */
   setExitType (widget, 0);

   /* Check if there is a pre-process function to be called. */
   if (PreProcessFuncOf (widget) != 0)
   {
      ppReturn = PreProcessFuncOf (widget) (vDIALOG,
					    widget,
					    PreProcessDataOf (widget),
					    input);
   }

   /* Should we continue? */
   if (ppReturn != 0)
   {
      /* Check for a key binding. */
      if (checkCDKObjectBind (vDIALOG, widget, input) != 0)
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
	    if (widget->currentButton == firstButton)
	    {
	       widget->currentButton = lastButton;;
	    }
	    else
	    {
	       widget->currentButton--;
	    }
	    break;

	 case KEY_RIGHT:
	 case KEY_TAB:
	 case SPACE:
	    if (widget->currentButton == lastButton)
	    {
	       widget->currentButton = firstButton;
	    }
	    else
	    {
	       widget->currentButton++;
	    }
	    break;

	 case KEY_UP:
	 case KEY_DOWN:
	    Beep ();
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
	 PostProcessFuncOf (widget) (vDIALOG,
				     widget,
				     PostProcessDataOf (widget),
				     input);
      }
   }

   if (!complete)
   {
      drawCDKDialogButtons (widget);
      wrefresh (widget->win);
      setExitType (widget, 0);
   }

   ResultOf (widget).valueInt = ret;
   return (ret != unknownInt);
}

/*
 * This moves the dialog field to the given location.
 */
static void _moveCDKDialog (CDKOBJS *object,
			    int xplace,
			    int yplace,
			    boolean relative,
			    boolean refresh_flag)
{
   CDKDIALOG *dialog = (CDKDIALOG *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx (dialog->win);
   int currentY = getbegy (dialog->win);
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
      xpos = getbegx (dialog->win) + xplace;
      ypos = getbegy (dialog->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (dialog), &xpos, &ypos, dialog->boxWidth, dialog->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (dialog->win, -xdiff, -ydiff);
   moveCursesWindow (dialog->shadowWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (dialog));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKDialog (dialog, ObjOf (dialog)->box);
   }
}

/*
 * This function draws the dialog widget.
 */
static void _drawCDKDialog (CDKOBJS *object, boolean Box)
{
   CDKDIALOG *dialog = (CDKDIALOG *)object;
   int x = 0;

   /* Is there a shadow? */
   if (dialog->shadowWin != 0)
   {
      drawShadow (dialog->shadowWin);
   }

   /* Box the widget if they asked. */
   if (Box)
   {
      drawObjBox (dialog->win, ObjOf (dialog));
   }

   /* Draw in the message. */
   for (x = 0; x < dialog->messageRows; x++)
   {
      writeChtype (dialog->win,
		   dialog->infoPos[x] + BorderOf (dialog), x + BorderOf (dialog),
		   dialog->info[x],
		   HORIZONTAL, 0,
		   dialog->infoLen[x]);
   }

   /* Draw in the buttons. */
   drawCDKDialogButtons (dialog);

   wrefresh (dialog->win);
}

/*
 * This function destroys the dialog widget.
 */
static void _destroyCDKDialog (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKDIALOG *dialog = (CDKDIALOG *)object;

      CDKfreeChtypes (dialog->info);
      freeChecked (dialog->infoLen);
      freeChecked (dialog->infoPos);

      CDKfreeChtypes (dialog->buttonLabel);
      freeChecked (dialog->buttonLen);
      freeChecked (dialog->buttonPos);

      /* Clean up the windows. */
      deleteCursesWindow (dialog->win);
      deleteCursesWindow (dialog->shadowWin);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vDIALOG, dialog);

      /* Unregister this object. */
      unregisterCDKObject (vDIALOG, dialog);
   }
}

/*
 * This function erases the dialog widget from the screen.
 */
static void _eraseCDKDialog (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKDIALOG *dialog = (CDKDIALOG *)object;

      eraseCursesWindow (dialog->win);
      eraseCursesWindow (dialog->shadowWin);
   }
}

/*
 * This sets attributes of the dialog box.
 */
void setCDKDialog (CDKDIALOG *dialog, chtype highlight, boolean separator, boolean Box)
{
   setCDKDialogHighlight (dialog, highlight);
   setCDKDialogSeparator (dialog, separator);
   setCDKDialogBox (dialog, Box);
}

/*
 * This sets the highlight attribute for the buttons.
 */
void setCDKDialogHighlight (CDKDIALOG *dialog, chtype highlight)
{
   dialog->highlight = highlight;
}
chtype getCDKDialogHighlight (CDKDIALOG *dialog)
{
   return dialog->highlight;
}

/*
 * This sets whether or not the dialog box will have a separator line.
 */
void setCDKDialogSeparator (CDKDIALOG *dialog, boolean separator)
{
   dialog->separator = separator;
}
boolean getCDKDialogSeparator (CDKDIALOG *dialog)
{
   return dialog->separator;
}

/*
 * This sets the box attribute of the widget.
 */
void setCDKDialogBox (CDKDIALOG *dialog, boolean Box)
{
   ObjOf (dialog)->box = Box;
   ObjOf (dialog)->borderSize = Box ? 1 : 0;
}
boolean getCDKDialogBox (CDKDIALOG *dialog)
{
   return ObjOf (dialog)->box;
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattrDialog (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKDIALOG *widget = (CDKDIALOG *)object;

      wbkgd (widget->win, attrib);
   }
}

/*
 * This draws the dialog buttons and the separation line.
 */
void drawCDKDialogButtons (CDKDIALOG *dialog)
{
   int x;

   for (x = 0; x < dialog->buttonCount; x++)
   {
      writeChtype (dialog->win,
		   dialog->buttonPos[x],
		   dialog->boxHeight - 1 - BorderOf (dialog),
		   dialog->buttonLabel[x],
		   HORIZONTAL, 0,
		   dialog->buttonLen[x]);
   }

   /* Draw the separation line. */
   if (dialog->separator)
   {
      chtype boxattr = BXAttrOf (dialog);

      for (x = 1; x < dialog->boxWidth - 1; x++)
      {
	 (void)mvwaddch (dialog->win,
			 dialog->boxHeight - 2 - BorderOf (dialog),
			 x,
			 ACS_HLINE | boxattr);
      }
      (void)mvwaddch (dialog->win,
		      dialog->boxHeight - 2 - BorderOf (dialog),
		      0,
		      ACS_LTEE | boxattr);
      (void)mvwaddch (dialog->win,
		      dialog->boxHeight - 2 - BorderOf (dialog),
		      getmaxx (dialog->win) - 1,
		      ACS_RTEE | boxattr);
   }
   writeChtypeAttrib (dialog->win,
		      dialog->buttonPos[dialog->currentButton],
		      dialog->boxHeight - 1 - BorderOf (dialog),
		      dialog->buttonLabel[dialog->currentButton],
		      dialog->highlight,
		      HORIZONTAL, 0,
		      dialog->buttonLen[dialog->currentButton]);

}

static void _focusCDKDialog (CDKOBJS *object)
{
   CDKDIALOG *widget = (CDKDIALOG *)object;

   drawCDKDialog (widget, ObjOf (widget)->box);
}

static void _unfocusCDKDialog (CDKOBJS *object)
{
   CDKDIALOG *widget = (CDKDIALOG *)object;

   drawCDKDialog (widget, ObjOf (widget)->box);
}

dummyRefreshData (Dialog)

dummySaveData (Dialog)
