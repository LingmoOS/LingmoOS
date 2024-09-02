#include <cdk_int.h>
#include <scroller.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 01:09:14 $
 * $Revision: 1.159 $
 */

/*
 * Declare file local prototypes.
 */
static int createList (CDKSELECTION *selection, CDK_CSTRING2 list, int listSize);
static void drawCDKSelectionList (CDKSELECTION *selection, boolean Box);
static void setViewSize (CDKSELECTION *scrollp, int listSize);
static int maxViewSize (CDKSELECTION *scrollp);

/* Determine how many characters we can shift to the right */
/* before all the items have been scrolled off the screen. */
#define AvailableWidth(w)  ((w)->boxWidth - 2*BorderOf(w) - (w)->maxchoicelen)
#define updateViewWidth(w, widest) \
	(w)->maxLeftChar = (((w)->boxWidth > widest) \
			      ? 0 \
			      : (widest - AvailableWidth(w)))
#define WidestItem(w)      ((w)->maxLeftChar + AvailableWidth(w))

#define SCREENPOS(w,n) (w)->itemPos[n] - (w)->leftChar + scrollbarAdj	/* + BorderOf(w) */

DeclareCDKObjects (SELECTION, Selection, setCdk, Int);

/*
 * This function creates a selection widget.
 */
CDKSELECTION *newCDKSelection (CDKSCREEN *cdkscreen,
			       int xplace,
			       int yplace,
			       int splace,
			       int height,
			       int width,
			       const char *title,
			       CDK_CSTRING2 list,
			       int listSize,
			       CDK_CSTRING2 choices,
			       int choiceCount,
			       chtype highlight,
			       boolean Box,
			       boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKSELECTION *selection      = 0;
   int widestItem               = -1;
   int parentWidth              = getmaxx (cdkscreen->window);
   int parentHeight             = getmaxy (cdkscreen->window);
   int boxWidth;
   int boxHeight;
   int xpos                     = xplace;
   int ypos                     = yplace;
   int j                        = 0;
   int junk2;
   /* *INDENT-OFF* */
   static const struct { int from; int to; } bindings[] = {
		{ CDK_BACKCHAR, KEY_PPAGE },
		{ CDK_FORCHAR,	KEY_NPAGE },
		{ 'g',		KEY_HOME },
		{ '1',		KEY_HOME },
		{ 'G',		KEY_END },
		{ '<',		KEY_HOME },
		{ '>',		KEY_END },
   };
   /* *INDENT-ON* */

   if (choiceCount <= 0
       || (selection = newCDKObject (CDKSELECTION, &my_funcs)) == 0
       || (selection->choice = typeCallocN (chtype *, choiceCount + 1)) == 0
       || (selection->choicelen = typeCallocN (int, choiceCount + 1)) == 0)
   {
      destroyCDKObject (selection);
      return (0);
   }

   setCDKSelectionBox (selection, Box);

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

   boxWidth = setCdkTitle (ObjOf (selection), title, boxWidth);

   /* Set the box height. */
   if (TitleLinesOf (selection) > boxHeight)
   {
      boxHeight = TitleLinesOf (selection)
	 + MINIMUM (listSize, 8)
	 + 2 * BorderOf (selection);
   }

   selection->maxchoicelen = 0;

   /* Adjust the box width if there is a scroll bar. */
   if (splace == LEFT || splace == RIGHT)
   {
      boxWidth++;
      selection->scrollbar = TRUE;
   }
   else
   {
      selection->scrollbar = FALSE;
   }

   /*
    * Make sure we didn't extend beyond the dimensions of the window.
    */
   selection->boxWidth = (boxWidth > parentWidth ? parentWidth : boxWidth);
   selection->boxHeight = (boxHeight > parentHeight ? parentHeight : boxHeight);

   setViewSize (selection, listSize);

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, selection->boxWidth, selection->boxHeight);

   /* Make the selection window */
   selection->win = newwin (selection->boxHeight, selection->boxWidth, ypos, xpos);

   /* Is the window null?? */
   if (selection->win == 0)
   {
      destroyCDKObject (selection);
      return (0);
   }

   /* Turn the keypad on for this window. */
   keypad (selection->win, TRUE);

   /* Create the scrollbar window. */
   if (splace == RIGHT)
   {
      selection->scrollbarWin = subwin (selection->win,
					maxViewSize (selection), 1,
					SCREEN_YPOS (selection, ypos),
					(xpos
					 + selection->boxWidth
					 - BorderOf (selection)
					 - 1));
   }
   else if (splace == LEFT)
   {
      selection->scrollbarWin = subwin (selection->win,
					maxViewSize (selection), 1,
					SCREEN_YPOS (selection, ypos),
					SCREEN_XPOS (selection, xpos));
   }
   else
   {
      selection->scrollbarWin = 0;
   }

   /* *INDENT-EQLS* Set the rest of the variables */
   ScreenOf (selection)                 = cdkscreen;
   selection->parent                    = cdkscreen->window;
   selection->scrollbarPlacement        = splace;
   selection->maxLeftChar               = 0;
   selection->leftChar                  = 0;
   selection->highlight                 = highlight;
   selection->choiceCount               = choiceCount;
   initExitType (selection);
   ObjOf (selection)->acceptsFocus      = TRUE;
   ObjOf (selection)->inputWindow       = selection->win;
   selection->shadow                    = shadow;

   setCDKSelectionCurrent (selection, 0);

   /* Each choice has to be converted from char * to chtype * */
   for (j = 0; j < choiceCount; j++)
   {
      selection->choice[j] = char2Chtype (choices[j],
					  &selection->choicelen[j],
					  &junk2);
      selection->maxchoicelen = MAXIMUM (selection->maxchoicelen,
					 selection->choicelen[j]);
   }

   /* Each item in the needs to be converted to chtype * */
   widestItem = createList (selection, list, listSize);
   if (widestItem > 0)
   {
      updateViewWidth (selection, widestItem);
   }
   else if (listSize)
   {
      destroyCDKObject (selection);
      return (0);
   }

   /* Do we need to create a shadow. */
   if (shadow)
   {
      selection->shadowWin = newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   /* Setup the key bindings. */
   for (j = 0; j < (int)SIZEOF (bindings); ++j)
      bindCDKObject (vSELECTION,
		     selection,
		     (chtype)bindings[j].from,
		     getcCDKBind,
		     (void *)(long)bindings[j].to);

   /* Register this baby. */
   registerCDKObject (cdkscreen, vSELECTION, selection);

   /* Return the selection list */
   return (selection);
}

/*
 * Put the cursor on the currently-selected item.
 */
static void fixCursorPosition (CDKSELECTION *selection)
{
   scroller_FixCursorPosition ((CDKSCROLLER *)selection);
}

/*
 * This actually manages the selection widget...
 */
int activateCDKSelection (CDKSELECTION *selection, chtype *actions)
{
   /* Draw the selection list */
   drawCDKSelection (selection, ObjOf (selection)->box);

   if (actions == 0)
   {
      boolean functionKey;

      for (;;)
      {
	 chtype input;
	 int ret;

	 fixCursorPosition (selection);
	 input = (chtype)getchCDKObject (ObjOf (selection), &functionKey);

	 /* Inject the character into the widget. */
	 ret = injectCDKSelection (selection, input);
	 if (selection->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }
   else
   {
      int length = chlen (actions);
      int j = 0;

      /* Inject each character one at a time. */
      for (j = 0; j < length; j++)
      {
	 int ret = injectCDKSelection (selection, actions[j]);
	 if (selection->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }

   /* Set the exit type and return. */
   setExitType (selection, 0);
   return 0;
}

/*
 * This injects a single character into the widget.
 */
static int _injectCDKSelection (CDKOBJS *object, chtype input)
{
   CDKSELECTION *selection = (CDKSELECTION *)object;
   CDKSCROLLER *widget = (CDKSCROLLER *)object;
   int ppReturn = 1;
   int ret = unknownInt;
   bool complete = FALSE;

   /* Set the exit type. */
   setExitType (widget, 0);

   /* Draw the widget list */
   drawCDKSelectionList (selection, ObjOf (widget)->box);

   /* Check if there is a pre-process function to be called. */
   if (PreProcessFuncOf (widget) != 0)
   {
      /* Call the pre-process function. */
      ppReturn = PreProcessFuncOf (widget) (vSELECTION,
					    widget,
					    PreProcessDataOf (widget),
					    input);
   }

   /* Should we continue? */
   if (ppReturn != 0)
   {
      /* Check for a predefined binding. */
      if (checkCDKObjectBind (vSELECTION, widget, input) != 0)
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
	    if (selection->mode[widget->currentItem] == 0)
	    {
	       if (selection->selections[widget->currentItem]
		   == (selection->choiceCount - 1))
	       {
		  selection->selections[widget->currentItem] = 0;
	       }
	       else
	       {
		  selection->selections[widget->currentItem]++;
	       }
	    }
	    else
	    {
	       Beep ();
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

	 case KEY_TAB:
	 case KEY_ENTER:
	    setExitType (widget, input);
	    ret = 1;
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
	 PostProcessFuncOf (widget) (vSELECTION,
				     widget,
				     PostProcessDataOf (widget),
				     input);
      }
   }

   if (!complete)
   {
      drawCDKSelectionList (selection, ObjOf (widget)->box);
      setExitType (widget, 0);
   }

   ResultOf (widget).valueInt = ret;
   fixCursorPosition (selection);
   return (ret != unknownInt);
}

/*
 * This moves the selection field to the given location.
 */
static void _moveCDKSelection (CDKOBJS *object,
			       int xplace,
			       int yplace,
			       boolean relative,
			       boolean refresh_flag)
{
   CDKSELECTION *selection = (CDKSELECTION *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx (selection->win);
   int currentY = getbegy (selection->win);
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
      xpos = getbegx (selection->win) + xplace;
      ypos = getbegy (selection->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (selection), &xpos, &ypos, selection->boxWidth, selection->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (selection->win, -xdiff, -ydiff);
   moveCursesWindow (selection->scrollbarWin, -xdiff, -ydiff);
   moveCursesWindow (selection->shadowWin, -xdiff, -ydiff);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (selection));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKSelection (selection, ObjOf (selection)->box);
   }
}

/*
 * This function draws the selection list.
 */
static void _drawCDKSelection (CDKOBJS *object, boolean Box)
{
   CDKSELECTION *selection = (CDKSELECTION *)object;

   /* Draw in the shadow if we need to. */
   if (selection->shadowWin != 0)
   {
      drawShadow (selection->shadowWin);
   }

   drawCdkTitle (selection->win, object);

   /* Redraw the list */
   drawCDKSelectionList (selection, Box);
}

static int maxViewSize (CDKSELECTION *widget)
{
   return scroller_MaxViewSize ((CDKSCROLLER *)widget);
}

/*
 * Set variables that depend upon the list-size.
 */
static void setViewSize (CDKSELECTION *widget, int listSize)
{
   scroller_SetViewSize ((CDKSCROLLER *)widget, listSize);
}

/*
 * This function draws the selection list window.
 */
static void drawCDKSelectionList (CDKSELECTION *selection, boolean Box GCC_UNUSED)
{
   /* *INDENT-EQLS* */
   int scrollbarAdj     = (selection->scrollbarPlacement == LEFT) ? 1 : 0;
   int screenPos        = 0;
   int j;
   int selItem          = -1;

   /* If there is to be a highlight, assign it now */
   if (ObjOf (selection)->hasFocus)
      selItem = selection->currentItem;

   /* draw the list... */
   for (j = 0; j < selection->viewSize; j++)
   {
      int xpos = SCREEN_XPOS (selection, 0);
      int ypos = SCREEN_YPOS (selection, j);
      int k;

      /* Draw the empty line. */
      writeBlanks (selection->win, xpos, ypos,
		   HORIZONTAL, 0, getmaxx (selection->win));

      k = j + selection->currentTop;

      /* Draw the element in the selection list. */
      if (k < selection->listSize)
      {
	 screenPos = SCREENPOS (selection, k);

	 /* Draw the selection item. */
	 writeChtypeAttrib (selection->win,
			    (screenPos >= 0) ? screenPos : 1,
			    ypos,
			    selection->item[k],
			    ((k == selItem)
			     ? selection->highlight
			     : A_NORMAL),
			    HORIZONTAL,
			    (screenPos >= 0) ? 0 : (1 - screenPos),
			    selection->itemLen[k]);

	 /* Draw the choice value. */
	 writeChtype (selection->win,
		      xpos + scrollbarAdj,
		      ypos,
		      selection->choice[selection->selections[k]],
		      HORIZONTAL,
		      0,
		      selection->choicelen[selection->selections[k]]);
      }
   }

   /* Determine where the toggle is supposed to be. */
   if (selection->scrollbar)
   {
      selection->togglePos = floorCDK (selection->currentItem * (double)selection->step);
      selection->togglePos = MINIMUM (selection->togglePos,
				      (getmaxy (selection->scrollbarWin) - 1));

      (void)mvwvline (selection->scrollbarWin, 0, 0, CDK_FILLED_BOX, getmaxy (selection->scrollbarWin));
      (void)mvwvline (selection->scrollbarWin,
		      selection->togglePos,
		      0,
		      ' ' | A_REVERSE, selection->toggleSize);
   }

   /* Box it if needed */
   if (ObjOf (selection)->box)
   {
      drawObjBox (selection->win, ObjOf (selection));
   }

   fixCursorPosition (selection);
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattrSelection (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKSELECTION *widget = (CDKSELECTION *)object;

      wbkgd (widget->win, attrib);
      if (widget->scrollbarWin != 0)
      {
	 wbkgd (widget->scrollbarWin, attrib);
      }
   }
}

static void destroyInfo (CDKSELECTION *widget)
{
   CDKfreeChtypes (widget->item);
   widget->item = 0;

   freeAndNull (widget->itemPos);
   freeAndNull (widget->itemLen);
   freeAndNull (widget->selections);
   freeAndNull (widget->mode);
}

/*
 * This function destroys the selection list.
 */
static void _destroyCDKSelection (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKSELECTION *selection = (CDKSELECTION *)object;

      cleanCdkTitle (object);
      CDKfreeChtypes (selection->choice);
      freeChecked (selection->choicelen);
      destroyInfo (selection);

      /* Clean up the windows. */
      deleteCursesWindow (selection->scrollbarWin);
      deleteCursesWindow (selection->shadowWin);
      deleteCursesWindow (selection->win);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vSELECTION, selection);

      /* Unregister this object. */
      unregisterCDKObject (vSELECTION, selection);
   }
}

/*
 * This function erases the selection list from the screen.
 */
static void _eraseCDKSelection (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKSELECTION *selection = (CDKSELECTION *)object;

      eraseCursesWindow (selection->win);
      eraseCursesWindow (selection->shadowWin);
   }
}

/*
 * This function sets a couple of the selection list attributes.
 */
void setCDKSelection (CDKSELECTION *selection,
		      chtype highlight,
		      int *choices,
		      boolean Box)
{
   setCDKSelectionChoices (selection, choices);
   setCDKSelectionHighlight (selection, highlight);
   setCDKSelectionBox (selection, Box);
}

/*
 * This sets the selection list items.
 */
void setCDKSelectionItems (CDKSELECTION *selection, CDK_CSTRING2 list, int listSize)
{
   int widestItem;
   int j = 0;

   widestItem = createList (selection, list, listSize);
   if (widestItem <= 0)
      return;

   /* Clean up the display. */
   for (j = 0; j < selection->viewSize; j++)
   {
      writeBlanks (selection->win,
		   SCREEN_XPOS (selection, 0),
		   SCREEN_YPOS (selection, j),
		   HORIZONTAL, 0, getmaxx (selection->win));
   }

   setViewSize (selection, listSize);
   setCDKSelectionCurrent (selection, 0);

   updateViewWidth (selection, widestItem);
}
int getCDKSelectionItems (CDKSELECTION *selection, char **list)
{
   if (list != 0)
   {
      int j;

      for (j = 0; j < selection->listSize; j++)
      {
	 list[j] = chtype2Char (selection->item[j]);
      }
   }
   return selection->listSize;
}

/*
 *
 */
void setCDKSelectionTitle (CDKSELECTION *selection, const char *title)
{
   /* Make sure the title isn't null. */
   if (title == 0)
   {
      return;
   }

   (void)setCdkTitle (ObjOf (selection), title, -(selection->boxWidth + 1));

   setViewSize (selection, selection->listSize);
}
char *getCDKSelectionTitle (CDKSELECTION *selection GCC_UNUSED)
{
   return chtype2Char (*TitleOf (selection));
}

/*
 * This sets the highlight bar.
 */
void setCDKSelectionHighlight (CDKSELECTION *selection, chtype highlight)
{
   selection->highlight = highlight;
}
chtype getCDKSelectionHighlight (CDKSELECTION *selection)
{
   return selection->highlight;
}

/*
 * This sets the default choices for the selection list.
 */
void setCDKSelectionChoices (CDKSELECTION *selection, int *choices)
{
   int j;

   /* Set the choice values in the selection list. */
   for (j = 0; j < selection->listSize; j++)
   {
      if (choices[j] < 0)
      {
	 selection->selections[j] = 0;
      }
      else if (choices[j] > selection->choiceCount)
      {
	 selection->selections[j] = selection->choiceCount - 1;
      }
      else
      {
	 selection->selections[j] = choices[j];
      }
   }
}
int *getCDKSelectionChoices (CDKSELECTION *selection)
{
   return selection->selections;
}

/*
 * This sets a single item's choice value.
 */
void setCDKSelectionChoice (CDKSELECTION *selection, int Index, int choice)
{
   int correctChoice = choice;
   int correctIndex = Index;

   /* Verify that the choice value is in range. */
   if (choice < 0)
   {
      correctChoice = 0;
   }
   else if (choice > selection->choiceCount)
   {
      correctChoice = selection->choiceCount - 1;
   }

   /* Make sure the index isn't out of range. */
   if (correctIndex < 0)
   {
      correctIndex = 0;
   }
   else if (correctIndex > selection->listSize)
   {
      correctIndex = selection->listSize - 1;
   }

   /* Set the choice value. */
   selection->selections[correctIndex] = correctChoice;
}
int getCDKSelectionChoice (CDKSELECTION *selection, int Index)
{
   /* Make sure the index isn't out of range. */
   if (Index < 0)
   {
      return selection->selections[0];
   }
   else if (Index > selection->listSize)
   {
      return selection->selections[selection->listSize - 1];
   }
   else
   {
      return selection->selections[Index];
   }
}

/*
 * This sets the modes of the items in the selection list. Currently
 * there are only two: editable=0 and read-only=1
 */
void setCDKSelectionModes (CDKSELECTION *selection, int *modes)
{
   int j;

   /* Make sure the widget pointer is not null. */
   if (selection == 0)
   {
      return;
   }

   /* Set the modes. */
   for (j = 0; j < selection->listSize; j++)
   {
      selection->mode[j] = modes[j];
   }
}
int *getCDKSelectionModes (CDKSELECTION *selection)
{
   return selection->mode;
}

/*
 * This sets a single mode of an item in the selection list.
 */
void setCDKSelectionMode (CDKSELECTION *selection, int Index, int mode)
{
   /* Make sure the widget pointer is not null. */
   if (selection == 0)
   {
      return;
   }

   /* Make sure the index isn't out of range. */
   if (Index < 0)
   {
      selection->mode[0] = mode;
   }
   else if (Index > selection->listSize)
   {
      selection->mode[selection->listSize - 1] = mode;
   }
   else
   {
      selection->mode[Index] = mode;
   }
}
int getCDKSelectionMode (CDKSELECTION *selection, int Index)
{
   /* Make sure the index isn't out of range. */
   if (Index < 0)
   {
      return selection->mode[0];
   }
   else if (Index > selection->listSize)
   {
      return selection->mode[selection->listSize - 1];
   }
   else
   {
      return selection->mode[Index];
   }
}

/*
 * This sets the box attribute of the widget.
 */
void setCDKSelectionBox (CDKSELECTION *selection, boolean Box)
{
   ObjOf (selection)->box = Box;
   ObjOf (selection)->borderSize = Box ? 1 : 0;
}
boolean getCDKSelectionBox (CDKSELECTION *selection)
{
   return ObjOf (selection)->box;
}

/*
 * set/get the current item index
 */
void setCDKSelectionCurrent (CDKSELECTION *selection, int item)
{
   scroller_SetPosition ((CDKSCROLLER *)selection, item);
}

int getCDKSelectionCurrent (CDKSELECTION *selection)
{
   return selection->currentItem;
}

/*
 * methods for generic type methods
 */
static void _focusCDKSelection (CDKOBJS *object)
{
   CDKSELECTION *selection = (CDKSELECTION *)object;

   drawCDKSelectionList (selection, ObjOf (selection)->box);
}

static void _unfocusCDKSelection (CDKOBJS *object)
{
   CDKSELECTION *selection = (CDKSELECTION *)object;

   drawCDKSelectionList (selection, ObjOf (selection)->box);
}

static int createList (CDKSELECTION *selection, CDK_CSTRING2 list, int listSize)
{
   int status = 0;
   int widestItem = 0;

   if (selection->listSize > 0)
   {
      CDKfreeChtypes (selection->item);
      selection->item = 0;
      freeAndNull (selection->itemLen);
      freeAndNull (selection->itemPos);
      freeAndNull (selection->selections);
      freeAndNull (selection->mode);
   }

   if (listSize >= 0)
   {
      /* *INDENT-EQLS* */
      chtype **newList  = typeCallocN (chtype *, listSize + 1);
      int *newLen       = typeCallocN (int, listSize + 1);
      int *newPos       = typeCallocN (int, listSize + 1);
      int *newSel       = typeCallocN (int, listSize + 1);
      int *newMode      = typeCallocN (int, listSize + 1);

      if (newList != 0
	  && newLen != 0
	  && newPos != 0
	  && newSel != 0
	  && newMode != 0)
      {
	 int boxWidth = AvailableWidth (selection);
	 int adjust = selection->maxchoicelen + BorderOf (selection);
	 int j;

	 status = 1;
	 for (j = 0; j < listSize; j++)
	 {
	    newList[j] = char2Chtype (list[j], &newLen[j], &newPos[j]);
	    if (newList[j] == 0)
	    {
	       status = 0;
	       break;
	    }
	    newPos[j] = justifyString (boxWidth, newLen[j], newPos[j]) + adjust;
	    widestItem = MAXIMUM (widestItem, newLen[j]);
	 }

	 if (status)
	 {
	    destroyInfo (selection);

	    /* *INDENT-EQLS* */
	    selection->listSize   = listSize;
	    selection->item       = newList;
	    selection->itemPos    = newPos;
	    selection->itemLen    = newLen;
	    selection->selections = newSel;
	    selection->mode       = newMode;
	 }
	 else
	 {
	    CDKfreeChtypes (newList);
	    freeChecked (newPos);
	    freeChecked (newLen);
	    freeChecked (newSel);
	    freeChecked (newMode);
	 }
      }
      else
      {
	 CDKfreeChtypes (newList);
	 freeChecked (newPos);
	 freeChecked (newLen);
	 freeChecked (newSel);
	 freeChecked (newMode);
      }
   }
   else
   {
      destroyInfo (selection);
   }

   return status ? widestItem : 0;
}

dummyRefreshData (Selection)

dummySaveData (Selection)
