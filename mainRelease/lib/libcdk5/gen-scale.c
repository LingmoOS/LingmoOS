#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 00:38:44 $
 * $Revision: 1.26 $
 */

/*
 * Declare file local prototypes.
 */
static void drawCDK<MIXED>Field (CDK<UPPER> * widget);

DeclareCDKObjects (<UPPER>, <MIXED>, setCdk, <DTYPE>);

/*
 * This function creates a widget.
 */
CDK<UPPER> *newCDK<MIXED> (CDKSCREEN *cdkscreen,
			   int xplace,
			   int yplace,
			   const char *title,
			   const char *label,
			   chtype fieldAttr,
			   int fieldWidth,
			   <CTYPE> start,
			   <CTYPE> low,
			   <CTYPE> high,
			   <CTYPE> inc,
			   <CTYPE> fastInc,
#if <FLOAT>
			   int digits,
#endif <FLOAT>
			   boolean Box,
			   boolean shadow)
{
   /* *INDENT-EQLS* */
   CDK<UPPER> *widget   = 0;
   int parentWidth      = getmaxx (cdkscreen->window);
   int parentHeight     = getmaxy (cdkscreen->window);
   int boxHeight;
   int boxWidth;
   int horizontalAdjust, oldWidth;
   int xpos             = xplace;
   int ypos             = yplace;
   int x, junk;
   /* *INDENT-OFF* */
   static const struct { int from; int to; } bindings[] = {
		{ 'u',		KEY_UP },
		{ 'U',		KEY_PPAGE },
		{ CDK_BACKCHAR,	KEY_PPAGE },
		{ CDK_FORCHAR,	KEY_NPAGE },
		{ 'g',		KEY_HOME },
		{ '^',		KEY_HOME },
		{ 'G',		KEY_END },
		{ '$',		KEY_END },
   };
   /* *INDENT-ON* */


   if ((widget = newCDKObject (CDK<UPPER>, &my_funcs)) == 0)
      return (0);

   setCDK<MIXED>Box (widget, Box);

   boxHeight = (BorderOf (widget) * 2) + 1;

   /* Set some basic values of the widget's data field. */
   widget->label = 0;
   widget->labelLen = 0;
   widget->labelWin = 0;

   /*
    * If the fieldWidth is a negative value, the fieldWidth will
    * be COLS-fieldWidth, otherwise, the fieldWidth will be the
    * given width.
    */
   fieldWidth = setWidgetDimension (parentWidth, fieldWidth, 0);
   boxWidth = fieldWidth + 2 * BorderOf (widget);

   /* Translate the label char *pointer to a chtype pointer. */
   if (label != 0)
   {
      widget->label = char2Chtype (label, &widget->labelLen, &junk);
      boxWidth = widget->labelLen + fieldWidth + 2;
   }

   oldWidth = boxWidth;
   boxWidth = setCdkTitle (ObjOf (widget), title, boxWidth);
   horizontalAdjust = (boxWidth - oldWidth) / 2;

   boxHeight += TitleLinesOf (widget);

   /*
    * Make sure we didn't extend beyond the dimensions of the window.
    */
   boxWidth = (boxWidth > parentWidth ? parentWidth : boxWidth);
   boxHeight = (boxHeight > parentHeight ? parentHeight : boxHeight);
   fieldWidth = (fieldWidth > (boxWidth - widget->labelLen - 2 * BorderOf (widget))
		 ? (boxWidth - widget->labelLen - 2 * BorderOf (widget))
		 : fieldWidth);

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* Make the widget's window. */
   widget->win = newwin (boxHeight, boxWidth, ypos, xpos);

   /* Is the main window null??? */
   if (widget->win == 0)
   {
      destroyCDKObject (widget);
      return (0);
   }

   /* Create the widget's label window. */
   if (widget->label != 0)
   {
      widget->labelWin = subwin (widget->win,
				 1, widget->labelLen,
				 ypos + TitleLinesOf (widget) + BorderOf (widget),
				 xpos + horizontalAdjust + BorderOf (widget));
      if (widget->labelWin == 0)
      {
	 destroyCDKObject (widget);
	 return (0);
      }
   }

   /* Create the widget's data field window. */
   widget->fieldWin = subwin (widget->win,
			      1, fieldWidth,
			      (ypos + TitleLinesOf (widget) + BorderOf (widget)),
			      (xpos
			       + widget->labelLen
			       + horizontalAdjust
			       + BorderOf (widget)));
   if (widget->fieldWin == 0)
   {
      destroyCDKObject (widget);
      return (0);
   }
   keypad (widget->fieldWin, TRUE);
   keypad (widget->win, TRUE);

   /* *INDENT-EQLS* Create the widget's data field. */
   ScreenOf (widget)            = cdkscreen;
   widget->parent               = cdkscreen->window;
   widget->shadowWin            = 0;
   widget->boxWidth             = boxWidth;
   widget->boxHeight            = boxHeight;
   widget->fieldWidth           = fieldWidth;
   widget->fieldAttr            = (chtype)fieldAttr;
   widget->current              = low;
   widget->low                  = low;
   widget->high                 = high;
   widget->current              = start;
   widget->inc                  = inc;
   widget->fastinc              = fastInc;
#if <FLOAT>
   widget->digits               = digits;
#endif <FLOAT>
   initExitType (widget);
   ObjOf (widget)->acceptsFocus = TRUE;
   ObjOf (widget)->inputWindow  = widget->win;
   widget->shadow               = shadow;

   /* Do we want a shadow??? */
   if (shadow)
   {
      widget->shadowWin = newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
      if (widget->shadowWin == 0)
      {
	 destroyCDKObject (widget);
	 return (0);
      }
   }

   /* Setup the key bindings. */
   for (x = 0; x < (int)SIZEOF (bindings); ++x)
      bindCDKObject (v<UPPER>,
		     widget,
		     (chtype) bindings[x].from,
		     getcCDKBind,
		     (void *)(long)bindings[x].to);

   registerCDKObject (cdkscreen, v<UPPER>, widget);

   return (widget);
}

/*
 * This allows the person to use the widget's data field.
 */
<CTYPE> activateCDK<MIXED> (CDK<UPPER> * widget, chtype *actions)
{
   <CTYPE> ret;

   /* Draw the widget. */
   drawCDK<MIXED> (widget, ObjOf (widget)->box);

   if (actions == 0)
   {
      boolean functionKey;

      for (;;)
      {
	 chtype input = (chtype) getchCDKObject (ObjOf (widget), &functionKey);

	 /* Inject the character into the widget. */
	 ret = (<CTYPE>) injectCDK<MIXED> (widget, input);
	 if (widget->exitType != vEARLY_EXIT)
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
	 ret = (<CTYPE>) injectCDK<MIXED> (widget, actions[x]);
	 if (widget->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }

   /* Set the exit type and return. */
   setExitType (widget, 0);
   return unknown<DTYPE>;
}

/*
 * Check if the value lies outside the low/high range.  If so, force it in.
 */
static void limitCurrentValue (CDK<UPPER> * widget)
{
   if (widget->current < widget->low)
   {
      widget->current = widget->low;
      Beep ();
   }
   else if (widget->current > widget->high)
   {
      widget->current = widget->high;
      Beep ();
   }
}

/*
 * Move the cursor to the given edit-position.
 */
static int moveToEditPosition (CDK<UPPER> * widget, int newPosition)
{
   return wmove (widget->fieldWin, 0, widget->fieldWidth - newPosition - 1);
}

/*
 * Check if the cursor is on a valid edit-position.  This must be one of
 * the non-blank cells in the field.
 */
static int validEditPosition (CDK<UPPER> * widget, int newPosition)
{
   chtype ch;
   if (newPosition <= 0 || newPosition >= widget->fieldWidth)
      return FALSE;
   if (moveToEditPosition (widget, newPosition) == ERR)
      return FALSE;
   ch = winch (widget->fieldWin);
   if (CharOf (ch) != ' ')
      return TRUE;
   if (newPosition > 1)
   {
      /* don't use recursion - only one level is wanted */
      if (moveToEditPosition (widget, newPosition - 1) == ERR)
	 return FALSE;
      ch = winch (widget->fieldWin);
      return CharOf (ch) != ' ';
   }
   return FALSE;
}

/*
 * Set the edit position.  Normally the cursor is one cell to the right of
 * the editable field.  Moving it left, over the field allows the user to
 * modify cells by typing in replacement characters for the field's value.
 */
static void setEditPosition (CDK<UPPER> * widget, int newPosition)
{
   if (newPosition < 0)
   {
      Beep ();
   }
   else if (newPosition == 0)
   {
      widget->fieldEdit = newPosition;
   }
   else if (validEditPosition (widget, newPosition))
   {
      widget->fieldEdit = newPosition;
   }
   else
   {
      Beep ();
   }
}

/*
 * Remove the character from the string at the given column, if it is blank.
 * Returns true if a change was made.
 */
static bool removeChar (char *string, int col)
{
   bool result = FALSE;

   if ((col >= 0) && (string[col] != ' '))
   {
      while (string[col] != '\0')
      {
	 string[col] = string[col + 1];
	 ++col;
      }
      result = TRUE;
   }
   return result;
}

/*
 * Perform an editing function for the field.
 */
static bool performEdit (CDK<UPPER> * widget, chtype input)
{
   bool result = FALSE;
   int need = widget->fieldWidth;
   char *temp = (char *)malloc ((size_t) need + 2);
   char test;
   int col = need - widget->fieldEdit - 1;
#if <FLOAT>
   double value;
#define SCANF_FMT "%lg%c"
#endif <FLOAT>
#if <INT>
   <CTYPE> value;
#define SCANF_FMT "%<PRINT>%c"
#endif <INT>

   if (temp != 0)
   {
      bool modify = TRUE;
      int base = 0;

      wmove (widget->fieldWin, 0, base);
      winnstr (widget->fieldWin, temp, need);
      strcpy (temp + need, " ");
      if (isChar (input))	/* replace the char at the cursor */
      {
	 temp[col] = (char) (input);
      }
      else if (input == KEY_BACKSPACE)	/* delete the char before the cursor */
      {
	 modify = removeChar (temp, col - 1);
      }
      else if (input == KEY_DC)	/* delete the char at the cursor */
      {
	 modify = removeChar (temp, col);
      }
      else
      {
	 modify = FALSE;
      }
      if (modify
	  && sscanf (temp, SCANF_FMT, &value, &test) == 2
	  && test == ' '
	  && value >= widget->low
	  && value <= widget->high)
      {
	 setCDK<MIXED>Value (widget, (<CTYPE>) value);
	 result = TRUE;
      }
      free (temp);
   }
   return result;
}

#define Decrement(value,by) if (value - by < value) value -= by
#define Increment(value,by) if (value + by > value) value += by

/*
 * This function injects a single character into the widget.
 */
static int _injectCDK<MIXED> (CDKOBJS *object, chtype input)
{
   CDK<UPPER> *widget = (CDK<UPPER> *) object;
   int ppReturn = 1;
   <CTYPE> ret = unknown<DTYPE>;
   bool complete = FALSE;

   /* Set the exit type. */
   setExitType (widget, 0);

   /* Draw the field. */
   drawCDK<MIXED>Field (widget);

   /* Check if there is a pre-process function to be called. */
   if (PreProcessFuncOf (widget) != 0)
   {
      /* Call the pre-process function. */
      ppReturn = PreProcessFuncOf (widget) (v<UPPER>,
					    widget,
					    PreProcessDataOf (widget),
					    input);
   }

   /* Should we continue? */
   if (ppReturn != 0)
   {
      /* Check for a key binding. */
      if (checkCDKObjectBind (v<UPPER>, widget, input) != 0)
      {
	 checkEarlyExit (widget);
	 complete = TRUE;
      }
      else
      {
	 switch (input)
	 {
	 case KEY_LEFT:
	    setEditPosition (widget, widget->fieldEdit + 1);
	    break;

	 case KEY_RIGHT:
	    setEditPosition (widget, widget->fieldEdit - 1);
	    break;

	 case KEY_DOWN:
	    Decrement (widget->current, widget->inc);
	    break;

	 case KEY_UP:
	    Increment (widget->current, widget->inc);
	    break;

	 case KEY_PPAGE:
	    Increment (widget->current, widget->fastinc);
	    break;

	 case KEY_NPAGE:
	    Decrement (widget->current, widget->fastinc);
	    break;

	 case KEY_HOME:
	    widget->current = widget->low;
	    break;

	 case KEY_END:
	    widget->current = widget->high;
	    break;

	 case KEY_TAB:
	 case KEY_ENTER:
	    setExitType (widget, input);
	    ret = (widget->current);
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
	    if (widget->fieldEdit)
	    {
	       if (!performEdit (widget, input))
		  Beep ();
	    }
	    else
	    {
	       /*
	        * The cursor is not within the editable text.  Interpret
	        * input as commands.
	        */
	       switch (input)
	       {
	       case 'd':
	       case '-':
		  return _injectCDK<MIXED> (object, KEY_DOWN);
	       case '+':
		  return _injectCDK<MIXED> (object, KEY_UP);
	       case 'D':
		  return _injectCDK<MIXED> (object, KEY_NPAGE);
	       case '0':
		  return _injectCDK<MIXED> (object, KEY_HOME);
	       default:
		  Beep ();
		  break;
	       }
	    }
	    break;
	 }
      }
      limitCurrentValue (widget);

      /* Should we call a post-process? */
      if (!complete && (PostProcessFuncOf (widget) != 0))
      {
	 PostProcessFuncOf (widget) (v<UPPER>,
				     widget,
				     PostProcessDataOf (widget),
				     input);
      }
   }

   if (!complete)
   {
      drawCDK<MIXED>Field (widget);
      setExitType (widget, 0);
   }

   ResultOf (widget).value<DTYPE> = ret;
   return (ret != unknown<DTYPE>);
}

/*
 * This moves the widget's data field to the given location.
 */
static void _moveCDK<MIXED> (CDKOBJS *object,
			     int xplace,
			     int yplace,
			     boolean relative,
			     boolean refresh_flag)
{
   CDK<UPPER> *widget = (CDK<UPPER> *) object;
   int currentX = getbegx (widget->win);
   int currentY = getbegy (widget->win);
   int xpos = xplace;
   int ypos = yplace;
   int xdiff = 0;
   int ydiff = 0;

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
   moveCursesWindow (widget->labelWin, -xdiff, -ydiff);
   moveCursesWindow (widget->fieldWin, -xdiff, -ydiff);
   moveCursesWindow (widget->shadowWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (widget));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDK<MIXED> (widget, ObjOf (widget)->box);
   }
}

/*
 * This function draws the widget.
 */
static void _drawCDK<MIXED> (CDKOBJS *object, boolean Box)
{
   CDK<UPPER> *widget = (CDK<UPPER> *) object;

   /* Draw the shadow. */
   if (widget->shadowWin != 0)
   {
      drawShadow (widget->shadowWin);
   }

   /* Box the widget if asked. */
   if (Box)
   {
      drawObjBox (widget->win, ObjOf (widget));
   }

   drawCdkTitle (widget->win, object);

   /* Draw the label. */
   if (widget->labelWin != 0)
   {
      writeChtype (widget->labelWin, 0, 0,
		   widget->label,
		   HORIZONTAL, 0,
		   widget->labelLen);
      wrefresh (widget->labelWin);
   }
   wrefresh (widget->win);

   /* Draw the field window. */
   drawCDK<MIXED>Field (widget);
}

/*
 * This draws the widget.
 */
static void drawCDK<MIXED>Field (CDK<UPPER> * widget)
{
   char temp[256];

   werase (widget->fieldWin);

   /* Draw the value in the field. */
#if <FLOAT>
   {
      char format[256];
      int digits = MINIMUM (widget->digits, 30);
      sprintf (format, "%%.%i<PRINT>", digits);
      sprintf (temp, format, widget->current);
   }
#endif <FLOAT>
#if <INT>
   sprintf (temp, "%<PRINT>", widget->current);
#endif <INT>
   writeCharAttrib (widget->fieldWin,
		    widget->fieldWidth - (int)strlen (temp) - 1,
		    0,
		    temp,
		    widget->fieldAttr,
		    HORIZONTAL,
		    0,
		    (int)strlen (temp));

   moveToEditPosition (widget, widget->fieldEdit);
   wrefresh (widget->fieldWin);
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattr<MIXED> (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDK<UPPER> *widget = (CDK<UPPER> *) object;

      wbkgd (widget->win, attrib);
      wbkgd (widget->fieldWin, attrib);
      if (widget->labelWin != 0)
      {
	 wbkgd (widget->labelWin, attrib);
      }
   }
}

/*
 * This function destroys the widget.
 */
static void _destroyCDK<MIXED> (CDKOBJS *object)
{
   if (object != 0)
   {
      CDK<UPPER> *widget = (CDK<UPPER> *) object;

      cleanCdkTitle (object);
      freeChtype (widget->label);

      /* Clean up the windows. */
      deleteCursesWindow (widget->fieldWin);
      deleteCursesWindow (widget->labelWin);
      deleteCursesWindow (widget->shadowWin);
      deleteCursesWindow (widget->win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (v<UPPER>, widget);

      /* Unregister this object. */
      unregisterCDKObject (v<UPPER>, widget);
   }
}

/*
 * This function erases the widget from the screen.
 */
static void _eraseCDK<MIXED> (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDK<UPPER> *widget = (CDK<UPPER> *) object;

      eraseCursesWindow (widget->labelWin);
      eraseCursesWindow (widget->fieldWin);
      eraseCursesWindow (widget->win);
      eraseCursesWindow (widget->shadowWin);
   }
}

/*
 * This function sets the low/high/current values of the widget.
 */
void setCDK<MIXED> (CDK<UPPER> * widget, <CTYPE> low, <CTYPE> high, <CTYPE> value, boolean Box)
{
   setCDK<MIXED>LowHigh (widget, low, high);
   setCDK<MIXED>Value (widget, value);
   setCDK<MIXED>Box (widget, Box);
}

/*
 * This sets the digits.
 */
#if <FLOAT>
void setCDK<MIXED>Digits (CDK<UPPER> * widget, int digits)
{
   widget->digits = MAXIMUM (0, digits);
}

int getCDK<MIXED>Digits (CDK<UPPER> * widget)
{
   return widget->digits;
}
#endif <FLOAT>

/*
 * This sets the widget's value.
 */
void setCDK<MIXED>Value (CDK<UPPER> * widget, <CTYPE> value)
{
   widget->current = value;
   limitCurrentValue (widget);
}
<CTYPE> getCDK<MIXED>Value (CDK<UPPER> * widget)
{
   return widget->current;
}

/*
 * This function sets the low/high values of the widget.
 */
void setCDK<MIXED>LowHigh (CDK<UPPER> * widget, <CTYPE> low, <CTYPE> high)
{
   /* Make sure the values aren't out of bounds. */
   if (low <= high)
   {
      widget->low = low;
      widget->high = high;
   }
   else
   {
      widget->low = high;
      widget->high = low;
   }

   /* Make sure the user hasn't done something silly. */
   limitCurrentValue (widget);
}
<CTYPE> getCDK<MIXED>LowValue (CDK<UPPER> * widget)
{
   return widget->low;
}
<CTYPE> getCDK<MIXED>HighValue (CDK<UPPER> * widget)
{
   return widget->high;
}

/*
 * This sets the widget's box attribute.
 */
void setCDK<MIXED>Box (CDK<UPPER> * widget, boolean Box)
{
   ObjOf (widget)->box = Box;
   ObjOf (widget)->borderSize = Box ? 1 : 0;
}
boolean getCDK<MIXED>Box (CDK<UPPER> * widget)
{
   return ObjOf (widget)->box;
}

static void _focusCDK<MIXED> (CDKOBJS *object)
{
   CDK<UPPER> *widget = (CDK<UPPER> *) object;

   drawCDK<MIXED> (widget, ObjOf (widget)->box);
}

static void _unfocusCDK<MIXED> (CDKOBJS *object)
{
   CDK<UPPER> *widget = (CDK<UPPER> *) object;

   drawCDK<MIXED> (widget, ObjOf (widget)->box);
}

dummyRefreshData (<MIXED>)

dummySaveData (<MIXED>)
