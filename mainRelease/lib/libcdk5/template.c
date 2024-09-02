#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 01:08:29 $
 * $Revision: 1.143 $
 */

/*
 * Declare file local prototypes.
 */
static void CDKTemplateCallBack (CDKTEMPLATE *widget, chtype input);
static void drawCDKTemplateField (CDKTEMPLATE *widget);
static void adjustCDKTemplateCursor (CDKTEMPLATE *widget, int direction);

#define isPlateChar(c) ((c) != 0 && strchr ("#ACcMXz", c) != 0)

DeclareCDKObjects (TEMPLATE, Template, setCdk, String);

/*
 * This creates a cdktemplate widget.
 */
CDKTEMPLATE *newCDKTemplate (CDKSCREEN *cdkscreen,
			     int xplace,
			     int yplace,
			     const char *title,
			     const char *label,
			     const char *plate,
			     const char *Overlay,
			     boolean Box,
			     boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKTEMPLATE *cdktemplate     = 0;
   int parentWidth              = getmaxx (cdkscreen->window);
   int parentHeight             = getmaxy (cdkscreen->window);
   int boxWidth                 = 0;
   int boxHeight                = Box ? 3 : 1;
   int xpos                     = xplace;
   int ypos                     = yplace;
   int horizontalAdjust, oldWidth;
   int fieldWidth               = 0;
   int plateLen                 = 0;
   int junk                     = 0;

   if (plate == 0
       || (cdktemplate = newCDKObject (CDKTEMPLATE, &my_funcs)) == 0)
        return (0);

   setCDKTemplateBox (cdktemplate, Box);

   fieldWidth = (int)strlen (plate) + 2 * BorderOf (cdktemplate);

   /* *INDENT-EQLS* Set some basic values of the cdktemplate field. */
   cdktemplate->label     = 0;
   cdktemplate->labelLen  = 0;
   cdktemplate->labelWin  = 0;

   /* Translate the char * label to a chtype * */
   if (label != 0)
   {
      cdktemplate->label = char2Chtype (label, &cdktemplate->labelLen, &junk);
   }

   /* Translate the char * Overlay to a chtype * */
   if (Overlay != 0)
   {
      cdktemplate->overlay = char2Chtype (Overlay, &cdktemplate->overlayLen, &junk);
      cdktemplate->fieldAttr = cdktemplate->overlay[0] & A_ATTRIBUTES;
   }
   else
   {
      /* *INDENT-EQLS* */
      cdktemplate->overlay      = 0;
      cdktemplate->overlayLen   = 0;
      cdktemplate->fieldAttr    = A_NORMAL;
   }

   /* Set the box width. */
   boxWidth = fieldWidth + cdktemplate->labelLen + 2 * BorderOf (cdktemplate);

   oldWidth = boxWidth;
   boxWidth = setCdkTitle (ObjOf (cdktemplate), title, boxWidth);
   horizontalAdjust = (boxWidth - oldWidth) / 2;

   boxHeight += TitleLinesOf (cdktemplate);

   /*
    * Make sure we didn't extend beyond the dimensions of the window.
    */
   /* *INDENT-EQLS* */
   boxWidth   = MINIMUM (boxWidth, parentWidth);
   boxHeight  = MINIMUM (boxHeight, parentHeight);
   fieldWidth = MINIMUM (fieldWidth,
			 boxWidth   - cdktemplate->labelLen - 2 * BorderOf (cdktemplate));

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* Make the cdktemplate window */
   cdktemplate->win = newwin (boxHeight, boxWidth, ypos, xpos);

   /* Is the cdktemplate window null?? */
   if (cdktemplate->win == 0)
   {
      destroyCDKObject (cdktemplate);
      return (0);
   }
   keypad (cdktemplate->win, TRUE);

   /* Make the label window. */
   if (cdktemplate->label != 0)
   {
      cdktemplate->labelWin = subwin (cdktemplate->win,
				      1, cdktemplate->labelLen,
				      (ypos +
				       TitleLinesOf (cdktemplate) +
				       BorderOf (cdktemplate)),
				      (xpos +
				       horizontalAdjust +
				       BorderOf (cdktemplate)));
   }

   /* Make the field window. */
   cdktemplate->fieldWin = subwin (cdktemplate->win,
				   1, fieldWidth,
				   (ypos +
				    TitleLinesOf (cdktemplate) +
				    BorderOf (cdktemplate)),
				   (xpos +
				    cdktemplate->labelLen +
				    horizontalAdjust +
				    BorderOf (cdktemplate)));
   keypad (cdktemplate->fieldWin, TRUE);

   /* Set up the info field. */
   cdktemplate->plateLen = (int)strlen (plate);
   cdktemplate->info = typeCallocN (char, cdktemplate->plateLen + 2);
   if (cdktemplate->info == 0)
   {
      destroyCDKObject (cdktemplate);
      return (0);
   }

   /* Copy the plate to the cdktemplate. */
   plateLen = (int)strlen (plate);
   cdktemplate->plate = typeMallocN (char, plateLen + 3);
   if (cdktemplate->plate == 0)
   {
      destroyCDKObject (cdktemplate);
      return (0);
   }
   strcpy (cdktemplate->plate, plate);

   /* *INDENT-EQLS* Set up the rest of the structure */
   ScreenOf (cdktemplate)               = cdkscreen;
   cdktemplate->parent                  = cdkscreen->window;
   cdktemplate->shadowWin               = 0;
   cdktemplate->fieldWidth              = fieldWidth;
   cdktemplate->boxHeight               = boxHeight;
   cdktemplate->boxWidth                = boxWidth;
   cdktemplate->platePos                = 0;
   cdktemplate->screenPos               = 0;
   cdktemplate->infoPos                 = 0;
   initExitType (cdktemplate);
   cdktemplate->min                     = 0;
   ObjOf (cdktemplate)->inputWindow     = cdktemplate->win;
   ObjOf (cdktemplate)->acceptsFocus    = TRUE;
   cdktemplate->shadow                  = shadow;
   cdktemplate->callbackfn              = CDKTemplateCallBack;

   /* Do we need to create a shadow??? */
   if (shadow)
   {
      cdktemplate->shadowWin = newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   registerCDKObject (cdkscreen, vTEMPLATE, cdktemplate);

   return cdktemplate;
}

/*
 * This actually manages the cdktemplate widget...
 */
char *activateCDKTemplate (CDKTEMPLATE *cdktemplate, chtype *actions)
{
   boolean functionKey;
   char *ret = 0;

   /* Draw the object. */
   drawCDKTemplate (cdktemplate, ObjOf (cdktemplate)->box);

   if (actions == 0)
   {
      for (;;)
      {
	 chtype input = (chtype)getchCDKObject (ObjOf (cdktemplate), &functionKey);

	 /* Inject the character into the widget. */
	 ret = injectCDKTemplate (cdktemplate, input);
	 if (cdktemplate->exitType != vEARLY_EXIT)
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
	 ret = injectCDKTemplate (cdktemplate, actions[x]);
	 if (cdktemplate->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }

   /* Set the exit type and return. */
   setExitType (cdktemplate, 0);
   return ret;
}

/*
 * This injects a character into the widget.
 */
static int _injectCDKTemplate (CDKOBJS *object, chtype input)
{
   CDKTEMPLATE *widget = (CDKTEMPLATE *)object;
   int ppReturn = 1;
   char *ret = unknownString;
   bool complete = FALSE;

   /* Set the exit type and return. */
   setExitType (widget, 0);

   /* Move the cursor. */
   drawCDKTemplateField (widget);

   /* Check if there is a pre-process function to be called. */
   if (PreProcessFuncOf (widget) != 0)
   {
      ppReturn = PreProcessFuncOf (widget) (vTEMPLATE,
					    widget,
					    PreProcessDataOf (widget),
					    input);
   }

   /* Should we continue? */
   if (ppReturn != 0)
   {
      /* Check a predefined binding...  */
      if (checkCDKObjectBind (vTEMPLATE, widget, input) != 0)
      {
	 checkEarlyExit (widget);
	 complete = TRUE;
      }
      else
      {
	 switch (input)
	 {
	 case CDK_ERASE:
	    if (strlen (widget->info) != 0)
	    {
	       cleanCDKTemplate (widget);
	       drawCDKTemplateField (widget);
	    }
	    break;

	 case CDK_CUT:
	    if ((int)strlen (widget->info) != 0)
	    {
	       freeChar (GPasteBuffer);
	       GPasteBuffer = copyChar (widget->info);
	       cleanCDKTemplate (widget);
	       drawCDKTemplateField (widget);
	    }
	    else
	    {
	       Beep ();
	    }
	    break;

	 case CDK_COPY:
	    if ((int)strlen (widget->info) != 0)
	    {
	       freeChar (GPasteBuffer);
	       GPasteBuffer = copyChar (widget->info);
	    }
	    else
	    {
	       Beep ();
	    }
	    break;

	 case CDK_PASTE:
	    if (GPasteBuffer != 0)
	    {
	       int length, x;

	       cleanCDKTemplate (widget);

	       /* Start inserting each character one at a time. */
	       length = (int)strlen (GPasteBuffer);
	       for (x = 0; x < length; x++)
	       {
		  (widget->callbackfn) (widget, (chtype)GPasteBuffer[x]);
	       }
	       drawCDKTemplateField (widget);
	    }
	    else
	    {
	       Beep ();
	    }
	    break;

	 case KEY_TAB:
	 case KEY_ENTER:
	    if ((int)strlen (widget->info) < (int)widget->min)
	    {
	       Beep ();
	    }
	    else
	    {
	       setExitType (widget, input);
	       ret = widget->info;
	       complete = TRUE;
	    }
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
	    (widget->callbackfn) (widget, input);
	    break;
	 }
      }

      /* Should we call a post-process? */
      if (!complete && (PostProcessFuncOf (widget) != 0))
      {
	 PostProcessFuncOf (widget) (vTEMPLATE,
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
 * Return true if the given string matches the template (may be incomplete).
 */
static boolean validTemplate (CDKTEMPLATE *cdktemplate, char *input)
{
   int pp, ip;
   const char *plate = cdktemplate->plate;

   for (pp = 0, ip = 0; input[ip] != '\0' && plate[pp] != '\0'; ++ip, ++pp)
   {
      int newchar = input[ip];

      while (plate[pp] != '\0' && !isPlateChar (plate[pp]))
      {
	 ++pp;
      }
      if (plate[pp] == 0)
      {
	 return FALSE;
      }

      /* check if the input matches the plate */
      if (isdigit (CharOf (newchar)) &&
	  (plate[pp] == 'A' ||
	   plate[pp] == 'C' ||
	   plate[pp] == 'c'))
      {
	 return FALSE;
      }
      if (!isdigit (CharOf (newchar)) &&
	  plate[pp] == '#')
      {
	 return FALSE;
      }

      /* Do we need to convert the case??? */
      if (plate[pp] == 'C' ||
	  plate[pp] == 'X')
      {
	 newchar = toupper (newchar);
      }
      else if (plate[pp] == 'c' ||
	       plate[pp] == 'x')
      {
	 newchar = tolower (newchar);
      }
      input[ip] = (char)newchar;
   }
   return TRUE;
}

/*
 * This is the standard callback proc for the cdktemplate.
 */
static void CDKTemplateCallBack (CDKTEMPLATE *cdktemplate, chtype input)
{
   boolean failed = FALSE;
   boolean change = FALSE;
   boolean moveby = FALSE;
   int amount = 0;
   size_t mark = (size_t)cdktemplate->infoPos;
   size_t have = strlen (cdktemplate->info);

   if (input == KEY_LEFT)
   {
      if (mark != 0)
      {
	 moveby = TRUE;
	 amount = -1;
      }
      else
      {
	 failed = TRUE;
      }
   }
   else if (input == KEY_RIGHT)
   {
      if (cdktemplate->info[mark] != '\0')
      {
	 moveby = TRUE;
	 amount = 1;
      }
      else
      {
	 failed = TRUE;
      }
   }
   else
   {
      char *test = (char *)malloc (have + 2);

      if (test != 0)
      {
	 strcpy (test, cdktemplate->info);

	 if (input == KEY_BACKSPACE)
	 {
	    if (mark != 0)
	    {
	       strcpy (test + mark - 1, cdktemplate->info + mark);
	       change = TRUE;
	       amount = -1;
	    }
	    else
	    {
	       failed = TRUE;
	    }
	 }
	 else if (input == KEY_DC)
	 {
	    if (test[mark] != '\0')
	    {
	       strcpy (test + mark, cdktemplate->info + mark + 1);
	       change = TRUE;
	       amount = 0;
	    }
	    else
	    {
	       failed = TRUE;
	    }
	 }
	 else if (isChar (input) &&
		  cdktemplate->platePos < cdktemplate->fieldWidth)
	 {
	    test[mark] = (char)(input);
	    strcpy (test + mark + 1, cdktemplate->info + mark + 1);
	    change = TRUE;
	    amount = 1;
	 }
	 else
	 {
	    failed = TRUE;
	 }

	 if (change)
	 {
	    if (validTemplate (cdktemplate, test))
	    {
	       strcpy (cdktemplate->info, test);
	       drawCDKTemplateField (cdktemplate);
	    }
	    else
	    {
	       failed = TRUE;
	    }
	 }

	 free (test);
      }
   }

   if (failed)
   {
      Beep ();
   }
   else if (change || moveby)
   {
      cdktemplate->infoPos += amount;
      cdktemplate->platePos += amount;
      cdktemplate->screenPos += amount;

      adjustCDKTemplateCursor (cdktemplate, amount);
   }
}

/*
 * Return a mixture of the plate-overlay and field-info.
 */
char *mixCDKTemplate (CDKTEMPLATE *cdktemplate)
{
   char *mixedString = 0;

   if (cdktemplate->info != 0 && cdktemplate->info[0] != '\0')
   {
      mixedString = typeCallocN (char, cdktemplate->plateLen + 3);

      if (mixedString != 0)
      {
	 int infoPos = 0;
	 int platePos = 0;

	 while (platePos < cdktemplate->plateLen)
	 {
	    mixedString[platePos] = (char)(isPlateChar (cdktemplate->plate[platePos])
					   ? cdktemplate->info[infoPos++]
					   : cdktemplate->plate[platePos]);
	    platePos++;
	 }
      }
   }

   return mixedString;
}

/*
 * Return the field-info from the mixed string.
 */
char *unmixCDKTemplate (CDKTEMPLATE *cdktemplate, const char *info)
{
   /* *INDENT-EQLS* */
   int infolen          = (int)strlen (info);
   char *unmixedString  = typeCallocN (char, infolen + 2);

   if (unmixedString != 0)
   {
      int pos = 0;
      int x = 0;

      while (pos < infolen)
      {
	 if (isPlateChar (cdktemplate->plate[pos]))
	 {
	    unmixedString[x++] = info[pos++];
	 }
	 else
	 {
	    pos++;
	 }
      }
   }

   return unmixedString;
}

/*
 * Move the cdktemplate field to the given location.
 */
static void _moveCDKTemplate (CDKOBJS *object,
			      int xplace,
			      int yplace,
			      boolean relative,
			      boolean refresh_flag)
{
   CDKTEMPLATE *cdktemplate = (CDKTEMPLATE *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx (cdktemplate->win);
   int currentY = getbegy (cdktemplate->win);
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
      xpos = getbegx (cdktemplate->win) + xplace;
      ypos = getbegy (cdktemplate->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (cdktemplate), &xpos, &ypos, cdktemplate->boxWidth, cdktemplate->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (cdktemplate->win, -xdiff, -ydiff);
   moveCursesWindow (cdktemplate->labelWin, -xdiff, -ydiff);
   moveCursesWindow (cdktemplate->fieldWin, -xdiff, -ydiff);
   moveCursesWindow (cdktemplate->shadowWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (cdktemplate));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKTemplate (cdktemplate, ObjOf (cdktemplate)->box);
   }
}

/*
 * Draw the template widget.
 */
static void _drawCDKTemplate (CDKOBJS *object, boolean Box)
{
   CDKTEMPLATE *cdktemplate = (CDKTEMPLATE *)object;

   /* Do we need to draw the shadow. */
   if (cdktemplate->shadowWin != 0)
   {
      drawShadow (cdktemplate->shadowWin);
   }

   /* Box it if needed */
   if (Box)
   {
      drawObjBox (cdktemplate->win, ObjOf (cdktemplate));
   }

   drawCdkTitle (cdktemplate->win, object);

   wrefresh (cdktemplate->win);

   drawCDKTemplateField (cdktemplate);
}

/*
 * Draw the cdktemplate field.
 */
static void drawCDKTemplateField (CDKTEMPLATE *cdktemplate)
{
   /* *INDENT-EQLS* */
   int infolen          = (int)strlen (cdktemplate->info);

   /* Draw in the label and the cdktemplate object. */
   if (cdktemplate->labelWin != 0)
   {
      writeChtype (cdktemplate->labelWin, 0, 0,
		   cdktemplate->label,
		   HORIZONTAL, 0,
		   cdktemplate->labelLen);
      wrefresh (cdktemplate->labelWin);
   }

   /* Draw in the cdktemplate... */
   if (cdktemplate->overlay != 0)
   {
      writeChtype (cdktemplate->fieldWin, 0, 0,
		   cdktemplate->overlay,
		   HORIZONTAL, 0,
		   cdktemplate->overlayLen);

      /* Adjust the cursor. */
      if (infolen != 0)
      {
	 int pos = 0;
	 int x = 0;
	 for (x = 0; x < cdktemplate->fieldWidth; x++)
	 {
	    if (isPlateChar (cdktemplate->plate[x]) && pos < infolen)
	    {
	       chtype fieldColor = cdktemplate->overlay[x] & A_ATTRIBUTES;
	       (void)mvwaddch (cdktemplate->fieldWin,
			       0, x,
			       CharOf (cdktemplate->info[pos++]) | fieldColor);
	    }
	 }
	 wmove (cdktemplate->fieldWin, 0, cdktemplate->screenPos);
      }
      else
      {
	 adjustCDKTemplateCursor (cdktemplate, +1);
      }
      wrefresh (cdktemplate->fieldWin);
   }
}

/*
 * Adjust the cursor for the cdktemplate.
 */
static void adjustCDKTemplateCursor (CDKTEMPLATE *cdktemplate, int direction)
{
   while (!isPlateChar (cdktemplate->plate[cdktemplate->platePos])
	  && cdktemplate->platePos < cdktemplate->fieldWidth)
   {
      cdktemplate->platePos += direction;
      cdktemplate->screenPos += direction;
   }
   wmove (cdktemplate->fieldWin, 0, cdktemplate->screenPos);
   wrefresh (cdktemplate->fieldWin);
}

/*
 * Set the background attribute of the widget.
 */
static void _setBKattrTemplate (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKTEMPLATE *widget = (CDKTEMPLATE *)object;

      wbkgd (widget->win, attrib);
      wbkgd (widget->fieldWin, attrib);
      if (widget->labelWin != 0)
      {
	 wbkgd (widget->labelWin, attrib);
      }
   }
}

/*
 * Destroy this widget.
 */
static void _destroyCDKTemplate (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKTEMPLATE *cdktemplate = (CDKTEMPLATE *)object;

      cleanCdkTitle (object);
      freeChtype (cdktemplate->label);
      freeChtype (cdktemplate->overlay);
      freeChar (cdktemplate->plate);
      freeChar (cdktemplate->info);

      /* Delete the windows. */
      deleteCursesWindow (cdktemplate->fieldWin);
      deleteCursesWindow (cdktemplate->labelWin);
      deleteCursesWindow (cdktemplate->shadowWin);
      deleteCursesWindow (cdktemplate->win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vTEMPLATE, cdktemplate);

      unregisterCDKObject (vTEMPLATE, cdktemplate);
   }
}

/*
 * Erase the widget.
 */
static void _eraseCDKTemplate (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKTEMPLATE *cdktemplate = (CDKTEMPLATE *)object;

      eraseCursesWindow (cdktemplate->fieldWin);
      eraseCursesWindow (cdktemplate->labelWin);
      eraseCursesWindow (cdktemplate->win);
      eraseCursesWindow (cdktemplate->shadowWin);
   }
}

/*
 * Set the value given to the cdktemplate.
 */
void setCDKTemplate (CDKTEMPLATE *cdktemplate, const char *newValue, boolean Box)
{
   setCDKTemplateValue (cdktemplate, newValue);
   setCDKTemplateBox (cdktemplate, Box);
}

/*
 * Set the value given to the cdktemplate.
 */
void setCDKTemplateValue (CDKTEMPLATE *cdktemplate, const char *newValue)
{
   /* *INDENT-EQLS* */
   int len              = 0;
   int copychars        = 0;
   int x;

   cleanCDKTemplate (cdktemplate);

   /* Just to be sure, if let's make sure the new value isn't null. */
   if (newValue == 0)
   {
      return;
   }

   /* Determine how many characters we need to copy. */
   len = (int)strlen (newValue);
   copychars = MINIMUM (len, cdktemplate->fieldWidth);

   /* OK, erase the old value, and copy in the new value. */
   cdktemplate->info[0] = '\0';
   strncpy (cdktemplate->info, newValue, (size_t)copychars);

   /* Use the function which handles the input of the characters. */
   for (x = 0; x < len; x++)
   {
      (cdktemplate->callbackfn) (cdktemplate, (chtype)newValue[x]);
   }
}
char *getCDKTemplateValue (CDKTEMPLATE *cdktemplate)
{
   return cdktemplate->info;
}

/*
 * Set the minimum number of characters to enter into the widget.
 */
void setCDKTemplateMin (CDKTEMPLATE *cdktemplate, int min)
{
   if (min >= 0)
   {
      cdktemplate->min = min;
   }
}
int getCDKTemplateMin (CDKTEMPLATE *cdktemplate)
{
   return cdktemplate->min;
}

/*
 * Set the box attribute of the cdktemplate widget.
 */
void setCDKTemplateBox (CDKTEMPLATE *cdktemplate, boolean Box)
{
   ObjOf (cdktemplate)->box = Box;
   ObjOf (cdktemplate)->borderSize = Box ? 1 : 0;
}
boolean getCDKTemplateBox (CDKTEMPLATE *cdktemplate)
{
   return ObjOf (cdktemplate)->box;
}

/*
 * Erase the information in the cdktemplate widget.
 */
void cleanCDKTemplate (CDKTEMPLATE *cdktemplate)
{
   if (cdktemplate->fieldWidth > 0)
      memset (cdktemplate->info, 0, (size_t)cdktemplate->fieldWidth);

   /* *INDENT-EQLS* */
   cdktemplate->screenPos = 0;
   cdktemplate->infoPos   = 0;
   cdktemplate->platePos  = 0;
}

/*
 * Set the callback function for the widget.
 */
void setCDKTemplateCB (CDKTEMPLATE *cdktemplate, TEMPLATECB callback)
{
   cdktemplate->callbackfn = callback;
}

static void _focusCDKTemplate (CDKOBJS *object)
{
   CDKTEMPLATE *widget = (CDKTEMPLATE *)object;

   drawCDKTemplate (widget, ObjOf (widget)->box);
}

static void _unfocusCDKTemplate (CDKOBJS *object)
{
   CDKTEMPLATE *widget = (CDKTEMPLATE *)object;

   drawCDKTemplate (widget, ObjOf (widget)->box);
}

dummyRefreshData (Template)

dummySaveData (Template)
