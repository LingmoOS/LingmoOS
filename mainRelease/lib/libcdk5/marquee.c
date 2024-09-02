#define NCURSES_NOMACROS
#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2016/11/20 19:14:46 $
 * $Revision: 1.82 $
 */

DeclareCDKObjects (MARQUEE, Marquee, setCdk, Unknown);

static void discardWin (WINDOW **winp)
{
   if (*winp != 0)
   {
      werase (*winp);
      wrefresh (*winp);
      delwin (*winp);
      *winp = 0;
   }
}

static void layoutWidget (CDKMARQUEE *widget,
			  int xpos,
			  int ypos)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = ScreenOf (widget);
   int parentWidth      = getmaxx (cdkscreen->window);
   WINDOW *window;
   int boxHeight;
   int boxWidth;

   discardWin (&(widget->win));
   discardWin (&(widget->shadowWin));

   boxWidth = setWidgetDimension (parentWidth, widget->width, 0);
   boxHeight = (BorderOf (widget) * 2) + 1;

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);
   window = newwin (boxHeight, boxWidth, ypos, xpos);

   if (window != 0)
   {
      /* *INDENT-EQLS* */
      widget->win          = window;
      widget->boxHeight    = boxHeight;
      widget->boxWidth     = boxWidth;

      keypad (widget->win, TRUE);

      /* Do we want a shadow? */
      if (widget->shadow)
      {
	 widget->shadowWin = subwin (cdkscreen->window,
				     boxHeight, boxWidth,
				     ypos + 1, xpos + 1);
      }
   }
}

/*
 * This creates a marquee widget.
 */
CDKMARQUEE *newCDKMarquee (CDKSCREEN *cdkscreen,
			   int xpos,
			   int ypos,
			   int width,
			   boolean Box,
			   boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKMARQUEE *widget   = 0;

   if ((widget = newCDKObject (CDKMARQUEE, &my_funcs)) == 0)
        return (0);

   /* *INDENT-EQLS* Create the widget pointer. */
   ScreenOf (widget)    = cdkscreen;
   widget->parent       = cdkscreen->window;
   widget->win          = newwin (1, 1, ypos, xpos);
   widget->active       = TRUE;
   widget->width        = width;
   widget->shadow       = shadow;

   setCDKMarqueeBox (widget, Box);

   /* Is the window null??? */
   if (widget->win == 0)
   {
      destroyCDKObject (widget);
      return (0);
   }

   registerCDKObject (cdkscreen, vMARQUEE, widget);

   return (widget);
}

/*
 * This activates the widget.
 */
int activateCDKMarquee (CDKMARQUEE *widget,
			const char *mesg,
			int delay,
			int repeat,
			boolean Box)
{
   /* *INDENT-EQLS* */
   int mesgLength       = 0;
   int startPos         = 0;
   int firstChar        = 0;
   int lastChar         = 1;
   int repeatCount      = 0;
   int viewSize         = 0;
   int viewLimit;
   int padding;
   chtype *message;
   int x, junk, oldcurs;
   bool firstTime       = TRUE;

   /* Make sure the message has some content. */
   if (mesg == 0 || *mesg == '\0')
   {
      return (-1);
   }

   /* Keep the box info, setting BorderOf () */
   setCDKMarqueeBox (widget, Box);

   padding = (mesg[strlen (mesg) - 1] == ' ') ? 0 : 1;

   /* Translate the char * to a chtype * */
   message = char2Chtype (mesg, &mesgLength, &junk);

   /* Draw in the widget. */
   drawCDKMarquee (widget, ObjOf (widget)->box);
   viewLimit = widget->width - (2 * BorderOf (widget));

   /* Start doing the marquee thing... */
   oldcurs = curs_set (0);
   while (widget->active)
   {
      int y;

      if (firstTime)
      {
	 firstChar = 0;
	 lastChar = 1;
	 viewSize = lastChar - firstChar;
	 startPos = widget->width - viewSize - BorderOf (widget);

	 firstTime = FALSE;
      }

      /* Draw in the characters. */
      y = firstChar;
      for (x = startPos; x < (startPos + viewSize); x++)
      {
	 chtype ch = (y < mesgLength) ? message[y] : ' ';
	 mvwaddch (widget->win, BorderOf (widget), x, ch);
	 y++;
      }
      wrefresh (widget->win);

      /* Set my variables. */
      if (mesgLength < viewLimit)
      {
	 if (lastChar < (mesgLength + padding))
	 {
	    lastChar++;
	    viewSize++;
	    startPos = widget->width - viewSize - BorderOf (widget);
	 }
	 else if (startPos > BorderOf (widget))
	 {
	    /* This means the whole string is visible. */
	    startPos--;
	    viewSize = mesgLength + padding;
	 }
	 else
	 {
	    /* We have to start chopping the viewSize */
	    startPos = BorderOf (widget);
	    firstChar++;
	    viewSize--;
	 }
      }
      else
      {
	 if (startPos > BorderOf (widget))
	 {
	    lastChar++;
	    viewSize++;
	    startPos--;
	 }
	 else if (lastChar < (mesgLength + padding))
	 {
	    firstChar++;
	    lastChar++;
	    startPos = BorderOf (widget);
	    viewSize = viewLimit;
	 }
	 else
	 {
	    startPos = BorderOf (widget);
	    firstChar++;
	    viewSize--;
	 }
      }

      /* OK, lets check if we have to start over. */
      if (viewSize <= 0 && firstChar == mesgLength + padding)
      {
	 /* Check if we repeat a specified number, or loop indefinitely. */
	 if ((repeat > 0) && (++repeatCount >= repeat))
	 {
	    break;
	 }

	 /* Time to start over.  */
	 mvwaddch (widget->win, BorderOf (widget), BorderOf (widget), ' ');
	 wrefresh (widget->win);
	 firstTime = TRUE;
      }

      /* Now sleep */
      napms (delay * 10);
   }
   if (oldcurs < 0)
      oldcurs = 1;
   curs_set (oldcurs);
   freeChtype (message);
   return (0);
}

/*
 * This de-activates a marquee widget.
 */
void deactivateCDKMarquee (CDKMARQUEE *widget)
{
   widget->active = FALSE;
}

/*
 * This moves the marquee field to the given location.
 */
static void _moveCDKMarquee (CDKOBJS *object,
			     int xplace,
			     int yplace,
			     boolean relative,
			     boolean refresh_flag)
{
   CDKMARQUEE *widget = (CDKMARQUEE *)object;
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
      drawCDKMarquee (widget, ObjOf (widget)->box);
   }
}

/*
 * This draws the marquee widget on the screen.
 */
static void _drawCDKMarquee (CDKOBJS *object, boolean Box)
{
   CDKMARQUEE *widget = (CDKMARQUEE *)object;

   /* Keep the box information. */
   ObjOf (widget)->box = Box;

   /* Do we need to draw a shadow??? */
   if (widget->shadowWin != 0)
   {
      drawShadow (widget->shadowWin);
   }

   /* Box it if needed. */
   if (Box)
   {
      drawObjBox (widget->win, ObjOf (widget));
   }
   else
   {
      touchwin (widget->win);
   }

   /* Refresh the window. */
   wrefresh (widget->win);
}

/*
 * This destroys the widget.
 */
static void _destroyCDKMarquee (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKMARQUEE *widget = (CDKMARQUEE *)object;

      /* Clean up the windows. */
      deleteCursesWindow (widget->shadowWin);
      deleteCursesWindow (widget->win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vMARQUEE, widget);

      /* Unregister this object. */
      unregisterCDKObject (vMARQUEE, widget);
   }
}

/*
 * This erases the widget.
 */
static void _eraseCDKMarquee (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKMARQUEE *widget = (CDKMARQUEE *)object;

      eraseCursesWindow (widget->win);
      eraseCursesWindow (widget->shadowWin);
   }
}

/*
 * This sets the widgets box attribute.
 */
void setCDKMarqueeBox (CDKMARQUEE *widget, boolean Box)
{
   int xpos = widget->win ? getbegx (widget->win) : 0;
   int ypos = widget->win ? getbegy (widget->win) : 0;

   ObjOf (widget)->box = Box;
   ObjOf (widget)->borderSize = Box ? 1 : 0;

   layoutWidget (widget, xpos, ypos);
}
boolean getCDKMarqueeBox (CDKMARQUEE *widget)
{
   return ObjOf (widget)->box;
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattrMarquee (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKMARQUEE *widget = (CDKMARQUEE *)object;

      wbkgd (widget->win, attrib);
   }
}

dummyInject (Marquee)

dummyFocus (Marquee)

dummyUnfocus (Marquee)

dummyRefreshData (Marquee)

dummySaveData (Marquee)
