#include <cdk_int.h>
#include <scroller.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 01:09:14 $
 * $Revision: 1.149 $
 */

/*
 * Declare file local prototypes.
 */
static int createList (CDKRADIO *radio, CDK_CSTRING2 list, int listSize, int width);
static void drawCDKRadioList (CDKRADIO *radio, boolean Box);
static void setViewSize (CDKRADIO *scrollp, int listSize);
static int maxViewSize (CDKRADIO *scrollp);

/* Determine how many characters we can shift to the right */
/* before all the items have been scrolled off the screen. */
#define AvailableWidth(w)  ((w)->boxWidth - 2*BorderOf(w) - 3)
#define updateViewWidth(w, widest) \
	(w)->maxLeftChar = (((w)->boxWidth > widest) \
			      ? 0 \
			      : (widest - AvailableWidth(w)))
#define WidestItem(w)      ((w)->maxLeftChar + AvailableWidth(w))

#define SCREENPOS(w,n) (w)->itemPos[n] - (w)->leftChar + scrollbarAdj + BorderOf(w)

DeclareCDKObjects (RADIO, Radio, setCdk, Int);

/*
 * This function creates the radio widget.
 */
CDKRADIO *newCDKRadio (CDKSCREEN *cdkscreen,
		       int xplace,
		       int yplace,
		       int splace,
		       int height,
		       int width,
		       const char *title,
		       CDK_CSTRING2 list,
		       int listSize,
		       chtype choiceChar,
		       int defItem,
		       chtype highlight,
		       boolean Box,
		       boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKRADIO *radio      = 0;
   int parentWidth      = getmaxx (cdkscreen->window);
   int parentHeight     = getmaxy (cdkscreen->window);
   int boxWidth;
   int boxHeight;
   int xpos             = xplace;
   int ypos             = yplace;
   int widestItem       = 0;
   int j;
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

   if ((radio = newCDKObject (CDKRADIO, &my_funcs)) == 0)
   {
      return (0);
   }

   setCDKRadioBox (radio, Box);

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
   boxWidth = setWidgetDimension (parentWidth, width, 5);

   boxWidth = setCdkTitle (ObjOf (radio), title, boxWidth);

   /* Set the box height. */
   if (TitleLinesOf (radio) > boxHeight)
   {
      boxHeight = TitleLinesOf (radio)
	 + MINIMUM (listSize, 8)
	 + 2 * BorderOf (radio);
   }

   /* Adjust the box width if there is a scroll bar. */
   if (splace == LEFT || splace == RIGHT)
   {
      boxWidth++;
      radio->scrollbar = TRUE;
   }
   else
   {
      radio->scrollbar = FALSE;
   }

   /*
    * Make sure we didn't extend beyond the dimensions of the window.
    */
   radio->boxWidth = MINIMUM (boxWidth, parentWidth);
   radio->boxHeight = MINIMUM (boxHeight, parentHeight);

   setViewSize (radio, listSize);

   /* Each item in the needs to be converted to chtype * */
   widestItem = createList (radio, list, listSize, radio->boxWidth);
   if (widestItem > 0)
   {
      updateViewWidth (radio, widestItem);
   }
   else if (listSize)
   {
      destroyCDKObject (radio);
      return (0);
   }

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, radio->boxWidth, radio->boxHeight);

   /* Make the radio window */
   radio->win = newwin (radio->boxHeight, radio->boxWidth, ypos, xpos);

   /* Is the window null??? */
   if (radio->win == 0)
   {
      destroyCDKObject (radio);
      return (0);
   }

   /* Turn on the keypad. */
   keypad (radio->win, TRUE);

   /* Create the scrollbar window. */
   if (splace == RIGHT)
   {
      radio->scrollbarWin = subwin (radio->win,
				    maxViewSize (radio), 1,
				    SCREEN_YPOS (radio, ypos),
				    (xpos
				     + radio->boxWidth
				     - BorderOf (radio)
				     - 1));
   }
   else if (splace == LEFT)
   {
      radio->scrollbarWin = subwin (radio->win,
				    maxViewSize (radio), 1,
				    SCREEN_YPOS (radio, ypos),
				    SCREEN_XPOS (radio, xpos));
   }
   else
   {
      radio->scrollbarWin = 0;
   }

   /* *INDENT-EQLS* Set the rest of the variables */
   ScreenOf (radio)             = cdkscreen;
   radio->parent                = cdkscreen->window;
   radio->scrollbarPlacement    = splace;
   radio->widestItem            = widestItem;
   radio->leftChar              = 0;
   radio->selectedItem          = 0;
   radio->highlight             = highlight;
   radio->choiceChar            = choiceChar;
   radio->leftBoxChar           = (chtype)'[';
   radio->rightBoxChar          = (chtype)']';
   radio->defItem               = defItem;
   initExitType (radio);
   ObjOf (radio)->inputWindow   = radio->win;
   ObjOf (radio)->acceptsFocus  = TRUE;
   radio->shadow                = shadow;

   setCDKRadioCurrentItem (radio, 0);

   /* Do we need to create the shadow??? */
   if (shadow)
   {
      radio->shadowWin = newwin (boxHeight, boxWidth + 1, ypos + 1, xpos + 1);
   }

   /* Setup the key bindings. */
   for (j = 0; j < (int)SIZEOF (bindings); ++j)
      bindCDKObject (vRADIO,
		     radio,
		     (chtype)bindings[j].from,
		     getcCDKBind,
		     (void *)(long)bindings[j].to);

   /* Register this baby. */
   registerCDKObject (cdkscreen, vRADIO, radio);

   /* Return the radio list */
   return (radio);
}

/*
 * Put the cursor on the currently-selected item.
 */
static void fixCursorPosition (CDKRADIO *widget)
{
   scroller_FixCursorPosition ((CDKSCROLLER *)widget);
}

/*
 * This actually manages the radio widget.
 */
int activateCDKRadio (CDKRADIO *radio, chtype *actions)
{
   /* Draw the radio list. */
   drawCDKRadio (radio, ObjOf (radio)->box);

   if (actions == 0)
   {
      boolean functionKey;

      for (;;)
      {
	 chtype input;
	 int ret;

	 fixCursorPosition (radio);
	 input = (chtype)getchCDKObject (ObjOf (radio), &functionKey);

	 /* Inject the character into the widget. */
	 ret = injectCDKRadio (radio, input);
	 if (radio->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }
   else
   {
      int length = chlen (actions);
      int j;

      /* Inject each character one at a time. */
      for (j = 0; j < length; j++)
      {
	 int ret = injectCDKRadio (radio, actions[j]);
	 if (radio->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }

   /* Set the exit type and return. */
   setExitType (radio, 0);
   return -1;
}

/*
 * This injects a single character into the widget.
 */
static int _injectCDKRadio (CDKOBJS *object, chtype input)
{
   CDKRADIO *radio = (CDKRADIO *)object;
   CDKSCROLLER *widget = (CDKSCROLLER *)object;
   int ppReturn = 1;
   int ret = unknownInt;
   bool complete = FALSE;

   /* Set the exit type. */
   setExitType (widget, 0);

   /* Draw the widget list */
   drawCDKRadioList (radio, ObjOf (widget)->box);

   /* Check if there is a pre-process function to be called. */
   if (PreProcessFuncOf (widget) != 0)
   {
      /* Call the pre-process function. */
      ppReturn = PreProcessFuncOf (widget) (vRADIO,
					    widget,
					    PreProcessDataOf (widget),
					    input);
   }

   /* Should we continue? */
   if (ppReturn != 0)
   {
      /* Check for a predefined key binding. */
      if (checkCDKObjectBind (vRADIO, widget, input) != 0)
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

	 case SPACE:
	    radio->selectedItem = widget->currentItem;
	    break;

	 case KEY_ESC:
	    setExitType (widget, input);
	    ret = -1;
	    complete = TRUE;
	    break;

	 case KEY_ERROR:
	    setExitType (widget, input);
	    complete = TRUE;
	    break;

	 case KEY_TAB:
	 case KEY_ENTER:
	    setExitType (widget, input);
	    ret = radio->selectedItem;
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
	 PostProcessFuncOf (widget) (vRADIO,
				     widget,
				     PostProcessDataOf (widget),
				     input);
      }
   }

   if (!complete)
   {
      drawCDKRadioList (radio, ObjOf (widget)->box);
      setExitType (widget, 0);
   }

   fixCursorPosition (radio);
   ResultOf (widget).valueInt = ret;
   return (ret != unknownInt);
}

/*
 * This moves the radio field to the given location.
 */
static void _moveCDKRadio (CDKOBJS *object,
			   int xplace,
			   int yplace,
			   boolean relative,
			   boolean refresh_flag)
{
   CDKRADIO *radio = (CDKRADIO *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx (radio->win);
   int currentY = getbegy (radio->win);
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
      xpos = getbegx (radio->win) + xplace;
      ypos = getbegy (radio->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (radio), &xpos, &ypos, radio->boxWidth, radio->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (radio->win, -xdiff, -ydiff);
   moveCursesWindow (radio->scrollbarWin, -xdiff, -ydiff);
   moveCursesWindow (radio->shadowWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (radio));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKRadio (radio, ObjOf (radio)->box);
   }
}

static int maxViewSize (CDKRADIO *widget)
{
   return scroller_MaxViewSize ((CDKSCROLLER *)widget);
}

/*
 * Set variables that depend upon the list-size.
 */
static void setViewSize (CDKRADIO *widget, int listSize)
{
   scroller_SetViewSize ((CDKSCROLLER *)widget, listSize);
}

/*
 * This function draws the radio widget.
 */
static void _drawCDKRadio (CDKOBJS *object, boolean Box GCC_UNUSED)
{
   CDKRADIO *radio = (CDKRADIO *)object;

   /* Do we need to draw in the shadow??? */
   if (radio->shadowWin != 0)
   {
      drawShadow (radio->shadowWin);
   }

   drawCdkTitle (radio->win, object);

   /* Draw in the radio list. */
   drawCDKRadioList (radio, ObjOf (radio)->box);
}

/*
 * This redraws the radio list.
 */
static void drawCDKRadioList (CDKRADIO *radio, boolean Box)
{
   int scrollbarAdj = (radio->scrollbarPlacement == LEFT) ? 1 : 0;
   int j, k;

   /* draw the list */
   for (j = 0; j < radio->viewSize; j++)
   {
      int xpos = SCREEN_XPOS (radio, 0);
      int ypos = SCREEN_YPOS (radio, j);

      /* Draw the empty string. */
      writeBlanks (radio->win, xpos, ypos,
		   HORIZONTAL, 0, radio->boxWidth - BorderOf (radio));

      k = j + radio->currentTop;

      /* Draw the element in the radio list. */
      if (k < radio->listSize)
      {
	 int screenPos = SCREENPOS (radio, k);

	 /* Draw the line. */
	 writeChtype (radio->win,
		      (screenPos >= 0) ? screenPos : 1,
		      ypos,
		      radio->item[k],
		      HORIZONTAL,
		      (screenPos >= 0) ? 0 : (1 - screenPos),
		      radio->itemLen[k]);

	 /* Draw the selected choice... */
	 xpos += scrollbarAdj;
	 (void)mvwaddch (radio->win, ypos, xpos++, radio->leftBoxChar);
	 (void)mvwaddch (radio->win, ypos, xpos++, ((k == radio->selectedItem)
						    ? radio->choiceChar
						    : ' '));
	 (void)mvwaddch (radio->win, ypos, xpos++, radio->rightBoxChar);
      }
   }

   /* Highlight the current item. */
   if (ObjPtr (radio)->hasFocus)
   {
      k = radio->currentItem;
      if (k < radio->listSize)
      {
	 int screenPos = SCREENPOS (radio, k);
	 int ypos = SCREEN_YPOS (radio, radio->currentHigh);

	 writeChtypeAttrib (radio->win,
			    (screenPos >= 0) ? screenPos : (1 + scrollbarAdj),
			    ypos,
			    radio->item[k],
			    radio->highlight,
			    HORIZONTAL,
			    (screenPos >= 0) ? 0 : (1 - screenPos),
			    radio->itemLen[k]);
      }
   }

   if (radio->scrollbar)
   {
      radio->togglePos = floorCDK (radio->currentItem * (double)radio->step);
      radio->togglePos = MINIMUM (radio->togglePos,
				  getmaxy (radio->scrollbarWin) - 1);

      (void)mvwvline (radio->scrollbarWin, 0, 0,
		      CDK_FILLED_BOX, getmaxy (radio->scrollbarWin));
      (void)mvwvline (radio->scrollbarWin, radio->togglePos, 0,
		      ' ' | A_REVERSE, radio->toggleSize);
   }

   /* Box it if needed. */
   if (Box)
   {
      drawObjBox (radio->win, ObjOf (radio));
   }
   else
   {
      touchwin (radio->win);
   }

   fixCursorPosition (radio);
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattrRadio (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKRADIO *widget = (CDKRADIO *)object;

      wbkgd (widget->win, attrib);
      if (widget->scrollbarWin != 0)
      {
	 wbkgd (widget->scrollbarWin, attrib);
      }
   }
}

static void destroyInfo (CDKRADIO *widget)
{
   CDKfreeChtypes (widget->item);
   widget->item = 0;

   freeAndNull (widget->itemLen);
   freeAndNull (widget->itemPos);
}

/*
 * This function destroys the radio widget.
 */
static void _destroyCDKRadio (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKRADIO *radio = (CDKRADIO *)object;

      cleanCdkTitle (object);
      destroyInfo (radio);

      /* Clean up the windows. */
      deleteCursesWindow (radio->scrollbarWin);
      deleteCursesWindow (radio->shadowWin);
      deleteCursesWindow (radio->win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vRADIO, radio);

      /* Unregister this object. */
      unregisterCDKObject (vRADIO, radio);
   }
}

/*
 * This function erases the radio widget.
 */
static void _eraseCDKRadio (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKRADIO *radio = (CDKRADIO *)object;

      eraseCursesWindow (radio->win);
      eraseCursesWindow (radio->shadowWin);
   }
}

/*
 * This set various attributes of the radio list.
 */
void setCDKRadio (CDKRADIO *radio, chtype highlight, chtype choiceChar, int Box)
{
   setCDKRadioHighlight (radio, highlight);
   setCDKRadioChoiceCharacter (radio, choiceChar);
   setCDKRadioBox (radio, Box);
}

/*
 * This sets the radio list items.
 */
void setCDKRadioItems (CDKRADIO *radio, CDK_CSTRING2 list, int listSize)
{
   int widestItem;
   int j = 0;

   widestItem = createList (radio, list, listSize, radio->boxWidth);
   if (widestItem <= 0)
      return;

   /* Clean up the display. */
   for (j = 0; j < radio->viewSize; j++)
   {
      writeBlanks (radio->win,
		   SCREEN_XPOS (radio, 0),
		   SCREEN_YPOS (radio, j),
		   HORIZONTAL,
		   0,
		   radio->boxWidth - BorderOf (radio));
   }

   setViewSize (radio, listSize);

   setCDKRadioCurrentItem (radio, 0);
   radio->leftChar = 0;
   radio->selectedItem = 0;

   updateViewWidth (radio, widestItem);
}
int getCDKRadioItems (CDKRADIO *radio, char **list)
{
   if (list != 0)
   {
      int j;

      for (j = 0; j < radio->listSize; j++)
      {
	 list[j] = chtype2Char (radio->item[j]);
      }
   }
   return radio->listSize;
}

/*
 * This sets the highlight bar of the radio list.
 */
void setCDKRadioHighlight (CDKRADIO *radio, chtype highlight)
{
   radio->highlight = highlight;
}
chtype getCDKRadioHighlight (CDKRADIO *radio)
{
   return radio->highlight;
}

/*
 * This sets the character to use when selecting an item in the list.
 */
void setCDKRadioChoiceCharacter (CDKRADIO *radio, chtype character)
{
   radio->choiceChar = character;
}
chtype getCDKRadioChoiceCharacter (CDKRADIO *radio)
{
   return radio->choiceChar;
}

/*
 * This sets the character to use to draw the left side of the
 * choice box on the list.
 */
void setCDKRadioLeftBrace (CDKRADIO *radio, chtype character)
{
   radio->leftBoxChar = character;
}
chtype getCDKRadioLeftBrace (CDKRADIO *radio)
{
   return radio->leftBoxChar;
}

/*
 * This sets the character to use to draw the right side of the
 * choice box on the list.
 */
void setCDKRadioRightBrace (CDKRADIO *radio, chtype character)
{
   radio->rightBoxChar = character;
}
chtype getCDKRadioRightBrace (CDKRADIO *radio)
{
   return radio->rightBoxChar;
}

/*
 * This sets the box attribute of the widget.
 */
void setCDKRadioBox (CDKRADIO *radio, boolean Box)
{
   ObjOf (radio)->box = Box;
   ObjOf (radio)->borderSize = Box ? 1 : 0;
}
boolean getCDKRadioBox (CDKRADIO *radio)
{
   return ObjOf (radio)->box;
}

/*
 * This sets the current high lighted item of the widget
 */
void setCDKRadioCurrentItem (CDKRADIO *radio, int item)
{
   scroller_SetPosition ((CDKSCROLLER *)radio, item);
   radio->selectedItem = item;
}
int getCDKRadioCurrentItem (CDKRADIO *radio)
{
   return radio->currentItem;
}

/*
 * This sets the selected item of the widget
 */
void setCDKRadioSelectedItem (CDKRADIO *radio, int item)
{
   radio->selectedItem = item;
}
int getCDKRadioSelectedItem (CDKRADIO *radio)
{
   return radio->selectedItem;
}

static void _focusCDKRadio (CDKOBJS *object)
{
   CDKRADIO *radio = (CDKRADIO *)object;

   drawCDKRadioList (radio, ObjOf (radio)->box);
}

static void _unfocusCDKRadio (CDKOBJS *object)
{
   CDKRADIO *radio = (CDKRADIO *)object;

   drawCDKRadioList (radio, ObjOf (radio)->box);
}

static int createList (CDKRADIO *radio, CDK_CSTRING2 list, int listSize, int boxWidth)
{
   int status = 0;
   int widestItem = 0;

   if (radio->listSize > 0)
   {
      CDKfreeChtypes (radio->item);
      radio->item = 0;
      freeAndNull (radio->itemLen);
      freeAndNull (radio->itemPos);
   }

   if (listSize >= 0)
   {
      /* *INDENT-EQLS* */
      chtype **newList = typeCallocN (chtype *, listSize + 1);
      int *newLen      = typeCallocN (int, listSize + 1);
      int *newPos      = typeCallocN (int, listSize + 1);

      if (newList != 0
	  && newLen != 0
	  && newPos != 0)
      {
	 int j;

	 /* Each item in the needs to be converted to chtype * */
	 status = 1;
	 boxWidth -= (2 + BorderOf (radio));
	 for (j = 0; j < listSize; j++)
	 {
	    newList[j] = char2Chtype (list[j], &newLen[j], &newPos[j]);
	    if (newList[j] == 0)
	    {
	       status = 0;
	       break;
	    }
	    newPos[j] = justifyString (boxWidth, newLen[j], newPos[j]) + 3;
	    widestItem = MAXIMUM (widestItem, newLen[j]);
	 }
	 if (status)
	 {
	    destroyInfo (radio);

	    /* *INDENT-EQLS* */
	    radio->listSize = listSize;
	    radio->item     = newList;
	    radio->itemLen  = newLen;
	    radio->itemPos  = newPos;
	 }
	 else
	 {
	    CDKfreeChtypes (newList);
	    freeChecked (newLen);
	    freeChecked (newPos);
	 }
      }
      else
      {
	 CDKfreeChtypes (newList);
	 freeChecked (newLen);
	 freeChecked (newPos);
      }
   }
   else
   {
      destroyInfo (radio);
   }

   return status ? widestItem : 0;
}

dummyRefreshData (Radio)

dummySaveData (Radio)
