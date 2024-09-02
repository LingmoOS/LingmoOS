#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2016/11/20 18:28:30 $
 * $Revision: 1.24 $
 */

#define limitFocusIndex(screen, value) \
 	(((value) >= (screen)->objectCount || (value) < 0) \
	 ? 0 \
	 : (value))

static int getFocusIndex (CDKSCREEN *screen)
{
   int result = limitFocusIndex (screen, screen->objectFocus);

   return result;
}

static void setFocusIndex (CDKSCREEN *screen, int value)
{
   screen->objectFocus = limitFocusIndex (screen, value);
}

static void unsetFocus (CDKOBJS *obj)
{
   curs_set (0);
   if (obj != 0)
   {
      HasFocusObj (obj) = FALSE;
      UnfocusObj (obj);
   }
}

static void setFocus (CDKOBJS *obj)
{
   if (obj != 0)
   {
      HasFocusObj (obj) = TRUE;
      FocusObj (obj);
   }
   curs_set (1);
}

static CDKOBJS *switchFocus (CDKOBJS *newobj, CDKOBJS *oldobj)
{
   if (oldobj != newobj)
   {
      unsetFocus (oldobj);
      setFocus (newobj);
   }
   return newobj;
}

static boolean checkMenuKey (int keyCode, int functionKey)
{
   int result;

   result = (keyCode == KEY_ESC && !functionKey);

   return result;
}

static CDKOBJS *handleMenu (CDKSCREEN *screen, CDKOBJS *menu, CDKOBJS *oldobj)
{
   bool done = FALSE;
   CDKOBJS *newobj;

   switchFocus (menu, oldobj);
   while (!done)
   {
      boolean functionKey;
      int key = getchCDKObject (menu, &functionKey);

      switch (key)
      {
      case KEY_TAB:
	 done = TRUE;
	 break;

      case KEY_ESC:
	 /* cleanup the menu */
	 (void)injectCDKMenu ((CDKMENU *)menu, (chtype)key);
	 done = TRUE;
	 break;

      default:
	 done = (injectCDKMenu ((CDKMENU *)menu, (chtype)key) >= 0);
	 break;
      }
   }

   if ((newobj = getCDKFocusCurrent (screen)) == 0)
      newobj = setCDKFocusNext (screen);

   return switchFocus (newobj, menu);
}

/*
 * Save data in widgets on a screen
 */
static void saveDataCDKScreen (CDKSCREEN *screen)
{
   int i;

   for (i = 0; i < screen->objectCount; ++i)
      SaveDataObj (screen->object[i]);
}

/*
 * Refresh data in widgets on a screen
 */
static void refreshDataCDKScreen (CDKSCREEN *screen)
{
   int i;

   for (i = 0; i < screen->objectCount; ++i)
      RefreshDataObj (screen->object[i]);
}


/*
 * ======================================================================
 * Public Interface
 */

void resetCDKScreen (CDKSCREEN *screen)
{
   refreshDataCDKScreen (screen);
}

void exitOKCDKScreen (CDKSCREEN *screen)
{
   screen->exitStatus = CDKSCREEN_EXITOK;
}

void exitCancelCDKScreen (CDKSCREEN *screen)
{
   screen->exitStatus = CDKSCREEN_EXITCANCEL;
}

void exitOKCDKScreenOf (CDKOBJS *obj)
{
   exitOKCDKScreen (obj->screen);
}

void exitCancelCDKScreenOf (CDKOBJS *obj)
{
   exitCancelCDKScreen (obj->screen);
}

void resetCDKScreenOf (CDKOBJS *obj)
{
   resetCDKScreen (obj->screen);
}

/*
 * Returns the object on which the focus lies.
 */
CDKOBJS *getCDKFocusCurrent (CDKSCREEN *screen)
{
   CDKOBJS *result = 0;
   int n = screen->objectFocus;

   if (n >= 0 && n < screen->objectCount)
      result = screen->object[n];
   return result;
}

/*
 * Set focus to the next object, returning it.
 */
CDKOBJS *setCDKFocusNext (CDKSCREEN *screen)
{
   CDKOBJS *result = 0;
   CDKOBJS *curobj;
   int n = getFocusIndex (screen);
   int first = n;

   for (;;)
   {
      if (++n >= screen->objectCount)
	 n = 0;
      curobj = screen->object[n];
      if (curobj != 0 && AcceptsFocusObj (curobj))
      {
	 result = curobj;
	 break;
      }
      else
      {
	 if (n == first)
	 {
	    break;
	 }
      }
   }

   setFocusIndex (screen, (result != 0) ? n : -1);
   return result;
}

/*
 * Set focus to the previous object, returning it.
 */
CDKOBJS *setCDKFocusPrevious (CDKSCREEN *screen)
{
   CDKOBJS *result = 0;
   CDKOBJS *curobj;
   int n = getFocusIndex (screen);
   int first = n;

   for (;;)
   {
      if (--n < 0)
	 n = screen->objectCount - 1;
      curobj = screen->object[n];
      if (curobj != 0 && AcceptsFocusObj (curobj))
      {
	 result = curobj;
	 break;
      }
      else if (n == first)
      {
	 break;
      }
   }

   setFocusIndex (screen, (result != 0) ? n : -1);
   return result;
}

/*
 * Set focus to a specific object, returning it.
 * If the object cannot be found, return null.
 */
CDKOBJS *setCDKFocusCurrent (CDKSCREEN *screen, CDKOBJS *newobj)
{
   CDKOBJS *result = 0;
   CDKOBJS *curobj;
   int n = getFocusIndex (screen);
   int first = n;

   for (;;)
   {
      if (++n >= screen->objectCount)
	 n = 0;

      curobj = screen->object[n];
      if (curobj == newobj)
      {
	 result = curobj;
	 break;
      }
      else if (n == first)
      {
	 break;
      }
   }

   setFocusIndex (screen, (result != 0) ? n : -1);
   return result;
}

/*
 * Set focus to the first object in the screen.
 */
CDKOBJS *setCDKFocusFirst (CDKSCREEN *screen)
{
   setFocusIndex (screen, screen->objectCount - 1);
   return switchFocus (setCDKFocusNext (screen), 0);
}

/*
 * Set focus to the last object in the screen.
 */
CDKOBJS *setCDKFocusLast (CDKSCREEN *screen)
{
   setFocusIndex (screen, 0);
   return switchFocus (setCDKFocusPrevious (screen), 0);
}

void traverseCDKOnce (CDKSCREEN *screen,
		      CDKOBJS *curobj,
		      int keyCode,
		      boolean functionKey,
		      CHECK_KEYCODE funcMenuKey)
{
   switch (keyCode)
   {
   case KEY_BTAB:
      switchFocus (setCDKFocusPrevious (screen), curobj);
      break;

   case KEY_TAB:
      switchFocus (setCDKFocusNext (screen), curobj);
      break;

   case KEY_F (10):
      /* save data and exit */
      exitOKCDKScreen (screen);
      break;

   case CTRL ('X'):
      exitCancelCDKScreen (screen);
      break;

   case CTRL ('R'):
      /* reset data to defaults */
      resetCDKScreen (screen);
      setFocus (curobj);
      break;

   case CDK_REFRESH:
      /* redraw screen */
      refreshCDKScreen (screen);
      setFocus (curobj);
      break;

   default:
      /* not everyone wants menus, so we make them optional here */
      if (funcMenuKey != 0 && funcMenuKey (keyCode, functionKey))
      {
	 /* find and enable drop down menu */
	 int j;

	 for (j = 0; j < screen->objectCount; ++j)
	    if (ObjTypeOf (screen->object[j]) == vMENU)
	    {
	       handleMenu (screen, screen->object[j], curobj);
	       break;
	    }
      }
      else
      {
	 InjectObj (curobj, (chtype)keyCode);
      }
      break;
   }
}

/*
 * Traverse the widgets on a screen.
 */
int traverseCDKScreen (CDKSCREEN *screen)
{
   int result = 0;
   CDKOBJS *curobj = setCDKFocusFirst (screen);

   if (curobj != 0)
   {
      refreshDataCDKScreen (screen);

      screen->exitStatus = CDKSCREEN_NOEXIT;

      while (((curobj = getCDKFocusCurrent (screen)) != 0)
	     && (screen->exitStatus == CDKSCREEN_NOEXIT))
      {
	 int key;
	 boolean function;

	 key = getchCDKObject (curobj, &function);

	 traverseCDKOnce (screen, curobj, key, function, checkMenuKey);
      }

      if (screen->exitStatus == CDKSCREEN_EXITOK)
      {
	 saveDataCDKScreen (screen);
	 result = 1;
      }
   }
   return result;
}
