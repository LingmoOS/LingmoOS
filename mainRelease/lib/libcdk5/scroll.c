#include <cdk_int.h>
#include <scroller.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 01:09:14 $
 * $Revision: 1.167 $
 */

/*
 * Declare file local prototypes.
 */
static void drawCDKScrollList (CDKSCROLL *scrollp, boolean Box);
static int createCDKScrollItemList (CDKSCROLL *scrollp, boolean numbers,
				    CDK_CSTRING2 list, int listSize);
static void fixCursorPosition (CDKSCROLL *widget);
static void setViewSize (CDKSCROLL *scrollp, int listSize);
static int maxViewSize (CDKSCROLL *scrollp);

#define NUMBER_FMT      "%4d. %s"
#define NUMBER_LEN(s)   (8 + strlen (s))

/* Determine how many characters we can shift to the right */
/* before all the items have been scrolled off the screen. */
#define AvailableWidth(w)  ((w)->boxWidth - 2 * BorderOf (w))
#define updateViewWidth(w, widest) \
	(w)->maxLeftChar = (((w)->boxWidth > widest) \
			      ? 0 \
			      : (widest - AvailableWidth (w)))
#define WidestItem(w)      ((w)->maxLeftChar + AvailableWidth (w))

#define SCREENPOS(w,n) (w)->itemPos[n] - (w)->leftChar	/* + scrollbarAdj + BorderOf(w) */

DeclareCDKObjects (SCROLL, Scroll, setCdk, Int);

/*
 * This function creates a new scrolling list widget.
 */
CDKSCROLL *newCDKScroll (CDKSCREEN *cdkscreen,
			 int xplace,
			 int yplace,
			 int splace,
			 int height,
			 int width,
			 const char *title,
			 CDK_CSTRING2 list,
			 int listSize,
			 boolean numbers,
			 chtype highlight,
			 boolean Box,
			 boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKSCROLL *scrollp           = 0;
   int parentWidth              = getmaxx (cdkscreen->window);
   int parentHeight             = getmaxy (cdkscreen->window);
   int boxWidth;
   int boxHeight;
   int xpos                     = xplace;
   int ypos                     = yplace;
   int scrollAdjust             = 0;
   int x;
   /* *INDENT-OFF* */
   static const struct { int from; int to; } bindings[] = {
		{ CDK_BACKCHAR,	KEY_PPAGE },
		{ CDK_FORCHAR,	KEY_NPAGE },
		{ 'g',		KEY_HOME },
		{ '1',		KEY_HOME },
		{ 'G',		KEY_END },
		{ '<',		KEY_HOME },
		{ '>',		KEY_END },
   };
   /* *INDENT-ON* */

   if ((scrollp = newCDKObject (CDKSCROLL, &my_funcs)) == 0)
   {
      destroyCDKObject (scrollp);
      return (0);
   }

   setCDKScrollBox (scrollp, Box);

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

   boxWidth = setCdkTitle (ObjOf (scrollp), title, boxWidth);

   /* Set the box height. */
   if (TitleLinesOf (scrollp) > boxHeight)
   {
      boxHeight = (TitleLinesOf (scrollp)
		   + MINIMUM (listSize, 8)
		   + 2 * BorderOf (scrollp));
   }

   /* Adjust the box width if there is a scrollp bar. */
   if ((splace == LEFT) || (splace == RIGHT))
   {
      scrollp->scrollbar = TRUE;
      boxWidth += 1;
   }
   else
   {
      scrollp->scrollbar = FALSE;
   }

   /*
    * Make sure we didn't extend beyond the dimensions of the window.
    */
   scrollp->boxWidth = (boxWidth > parentWidth
			? (parentWidth - scrollAdjust)
			: boxWidth);
   scrollp->boxHeight = (boxHeight > parentHeight
			 ? parentHeight
			 : boxHeight);

   setViewSize (scrollp, listSize);

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, scrollp->boxWidth, scrollp->boxHeight);

   /* Make the scrolling window */
   scrollp->win = newwin (scrollp->boxHeight, scrollp->boxWidth, ypos, xpos);

   /* Is the scrolling window null?? */
   if (scrollp->win == 0)
   {
      destroyCDKObject (scrollp);
      return (0);
   }

   /* Turn the keypad on for the window. */
   keypad (scrollp->win, TRUE);

   /* Create the scrollbar window. */
   if (splace == RIGHT)
   {
      scrollp->scrollbarWin = subwin (scrollp->win,
				      maxViewSize (scrollp), 1,
				      SCREEN_YPOS (scrollp, ypos),
				      xpos + scrollp->boxWidth
				      - BorderOf (scrollp) - 1);
   }
   else if (splace == LEFT)
   {
      scrollp->scrollbarWin = subwin (scrollp->win,
				      maxViewSize (scrollp), 1,
				      SCREEN_YPOS (scrollp, ypos),
				      SCREEN_XPOS (scrollp, xpos));
   }
   else
   {
      scrollp->scrollbarWin = 0;
   }

   /* create the list window */

   scrollp->listWin = subwin (scrollp->win,
			      maxViewSize (scrollp),
			      scrollp->boxWidth
			      - 2 * BorderOf (scrollp) - scrollAdjust,
			      SCREEN_YPOS (scrollp, ypos),
			      SCREEN_XPOS (scrollp, xpos)
			      + (splace == LEFT ? 1 : 0));

   /* *INDENT-EQLS* Set the rest of the variables */
   ScreenOf (scrollp)           = cdkscreen;
   scrollp->parent              = cdkscreen->window;
   scrollp->shadowWin           = 0;
   scrollp->scrollbarPlacement  = splace;
   scrollp->maxLeftChar         = 0;
   scrollp->leftChar            = 0;
   scrollp->highlight           = highlight;
   initExitType (scrollp);
   ObjOf (scrollp)->acceptsFocus = TRUE;
   ObjOf (scrollp)->inputWindow = scrollp->win;
   scrollp->shadow              = shadow;

   setCDKScrollPosition (scrollp, 0);

   /* Create the scrolling list item list and needed variables. */
   if (createCDKScrollItemList (scrollp, numbers, list, listSize) <= 0)
   {
      destroyCDKObject (scrollp);
      return (0);
   }

   /* Do we need to create a shadow? */
   if (shadow)
   {
      scrollp->shadowWin = newwin (scrollp->boxHeight,
				   boxWidth,
				   ypos + 1,
				   xpos + 1);
   }

   /* Setup the key bindings. */
   for (x = 0; x < (int)SIZEOF (bindings); ++x)
      bindCDKObject (vSCROLL,
		     scrollp,
		     (chtype)bindings[x].from,
		     getcCDKBind,
		     (void *)(long)bindings[x].to);

   registerCDKObject (cdkscreen, vSCROLL, scrollp);

   /* Return the scrolling list */
   return scrollp;
}

/*
 * Put the cursor on the currently-selected item's row.
 */
static void fixCursorPosition (CDKSCROLL *widget)
{
   scroller_FixCursorPosition ((CDKSCROLLER *)widget);
}

/*
 * This actually does all the 'real' work of managing the scrolling list.
 */
int activateCDKScroll (CDKSCROLL *scrollp, chtype *actions)
{
   /* Draw the scrolling list */
   drawCDKScroll (scrollp, ObjOf (scrollp)->box);

   if (actions == 0)
   {
      boolean functionKey;

      for (;;)
      {
	 chtype input;
	 int ret;

	 fixCursorPosition (scrollp);
	 input = (chtype)getchCDKObject (ObjOf (scrollp), &functionKey);

	 /* Inject the character into the widget. */
	 ret = injectCDKScroll (scrollp, input);
	 if (scrollp->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }
   else
   {
      int length = chlen (actions);
      int i = 0;

      /* Inject each character one at a time. */
      for (i = 0; i < length; i++)
      {
	 int ret = injectCDKScroll (scrollp, actions[i]);
	 if (scrollp->exitType != vEARLY_EXIT)
	    return ret;
      }
   }

   /* Set the exit type for the widget and return. */
   setExitType (scrollp, 0);
   return -1;
}

/*
 * This injects a single character into the widget.
 */
static int _injectCDKScroll (CDKOBJS *object, chtype input)
{
   CDKSCROLL *myself = (CDKSCROLL *)object;
   CDKSCROLLER *widget = (CDKSCROLLER *)object;
   int ppReturn = 1;
   int ret = unknownInt;
   bool complete = FALSE;

   /* Set the exit type for the widget. */
   setExitType (widget, 0);

   /* Draw the scrolling list */
   drawCDKScrollList (myself, ObjOf (widget)->box);

   /* Check if there is a pre-process function to be called. */
   if (PreProcessFuncOf (widget) != 0)
   {
      /* Call the pre-process function. */
      ppReturn = PreProcessFuncOf (widget) (vSCROLL,
					    widget,
					    PreProcessDataOf (widget),
					    input);
   }

   /* Should we continue? */
   if (ppReturn != 0)
   {
      /* Check for a predefined key binding. */
      if (checkCDKObjectBind (vSCROLL, widget, input) != 0)
      {
	 checkEarlyExit (widget);
	 complete = TRUE;
      }
      else
      {
	 switch (input)
	 {
	 case KEY_UP:
	    scroller_KEY_UP (widget);
	    break;

	 case KEY_DOWN:
	    scroller_KEY_DOWN (widget);
	    break;

	 case KEY_RIGHT:
	    scroller_KEY_RIGHT (widget);
	    break;

	 case KEY_LEFT:
	    scroller_KEY_LEFT (widget);
	    break;

	 case KEY_PPAGE:
	    scroller_KEY_PPAGE (widget);
	    break;

	 case KEY_NPAGE:
	    scroller_KEY_NPAGE (widget);
	    break;

	 case KEY_HOME:
	    scroller_KEY_HOME (widget);
	    break;

	 case KEY_END:
	    scroller_KEY_END (widget);
	    break;

	 case '$':
	    widget->leftChar = widget->maxLeftChar;
	    break;

	 case '|':
	    widget->leftChar = 0;
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

	 case KEY_TAB:
	 case KEY_ENTER:
	    setExitType (widget, input);
	    ret = widget->currentItem;
	    complete = TRUE;
	    break;

	 default:
	    break;
	 }
      }

      /* Should we call a post-process? */
      if (!complete && (PostProcessFuncOf (widget) != 0))
      {
	 PostProcessFuncOf (widget) (vSCROLL,
				     widget,
				     PostProcessDataOf (widget),
				     input);
      }
   }

   if (!complete)
   {
      drawCDKScrollList (myself, ObjOf (widget)->box);
      setExitType (widget, 0);
   }

   fixCursorPosition (myself);
   ResultOf (widget).valueInt = ret;
   return (ret != unknownInt);
}

/*
 * This allows the user to accelerate to a position in the scrolling list.
 */
void setCDKScrollPosition (CDKSCROLL *scrollp, int item)
{
   scroller_SetPosition ((CDKSCROLLER *)scrollp, item);
}

/* obsolete (because the name is inconsistent) */
int getCDKScrollCurrent (CDKSCROLL *scrollp)
{
   return scrollp->currentItem;
}

void setCDKScrollCurrent (CDKSCROLL *scrollp, int item)
{
   scroller_SetPosition ((CDKSCROLLER *)scrollp, item);
}

/*
 * Get/Set the current item number of the scroller.
 */
int getCDKScrollCurrentItem (CDKSCROLL *widget)
{
   return widget->currentItem;
}

void setCDKScrollCurrentItem (CDKSCROLL *widget, int item)
{
   scroller_SetPosition ((CDKSCROLLER *)widget, item);
}

/*
 * Get/Set the top line of the scroller.
 */
int getCDKScrollCurrentTop (CDKSCROLL *widget)
{
   return widget->currentTop;
}

void setCDKScrollCurrentTop (CDKSCROLL *widget, int item)
{
   if (item < 0)
      item = 0;
   else if (item > widget->maxTopItem)
      item = widget->maxTopItem;
   widget->currentTop = item;

   scroller_SetPosition ((CDKSCROLLER *)widget, item);
}

/*
 * This moves the scroll field to the given location.
 */
static void _moveCDKScroll (CDKOBJS *object,
			    int xplace,
			    int yplace,
			    boolean relative,
			    boolean refresh_flag)
{
   /* *INDENT-EQLS* */
   CDKSCROLL *scrollp = (CDKSCROLL *)object;
   int currentX       = getbegx (scrollp->win);
   int currentY       = getbegy (scrollp->win);
   int xpos           = xplace;
   int ypos           = yplace;
   int xdiff          = 0;
   int ydiff          = 0;

   /*
    * If this is a relative move, then we will adjust where we want
    * to move to.
    */
   if (relative)
   {
      xpos = getbegx (scrollp->win) + xplace;
      ypos = getbegy (scrollp->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (scrollp), &xpos, &ypos, scrollp->boxWidth, scrollp->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (scrollp->win, -xdiff, -ydiff);
   moveCursesWindow (scrollp->listWin, -xdiff, -ydiff);
   moveCursesWindow (scrollp->shadowWin, -xdiff, -ydiff);
   moveCursesWindow (scrollp->scrollbarWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (scrollp));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKScroll (scrollp, ObjOf (scrollp)->box);
   }
}

/*
 * This function draws the scrolling list widget.
 */
static void _drawCDKScroll (CDKOBJS *object, boolean Box)
{
   CDKSCROLL *scrollp = (CDKSCROLL *)object;

   /* Draw in the shadow if we need to. */
   if (scrollp->shadowWin != 0)
      drawShadow (scrollp->shadowWin);

   drawCdkTitle (scrollp->win, object);

   /* Draw in the scolling list items. */
   drawCDKScrollList (scrollp, Box);
}

static void drawCDKScrollCurrent (CDKSCROLL *s)
{
   if ((s->listSize > 0) && (s->listSize > s->currentItem))
   {
      /* Rehighlight the current menu item. */
      int screenPos = s->itemPos[s->currentItem] - s->leftChar;
      chtype highlight = HasFocusObj (s) ? s->highlight : A_NORMAL;

      writeChtypeAttrib (s->listWin,
			 (screenPos >= 0) ? screenPos : 0,
			 s->currentHigh,
			 s->item[s->currentItem],
			 highlight,
			 HORIZONTAL,
			 (screenPos >= 0) ? 0 : (1 - screenPos),
			 s->itemLen[s->currentItem]);
   }
}

static int maxViewSize (CDKSCROLL *scrollp)
{
   return scroller_MaxViewSize ((CDKSCROLLER *)scrollp);
}

/*
 * Set variables that depend upon the list-size.
 */
static void setViewSize (CDKSCROLL *scrollp, int listSize)
{
   scroller_SetViewSize ((CDKSCROLLER *)scrollp, listSize);
}

#undef  SCREEN_YPOS		/* because listWin is separate */
#define SCREEN_YPOS(w,n) (n)

/*
 * This redraws the scrolling list.
 */
static void drawCDKScrollList (CDKSCROLL *scrollp, boolean Box)
{
   /* If the list is empty, don't draw anything. */
   if (scrollp->listSize > 0)
   {
      int j;

      /* Redraw the list */
      for (j = 0; j < scrollp->viewSize; j++)
      {
	 int k;
	 int xpos = SCREEN_YPOS (scrollp, 0);
	 int ypos = SCREEN_YPOS (scrollp, j);

	 writeBlanks (scrollp->listWin, xpos, ypos,
		      HORIZONTAL, 0, scrollp->boxWidth - 2 * BorderOf (scrollp));

	 k = j + scrollp->currentTop;

	 /* Draw the elements in the scroll list. */
	 if (k < scrollp->listSize)
	 {
	    int screenPos = SCREENPOS (scrollp, k);

	    /* Write in the correct line. */
	    writeChtype (scrollp->listWin,
			 (screenPos >= 0) ? screenPos : 1,
			 ypos,
			 scrollp->item[k],
			 HORIZONTAL,
			 (screenPos >= 0) ? 0 : (1 - screenPos),
			 scrollp->itemLen[k]);
	 }
      }

      drawCDKScrollCurrent (scrollp);

      /* Determine where the toggle is supposed to be. */
      if (scrollp->scrollbarWin != 0)
      {
	 scrollp->togglePos = floorCDK (scrollp->currentItem * (double)scrollp->step);

	 /* Make sure the toggle button doesn't go out of bounds. */

	 if (scrollp->togglePos >= getmaxy (scrollp->scrollbarWin))
	    scrollp->togglePos = getmaxy (scrollp->scrollbarWin) - 1;

	 /* Draw the scrollbar. */
	 (void)mvwvline (scrollp->scrollbarWin,
			 0, 0,
			 CDK_FILLED_BOX,
			 getmaxy (scrollp->scrollbarWin));
	 (void)mvwvline (scrollp->scrollbarWin,
			 scrollp->togglePos, 0,
			 ' ' | A_REVERSE,
			 scrollp->toggleSize);
      }
   }

   /* Box it if needed. */
   if (Box)
   {
      drawObjBox (scrollp->win, ObjOf (scrollp));
   }
   else
   {
      touchwin (scrollp->win);
   }
   wrefresh (scrollp->win);
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattrScroll (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKSCROLL *widget = (CDKSCROLL *)object;

      wbkgd (widget->win, attrib);
      wbkgd (widget->listWin, attrib);
      if (widget->scrollbarWin != 0)
      {
	 wbkgd (widget->scrollbarWin, attrib);
      }
   }
}

/*
 * This function destroys
 */
static void _destroyCDKScroll (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKSCROLL *scrollp = (CDKSCROLL *)object;

      cleanCdkTitle (object);
      CDKfreeChtypes (scrollp->item);
      freeChecked (scrollp->itemPos);
      freeChecked (scrollp->itemLen);

      /* Clean up the windows. */
      deleteCursesWindow (scrollp->scrollbarWin);
      deleteCursesWindow (scrollp->shadowWin);
      deleteCursesWindow (scrollp->listWin);
      deleteCursesWindow (scrollp->win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vSCROLL, scrollp);

      /* Unregister this object. */
      unregisterCDKObject (vSCROLL, scrollp);
   }
}

/*
 * This function erases the scrolling list from the screen.
 */
static void _eraseCDKScroll (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKSCROLL *scrollp = (CDKSCROLL *)object;

      eraseCursesWindow (scrollp->win);
      eraseCursesWindow (scrollp->shadowWin);
   }
}

static boolean allocListArrays (CDKSCROLL *scrollp,
				int newSize,
				bool first)
{
   /* *INDENT-EQLS* */
   boolean result;
   chtype **newList     = 0;
   int *newLen          = 0;
   int *newPos          = 0;
   int n;

   if (newSize != 0)
   {
      /* *INDENT-EQLS* */
      int nchunk  = ((newSize + 1) | 31) + 1;
      newList     = typeCallocN (chtype *, nchunk);
      newLen      = typeCallocN (int, nchunk);
      newPos      = typeCallocN (int, nchunk);
   }

   /* if called via setCDKScrollItems(), free the obsolete data */
   if ((first || (newSize == 0)) && scrollp->item && scrollp->listSize)
   {
      for (n = 0; n < scrollp->listSize; ++n)
      {
	 freeAndNull (scrollp->item[n]);
	 scrollp->itemLen[n] = 0;
	 scrollp->itemPos[n] = 0;
      }
   }

   if (newList != 0 &&
       newLen != 0 &&
       newPos != 0)
   {
      if (!first)
      {
	 for (n = 0; n < scrollp->listSize; ++n)
	 {
	    /* *INDENT-EQLS* */
	    newList[n] = scrollp->item[n];
	    newLen[n]  = scrollp->itemLen[n];
	    newPos[n]  = scrollp->itemPos[n];
	 }
      }

      freeChecked (scrollp->item);
      freeChecked (scrollp->itemPos);
      freeChecked (scrollp->itemLen);

      /* *INDENT-EQLS* */
      scrollp->listSize = newSize;
      scrollp->item     = newList;
      scrollp->itemLen  = newLen;
      scrollp->itemPos  = newPos;

      result = TRUE;
   }
   else
   {
      freeAndNull (scrollp->item);
      freeAndNull (scrollp->itemPos);
      freeAndNull (scrollp->itemLen);

      freeChecked (newList);
      freeChecked (newLen);
      freeChecked (newPos);
      result = FALSE;
   }
   return result;
}

static boolean allocListItem (CDKSCROLL *scrollp,
			      int which,
			      char **work,
			      size_t *used,
			      int number,
			      const char *value)
{
   if (number > 0)
   {
      size_t need = NUMBER_LEN (value);
      if (need > *used)
      {
	 *used = ((need + 2) * 2);
	 if (*work == 0)
	 {
	    if ((*work = (char *)malloc (*used)) == 0)
	       return FALSE;
	 }
	 else
	 {
	    if ((*work = (char *)realloc (*work, *used)) == 0)
	       return FALSE;
	 }
      }
      sprintf (*work, NUMBER_FMT, number, value);
      value = *work;
   }

   if ((scrollp->item[which] = char2Chtype (value,
					    &(scrollp->itemLen[which]),
					    &(scrollp->itemPos[which]))) == 0)
      return FALSE;

   scrollp->itemPos[which] = justifyString (scrollp->boxWidth,
					    scrollp->itemLen[which],
					    scrollp->itemPos[which]);

   return TRUE;
}

/*
 * This function creates the scrolling list information and sets up the needed
 * variables for the scrolling list to work correctly.
 */
static int createCDKScrollItemList (CDKSCROLL *scrollp,
				    boolean numbers,
				    CDK_CSTRING2 list,
				    int listSize)
{
   int status = 0;

   if (listSize > 0)
   {
      /* *INDENT-EQLS* */
      size_t have               = 0;
      char *temp                = 0;

      if (allocListArrays (scrollp, listSize, TRUE))
      {
	 int widestItem = 0;
	 int x = 0;

	 /* Create the items in the scrolling list. */
	 status = 1;
	 for (x = 0; x < listSize; x++)
	 {
	    if (!allocListItem (scrollp,
				x,
				&temp,
				&have,
				numbers ? (x + 1) : 0,
				list[x]))
	    {
	       status = 0;
	       break;
	    }

	    widestItem = MAXIMUM (scrollp->itemLen[x], widestItem);
	 }
	 freeChecked (temp);

	 if (status)
	 {
	    updateViewWidth (scrollp, widestItem);

	    /* Keep the boolean flag 'numbers' */
	    scrollp->numbers = numbers;
	 }
      }
   }
   else
   {
      status = 1;		/* null list is ok - for a while */
   }

   return status;
}

/*
 * This sets certain attributes of the scrolling list.
 */
void setCDKScroll (CDKSCROLL *scrollp,
		   CDK_CSTRING2 list,
		   int listSize,
		   boolean numbers,
		   chtype highlight,
		   boolean Box)
{
   setCDKScrollItems (scrollp, list, listSize, numbers);
   setCDKScrollHighlight (scrollp, highlight);
   setCDKScrollBox (scrollp, Box);
}

/*
 * This sets the scrolling list items.
 */
void setCDKScrollItems (CDKSCROLL *scrollp, CDK_CSTRING2 list, int listSize, boolean numbers)
{
   int x = 0;

   if (createCDKScrollItemList (scrollp, numbers, list, listSize) <= 0)
      return;

   /* Clean up the display. */
   for (x = 0; x < scrollp->viewSize; x++)
   {
      writeBlanks (scrollp->win, 1, SCREEN_YPOS (scrollp, x),
		   HORIZONTAL, 0, scrollp->boxWidth - 2);
   }

   setViewSize (scrollp, listSize);
   setCDKScrollPosition (scrollp, 0);
   scrollp->leftChar = 0;
}
int getCDKScrollItems (CDKSCROLL *scrollp, char **list)
{
   if (list != 0)
   {
      int x;

      for (x = 0; x < scrollp->listSize; x++)
      {
	 list[x] = chtype2Char (scrollp->item[x]);
      }
   }
   return scrollp->listSize;
}

/*
 * This sets the highlight of the scrolling list.
 */
void setCDKScrollHighlight (CDKSCROLL *scrollp, chtype highlight)
{
   scrollp->highlight = highlight;
}
chtype getCDKScrollHighlight (CDKSCROLL *scrollp, chtype highlight GCC_UNUSED)
{
   return scrollp->highlight;
}

/*
 * This sets the box attribute of the scrolling list.
 */
void setCDKScrollBox (CDKSCROLL *scrollp, boolean Box)
{
   ObjOf (scrollp)->box = Box;
   ObjOf (scrollp)->borderSize = Box ? 1 : 0;
}
boolean getCDKScrollBox (CDKSCROLL *scrollp)
{
   return ObjOf (scrollp)->box;
}

/*
 * Resequence the numbers after a insertion/deletion.
 */
static void resequence (CDKSCROLL *scrollp)
{
   if (scrollp->numbers)
   {
      int j, k;
      for (j = 0; j < scrollp->listSize; ++j)
      {
	 char source[80];
	 chtype *target = scrollp->item[j];

	 sprintf (source, NUMBER_FMT, j + 1, "");

	 for (k = 0; source[k] != 0; ++k)
	 {
	    /* handle deletions that change the length of number */
	    if (source[k] == '.' && CharOf (target[k]) != '.')
	    {
	       int k2 = k;
	       while ((target[k2] = target[k2 + 1]) != 0)
		  ++k2;
	       scrollp->itemLen[j] -= 1;
	    }
	    target[k] &= A_ATTRIBUTES;
	    target[k] |= (chtype)(unsigned char)source[k];
	 }
      }
   }
}

static boolean insertListItem (CDKSCROLL *scrollp, int item)
{
   int x;
   for (x = scrollp->listSize; x > item; --x)
   {
      scrollp->item[x] = scrollp->item[x - 1];
      scrollp->itemLen[x] = scrollp->itemLen[x - 1];
      scrollp->itemPos[x] = scrollp->itemPos[x - 1];
   }
   return TRUE;
}

/*
 * This adds a single item to a scrolling list, at the end of the list.
 */
void addCDKScrollItem (CDKSCROLL *scrollp, const char *item)
{
   int itemNumber = scrollp->listSize;
   int widestItem = WidestItem (scrollp);
   char *temp = 0;
   size_t have = 0;

   if (allocListArrays (scrollp, scrollp->listSize + 1, FALSE) &&
       allocListItem (scrollp,
		      itemNumber,
		      &temp,
		      &have,
		      scrollp->numbers ? (itemNumber + 1) : 0,
		      item))
   {
      /* Determine the size of the widest item. */
      widestItem = MAXIMUM (scrollp->itemLen[itemNumber], widestItem);

      updateViewWidth (scrollp, widestItem);

      setViewSize (scrollp, scrollp->listSize);
   }

   freeChecked (temp);
}

/*
 * This adds a single item to a scrolling list, before the current item.
 */
void insertCDKScrollItem (CDKSCROLL *scrollp, const char *item)
{
   int widestItem = WidestItem (scrollp);
   char *temp = 0;
   size_t have = 0;

   if (allocListArrays (scrollp, scrollp->listSize + 1, FALSE) &&
       insertListItem (scrollp, scrollp->currentItem) &&
       allocListItem (scrollp,
		      scrollp->currentItem,
		      &temp,
		      &have,
		      scrollp->numbers ? (scrollp->currentItem + 1) : 0,
		      item))
   {
      /* Determine the size of the widest item. */
      widestItem = MAXIMUM (scrollp->itemLen[scrollp->currentItem], widestItem);

      updateViewWidth (scrollp, widestItem);

      setViewSize (scrollp, scrollp->listSize);

      resequence (scrollp);
   }

   freeChecked (temp);
}

/*
 * This removes a single item from a scrolling list.
 */
void deleteCDKScrollItem (CDKSCROLL *scrollp, int position)
{
   if (position >= 0 && position < scrollp->listSize)
   {
      int x;

      freeChtype (scrollp->item[position]);

      /* Adjust the list. */
      for (x = position; x < scrollp->listSize; x++)
      {
	 scrollp->item[x] = scrollp->item[x + 1];
	 scrollp->itemLen[x] = scrollp->itemLen[x + 1];
	 scrollp->itemPos[x] = scrollp->itemPos[x + 1];
      }
      setViewSize (scrollp, scrollp->listSize - 1);

      if (scrollp->listSize > 0)
	 resequence (scrollp);

      if (scrollp->listSize < maxViewSize (scrollp))
	 werase (scrollp->win);	/* force the next redraw to be complete */

      /* do this to update the view size, etc. */
      setCDKScrollPosition (scrollp, scrollp->currentItem);
   }
}

static void _focusCDKScroll (CDKOBJS *object)
{
   CDKSCROLL *scrollp = (CDKSCROLL *)object;

   drawCDKScrollCurrent (scrollp);
   wrefresh (scrollp->listWin);
}

static void _unfocusCDKScroll (CDKOBJS *object)
{
   CDKSCROLL *scrollp = (CDKSCROLL *)object;

   drawCDKScrollCurrent (scrollp);
   wrefresh (scrollp->listWin);
}

dummyRefreshData (Scroll)

dummySaveData (Scroll)
