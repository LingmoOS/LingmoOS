#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 00:45:55 $
 * $Revision: 1.106 $
 */

#define TITLELINES 1

/*
 * Declare file local prototypes.
 */
static void cleanUpMenu (CDKMENU *menu);

DeclareCDKObjects (MENU, Menu, setCdk, Int);

/*
 * This creates a new menu widget.
 */
CDKMENU *newCDKMenu (CDKSCREEN *cdkscreen,
		     const char *menulist[MAX_MENU_ITEMS][MAX_SUB_ITEMS],
		     int menuItems,
		     int *subsize,
		     int *menuLocation,
		     int menuPos,
		     chtype titleAttr,
		     chtype subtitleAttr)
{
   /* *INDENT-EQLS* */
   CDKMENU *menu        = 0;
   int rightcount;
   int rightloc         = getmaxx (cdkscreen->window);
   int leftloc          = 0;
   int x, y, junk;
   int xpos             = getbegx (cdkscreen->window);
   int ypos             = getbegy (cdkscreen->window);
   int ymax             = getmaxy (cdkscreen->window);

   if ((menu = newCDKObject (CDKMENU, &my_funcs)) == 0)
        return (0);

   /* *INDENT-EQLS* Start making a copy of the information. */
   ScreenOf (menu)              = cdkscreen;
   ObjOf (menu)->box            = FALSE;
   ObjOf (menu)->acceptsFocus   = FALSE;
   rightcount                   = menuItems - 1;
   menu->parent                 = cdkscreen->window;
   menu->menuItems              = menuItems;
   menu->titleAttr              = titleAttr;
   menu->subtitleAttr           = subtitleAttr;
   menu->currentTitle           = 0;
   menu->currentSubtitle        = 0;
   menu->lastSelection          = -1;
   menu->menuPos                = menuPos;
   initExitType (menu);

   /* Create the pull down menus. */
   for (x = 0; x < menuItems; x++)
   {
      /* *INDENT-EQLS* */
      int x1   = (menuLocation[x] == LEFT) ? x : rightcount--;
      int x2;
      int y1   = (menuPos == BOTTOM) ? (ymax - 1) : 0;
      int y2   = (menuPos == BOTTOM) ? (ymax - subsize[x] - 2) : TITLELINES;
      int high = subsize[x] + TITLELINES;
      int max  = -1;

      /*
       * Limit the menu height to fit on the screen.
       */
      if (high + y2 > ymax)
      {
	 high = ymax - TITLELINES;
      }

      max = -1;
      for (y = TITLELINES; y < subsize[x]; y++)
      {
	 int y0 = y - TITLELINES;

	 menu->sublist[x1][y0] = char2Chtype (menulist[x][y],
					      &menu->sublistLen[x1][y0],
					      &junk);
	 max = MAXIMUM (max, menu->sublistLen[x1][y0]);
      }

      if (menuLocation[x] == LEFT)
      {
	 x2 = leftloc;
      }
      else
      {
	 x2 = (rightloc -= max + 2);
      }
      /* *INDENT-EQLS* */
      menu->title[x1]    = char2Chtype (menulist[x][0], &menu->titleLen[x1], &junk);
      menu->subsize[x1]  = subsize[x] - TITLELINES;
      menu->titleWin[x1] = subwin (cdkscreen->window,
				   TITLELINES,
				   menu->titleLen[x1] + 2,
				   ypos               + y1,
				   xpos               + x2);
      menu->pullWin[x1]  = subwin (cdkscreen->window,
				   high,
				   max                + 2,
				   ypos               + y2,
				   xpos               + x2);
      if (menu->titleWin[x1] == 0 || menu->pullWin[x1] == 0)
      {
	 destroyCDKMenu (menu);
	 return (0);
      }

      leftloc += menu->titleLen[x] + 1;

      keypad (menu->titleWin[x1], TRUE);
      keypad (menu->pullWin[x1], TRUE);
   }
   ObjOf (menu)->inputWindow = menu->titleWin[menu->currentTitle];

   /* Register this baby. */
   registerCDKObject (cdkscreen, vMENU, menu);

   /* Return the menu object. */
   return (menu);
}

/*
 * This activates the CDK Menu.
 */
int activateCDKMenu (CDKMENU *menu, chtype *actions)
{
   boolean functionKey;
   int ret;

   /* Draw in the screen. */
   refreshCDKScreen (ScreenOf (menu));

   /* Display the menu titles. */
   drawCDKMenu (menu, ObjOf (menu)->box);

   /* Highlight the current title and window. */
   drawCDKMenuSubwin (menu);

   /* If the input string is null, this is an interactive activate. */
   if (actions == 0)
   {
      ObjOf (menu)->inputWindow = menu->titleWin[menu->currentTitle];

      /* Start taking input from the keyboard. */
      for (;;)
      {
	 chtype input = (chtype)getchCDKObject (ObjOf (menu), &functionKey);

	 /* Inject the character into the widget. */
	 ret = injectCDKMenu (menu, input);
	 if (menu->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }
   else
   {
      int count = chlen (actions);
      int x = 0;

      for (x = 0; x < count; x++)
      {
	 ret = injectCDKMenu (menu, actions[x]);
	 if (menu->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }

   /* Set the exit type and return. */
   setExitType (menu, 0);
   return -1;
}

/*
 * The "%" operator is simpler but does not handle negative values.
 */
static int wrapped (int within, int limit)
{
   if (within < 0)
      within = limit - 1;
   else if (within >= limit)
      within = 0;
   return within;
}

static void drawTitle (CDKMENU *menu, int item)
{
   writeChtype (menu->titleWin[item],
		0, 0, menu->title[item],
		HORIZONTAL,
		0, menu->titleLen[item]);
}

static void drawItem (CDKMENU *menu, int item, int offset)
{
   writeChtype (menu->pullWin[menu->currentTitle],
		1, item + TITLELINES - offset,
		menu->sublist[menu->currentTitle][item],
		HORIZONTAL,
		0, menu->sublistLen[menu->currentTitle][item]);
}

/* Highlight the current sub-menu item. */
static void selectItem (CDKMENU *menu, int item, int offset)
{
   writeChtypeAttrib (menu->pullWin[menu->currentTitle],
		      1, item + TITLELINES - offset,
		      menu->sublist[menu->currentTitle][item],
		      menu->subtitleAttr,
		      HORIZONTAL,
		      0, menu->sublistLen[menu->currentTitle][item]);
}

static void withinSubmenu (CDKMENU *menu, int step)
{
   int next = wrapped (menu->currentSubtitle + step, menu->subsize[menu->currentTitle]);

   if (next != menu->currentSubtitle)
   {
      CDKSCREEN *screen = ScreenOf (menu);
      int ymax = getmaxy (screen->window);

      if ((1 +
	   getbegy (menu->pullWin[menu->currentTitle]) +
	   menu->subsize[menu->currentTitle]) >= ymax)
      {
	 menu->currentSubtitle = next;
	 drawCDKMenuSubwin (menu);
      }
      else
      {
	 /* Erase the old subtitle. */
	 drawItem (menu, menu->currentSubtitle, 0);

	 /* Set the values. */
	 menu->currentSubtitle = next;

	 /* Draw the new sub-title. */
	 selectItem (menu, menu->currentSubtitle, 0);

	 wrefresh (menu->pullWin[menu->currentTitle]);
      }

      ObjOf (menu)->inputWindow = menu->titleWin[menu->currentTitle];
   }
}

static void acrossSubmenus (CDKMENU *menu, int step)
{
   int next = wrapped (menu->currentTitle + step, menu->menuItems);

   if (next != menu->currentTitle)
   {
      /* Erase the menu sub-window. */
      eraseCDKMenuSubwin (menu);
      refreshCDKScreen (ScreenOf (menu));

      /* Set the values. */
      menu->currentTitle = next;
      menu->currentSubtitle = 0;

      /* Draw the new menu sub-window. */
      drawCDKMenuSubwin (menu);
      ObjOf (menu)->inputWindow = menu->titleWin[menu->currentTitle];
   }
}

/*
 * Inject a character into the menu widget.
 */
static int _injectCDKMenu (CDKOBJS *object, chtype input)
{
   CDKMENU *widget = (CDKMENU *)object;
   int ppReturn = 1;
   int ret = unknownInt;
   bool complete = FALSE;

   /* Set the exit type. */
   setExitType (widget, 0);

   /* Check if there is a pre-process function to be called. */
   if (PreProcessFuncOf (widget) != 0)
   {
      /* Call the pre-process function. */
      ppReturn = PreProcessFuncOf (widget) (vMENU,
					    widget,
					    PreProcessDataOf (widget),
					    input);
   }

   /* Should we continue? */
   if (ppReturn != 0)
   {
      /* Check for key bindings. */
      if (checkCDKObjectBind (vMENU, widget, input) != 0)
      {
	 checkEarlyExit (widget);
	 complete = TRUE;
      }
      else
      {
	 switch (input)
	 {
	 case KEY_LEFT:
	    acrossSubmenus (widget, -1);
	    break;

	 case KEY_RIGHT:
	 case KEY_TAB:
	    acrossSubmenus (widget, 1);
	    break;

	 case KEY_UP:
	    withinSubmenu (widget, -1);
	    break;

	 case KEY_DOWN:
	 case SPACE:
	    withinSubmenu (widget, 1);
	    break;

	 case KEY_ENTER:
	    cleanUpMenu (widget);
	    setExitType (widget, input);
	    widget->lastSelection = ((widget->currentTitle * 100) + widget->currentSubtitle);
	    ret = widget->lastSelection;
	    complete = TRUE;
	    break;

	 case KEY_ESC:
	    cleanUpMenu (widget);
	    setExitType (widget, input);
	    widget->lastSelection = -1;
	    ret = widget->lastSelection;
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
	 }
      }

      /* Should we call a post-process? */
      if (!complete && (PostProcessFuncOf (widget) != 0))
      {
	 PostProcessFuncOf (widget) (vMENU,
				     widget,
				     PostProcessDataOf (widget),
				     input);
      }
   }

   if (!complete)
   {
      setExitType (widget, 0);
   }

   ResultOf (widget).valueInt = ret;
   return (ret != unknownInt);
}

/*
 * Draw a menu item subwindow.
 */
void drawCDKMenuSubwin (CDKMENU *menu)
{
   int x;
   int high = getmaxy (menu->pullWin[menu->currentTitle]) - 2;
   int x0 = 0;
   int x1 = menu->subsize[menu->currentTitle];

   if (x1 > high)
      x1 = high;

   if (menu->currentSubtitle >= x1)
   {
      x0 = (menu->currentSubtitle - x1) + 1;
      x1 += x0;
   }

   /* Box the window. */
   werase (menu->pullWin[menu->currentTitle]);
   box (menu->pullWin[menu->currentTitle], ACS_VLINE, ACS_HLINE);

   if (menu->menuPos == BOTTOM)
   {
      (void)mvwaddch (menu->pullWin[menu->currentTitle],
		      menu->subsize[menu->currentTitle] + 1, 0, ACS_LTEE);
   }
   else
   {
      (void)mvwaddch (menu->pullWin[menu->currentTitle], 0, 0, ACS_LTEE);
   }

   /* Draw the items. */
   for (x = x0; x < x1; x++)
   {
      drawItem (menu, x, x0);
   }

   selectItem (menu, menu->currentSubtitle, x0);
   wrefresh (menu->pullWin[menu->currentTitle]);

   /* Highlight the title. */
   writeChtypeAttrib (menu->titleWin[menu->currentTitle],
		      0, 0, menu->title[menu->currentTitle],
		      menu->titleAttr, HORIZONTAL, 0,
		      menu->titleLen[menu->currentTitle]);
   wrefresh (menu->titleWin[menu->currentTitle]);
}

/*
 * Erase a menu item subwindow.
 */
void eraseCDKMenuSubwin (CDKMENU *menu)
{
   eraseCursesWindow (menu->pullWin[menu->currentTitle]);

   /* Redraw the sub-menu title. */
   drawTitle (menu, menu->currentTitle);
   wrefresh (menu->titleWin[menu->currentTitle]);
}

/*
 * Draw the menu.
 */
static void _drawCDKMenu (CDKOBJS *object, boolean Box GCC_UNUSED)
{
   CDKMENU *menu = (CDKMENU *)object;
   int x;

   /* Draw in the menu titles. */
   for (x = 0; x < menu->menuItems; x++)
   {
      drawTitle (menu, x);
      wrefresh (menu->titleWin[x]);
   }
}

/*
 * Move the menu to the given location.
 */
static void _moveCDKMenu (CDKOBJS *object,
			  int xplace,
			  int yplace,
			  boolean relative,
			  boolean refresh_flag)
{
   CDKMENU *menu = (CDKMENU *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx (WindowOf (menu));
   int currentY = getbegy (WindowOf (menu));
   int xpos     = xplace;
   int ypos     = yplace;
   int xdiff    = 0;
   int ydiff    = 0;
   int x;

   /*
    * If this is a relative move, then we will adjust where we want
    * to move to.
    */
   if (relative)
   {
      xpos = getbegx (WindowOf (menu)) + xplace;
      ypos = getbegy (WindowOf (menu)) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (menu),
	    &xpos,
	    &ypos,
	    getmaxx (WindowOf (menu)),
	    getmaxy (WindowOf (menu)));

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the windows to the new location. */
   moveCursesWindow (WindowOf (menu), -xdiff, -ydiff);
   for (x = 0; x < menu->menuItems; x++)
   {
      moveCursesWindow (menu->titleWin[x], -xdiff, -ydiff);
   }

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (menu));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKMenu (menu, ObjOf (menu)->box);
   }
}

/*
 * Set the background attribute of the widget.
 */
static void _setBKattrMenu (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKMENU *widget = (CDKMENU *)object;
      int x;

      for (x = 0; x < widget->menuItems; x++)
      {
	 wbkgd (widget->titleWin[x], attrib);
	 wbkgd (widget->pullWin[x], attrib);
      }
   }
}

/*
 * Destroy a menu widget.
 */
static void _destroyCDKMenu (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKMENU *menu = (CDKMENU *)object;
      int x, y;

      /* Clean up both the winodws and the char pointers. */
      for (x = 0; x < menu->menuItems; x++)
      {
	 /* Clean the windows. */
	 deleteCursesWindow (menu->titleWin[x]);
	 deleteCursesWindow (menu->pullWin[x]);

	 /* Delete the character pointers. */
	 freeChtype (menu->title[x]);
	 for (y = 0; y < menu->subsize[x]; y++)
	 {
	    freeChtype (menu->sublist[x][y]);
	 }
      }

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vMENU, menu);

      /* Unregister this object. */
      unregisterCDKObject (vMENU, menu);
   }
}

/*
 * Erase the menu widget from the screen.
 */
static void _eraseCDKMenu (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKMENU *menu = (CDKMENU *)object;
      int x = 0;

      for (x = 0; x < menu->menuItems; x++)
      {
	 werase (menu->titleWin[x]);
	 wrefresh (menu->titleWin[x]);
	 werase (menu->pullWin[x]);
	 wrefresh (menu->pullWin[x]);
      }
   }
}

/*
 * Set multiple features of the menu.
 */
void setCDKMenu (CDKMENU *menu,
		 int menuItem,
		 int subMenuItem,
		 chtype titleHighlight,
		 chtype subTitleHighlight)
{
   setCDKMenuCurrentItem (menu, menuItem, subMenuItem);
   setCDKMenuTitleHighlight (menu, titleHighlight);
   setCDKMenuSubTitleHighlight (menu, subTitleHighlight);
}

/*
 * Set the current menu item to highlight.
 */
void setCDKMenuCurrentItem (CDKMENU *menu, int menuitem, int submenuitem)
{
   /* *INDENT-EQLS* */
   menu->currentTitle    = wrapped (menuitem, menu->menuItems);
   menu->currentSubtitle = wrapped (submenuitem, menu->subsize[menu->currentTitle]);
}
void getCDKMenuCurrentItem (CDKMENU *menu, int *menuItem, int *subMenuItem)
{
   /* *INDENT-EQLS* */
   (*menuItem)          = menu->currentTitle;
   (*subMenuItem)       = menu->currentSubtitle;
}

/*
 * Set the attribute of the menu titles.
 */
void setCDKMenuTitleHighlight (CDKMENU *menu, chtype highlight)
{
   menu->titleAttr = highlight;
}
chtype getCDKMenuTitleHighlight (CDKMENU *menu)
{
   return menu->titleAttr;
}

/*
 * Set the attribute of the sub-title.
 */
void setCDKMenuSubTitleHighlight (CDKMENU *menu, chtype highlight)
{
   menu->subtitleAttr = highlight;
}
chtype getCDKMenuSubTitleHighlight (CDKMENU *menu)
{
   return menu->subtitleAttr;
}

/*
 * Exit the menu.
 */
static void cleanUpMenu (CDKMENU *menu)
{
   /* Erase the sub-menu. */
   eraseCDKMenuSubwin (menu);
   wrefresh (menu->pullWin[menu->currentTitle]);

   /* Refresh the screen. */
   refreshCDKScreen (ScreenOf (menu));
}

static void _focusCDKMenu (CDKOBJS *object)
{
   CDKMENU *menu = (CDKMENU *)object;

   drawCDKMenuSubwin (menu);
   ObjOf (menu)->inputWindow = menu->titleWin[menu->currentTitle];
}

dummyUnfocus (Menu)

dummyRefreshData (Menu)

dummySaveData (Menu)
