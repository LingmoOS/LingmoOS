#include <cdk_int.h>

#ifdef HAVE_SETLOCALE
#include <locale.h>
#endif

/*
 * $Author: tom $
 * $Date: 2016/12/04 15:41:50 $
 * $Revision: 1.91 $
 */

typedef struct _all_screens
{
   struct _all_screens *link;
   CDKSCREEN *screen;
}
ALL_SCREENS;

static ALL_SCREENS *all_screens;

typedef struct _all_objects
{
   struct _all_objects *link;
   CDKOBJS *object;
}
ALL_OBJECTS;

static ALL_OBJECTS *all_objects;

static boolean validObjType (CDKOBJS *obj, EObjectType type)
{
   bool valid = FALSE;

   if (obj != 0 && ObjTypeOf (obj) == type)
   {
      switch (type)
      {
      case vALPHALIST:
      case vBUTTON:
      case vBUTTONBOX:
      case vCALENDAR:
      case vDIALOG:
      case vDSCALE:
      case vENTRY:
      case vFSCALE:
      case vFSELECT:
      case vFSLIDER:
      case vGRAPH:
      case vHISTOGRAM:
      case vITEMLIST:
      case vLABEL:
      case vMARQUEE:
      case vMATRIX:
      case vMENTRY:
      case vMENU:
      case vRADIO:
      case vSCALE:
      case vSCROLL:
      case vSELECTION:
      case vSLIDER:
      case vSWINDOW:
      case vTEMPLATE:
      case vUSCALE:
      case vUSLIDER:
      case vVIEWER:
	 valid = TRUE;
	 break;
      case vTRAVERSE:		/* not really an object */
      case vNULL:
	 break;
      }
   }
   return valid;
}

/*
 * Set indices so the screen and object point to each other.
 */
static void setScreenIndex (CDKSCREEN *screen, int number, CDKOBJS *obj)
{
   (obj)->screenIndex = number;
   (obj)->screen = screen;
   screen->object[number] = obj;
}

/*
 * Returns true if we have done a "new" on this object but no "destroy"
 */
bool validCDKObject (CDKOBJS *obj)
{
   bool result = FALSE;
   if (obj != 0)
   {
      ALL_OBJECTS *ptr;

      for (ptr = all_objects; ptr != 0; ptr = ptr->link)
      {
	 if (ptr->object == obj)
	 {
	    result = validObjType (obj, ObjTypeOf (obj));
	    break;
	 }
      }
   }
   return result;
}

/*
 * Create a new object beginning with a CDKOBJS struct.  The whole object is
 * initialized to zeroes except for special cases which have known values.
 */
void *_newCDKObject (unsigned size, const CDKFUNCS * funcs)
{
   ALL_OBJECTS *item;
   CDKOBJS *result = 0;
   if ((item = typeCalloc (ALL_OBJECTS)) != 0)
   {
      if ((result = (CDKOBJS *)calloc (1, size)) != 0)
      {
	 result->fn = funcs;
	 result->hasFocus = TRUE;
	 result->isVisible = TRUE;

	 item->link = all_objects;
	 item->object = result;
	 all_objects = item;

	 /* set default line-drawing characters */
	 result->ULChar = ACS_ULCORNER;
	 result->URChar = ACS_URCORNER;
	 result->LLChar = ACS_LLCORNER;
	 result->LRChar = ACS_LRCORNER;
	 result->HZChar = ACS_HLINE;
	 result->VTChar = ACS_VLINE;
	 result->BXAttr = A_NORMAL;

	 /* set default exit-types */
	 result->exitType = vNEVER_ACTIVATED;
	 result->earlyExit = vNEVER_ACTIVATED;
      }
      else
      {
	 free (item);
      }
   }
   return (void *)result;
}

void _destroyCDKObject (CDKOBJS *obj)
{
   if (validCDKObject (obj))
   {
      ALL_OBJECTS *p, *q;

      for (p = all_objects, q = 0; p != 0; q = p, p = p->link)
      {
	 if (p->object == obj)
	 {
	    /* delink it first, to avoid problems with recursion */
	    if (q != 0)
	       q->link = p->link;
	    else
	       all_objects = p->link;

	    MethodPtr (obj, destroyObj) (obj);
	    free (obj);
	    free (p);
	    break;
	 }
      }
   }
}

/*
 * This creates a new CDK screen.
 */
CDKSCREEN *initCDKScreen (WINDOW *window)
{
   ALL_SCREENS *item;
   CDKSCREEN *screen = 0;

   /* initialization, for the first time */
   if (all_screens == 0 || stdscr == 0 || window == 0)
   {
      /* Set up basic curses settings. */
#ifdef HAVE_SETLOCALE
      setlocale (LC_ALL, "");
#endif
      /* Initialize curses after setting the locale, since curses depends
       * on having a correct locale to reflect the terminal's encoding.
       */
      if (stdscr == 0 || window == 0)
      {
	 window = initscr ();
      }
      noecho ();
      cbreak ();
   }

   if ((item = typeMalloc (ALL_SCREENS)) != 0)
   {
      if ((screen = typeCalloc (CDKSCREEN)) != 0)
      {
	 item->link = all_screens;
	 item->screen = screen;
	 all_screens = item;

	 /* Initialize the CDKSCREEN pointer. */
	 screen->objectCount = 0;
	 screen->objectLimit = 2;
	 screen->object = typeMallocN (CDKOBJS *, screen->objectLimit);
	 screen->window = window;

	 /* OK, we are done. */
      }
      else
      {
	 free (item);
      }
   }
   return (screen);
}

/*
 * This registers a CDK object with a screen.
 */
void registerCDKObject (CDKSCREEN *screen, EObjectType cdktype, void *object)
{
   CDKOBJS *obj = (CDKOBJS *)object;

   if (screen->objectCount + 1 >= screen->objectLimit)
   {
      screen->objectLimit += 2;
      screen->objectLimit *= 2;
      screen->object = typeReallocN (CDKOBJS *, screen->object, screen->objectLimit);
   }
   if (validObjType (obj, cdktype))
   {
      setScreenIndex (screen, screen->objectCount++, obj);
   }
}

/*
 * This registers a CDK object with a screen.
 */
void reRegisterCDKObject (EObjectType cdktype, void *object)
{
   CDKOBJS *obj = (CDKOBJS *)object;

   registerCDKObject (obj->screen, cdktype, object);
}

/*
 * This removes an object from the CDK screen.
 */
void unregisterCDKObject (EObjectType cdktype, void *object)
{
   CDKOBJS *obj = (CDKOBJS *)object;

   if (validObjType (obj, cdktype) && obj->screenIndex >= 0)
   {
      CDKSCREEN *screen = (obj)->screen;

      if (screen != 0)
      {
	 int Index = (obj)->screenIndex;
	 int x;

	 obj->screenIndex = -1;

	 /*
	  * Resequence the objects.
	  */
	 for (x = Index; x < screen->objectCount - 1; x++)
	 {
	    setScreenIndex (screen, x, screen->object[x + 1]);
	 }

	 if (screen->objectCount <= 1)
	 {
	    /* if no more objects, remove the array */
	    freeAndNull (screen->object);
	    screen->objectCount = 0;
	    screen->objectLimit = 0;
	 }
	 else
	 {
	    /* Reduce the list by one object. */
	    screen->object[screen->objectCount--] = 0;

	    /*
	     * Update the object-focus
	     */
	    if (screen->objectFocus == Index)
	    {
	       screen->objectFocus--;
	       (void)setCDKFocusNext (screen);
	    }
	    else if (screen->objectFocus > Index)
	    {
	       screen->objectFocus--;
	    }
	 }
      }
   }
}

#define validIndex(screen, n) ((n) >= 0 && (n) < (screen)->objectCount)

static void swapCDKIndices (CDKSCREEN *screen, int n1, int n2)
{
   if (n1 != n2 && validIndex (screen, n1) && validIndex (screen, n2))
   {
      CDKOBJS *o1 = screen->object[n1];
      CDKOBJS *o2 = screen->object[n2];
      setScreenIndex (screen, n1, o2);
      setScreenIndex (screen, n2, o1);

      if (screen->objectFocus == n1)
	 screen->objectFocus = n2;
      else if (screen->objectFocus == n2)
	 screen->objectFocus = n1;
   }
}

/*
 * This 'brings' a CDK object to the top of the stack.
 */
void raiseCDKObject (EObjectType cdktype, void *object)
{
   CDKOBJS *obj = (CDKOBJS *)object;

   if (validObjType (obj, cdktype))
   {
      CDKSCREEN *screen = obj->screen;
      swapCDKIndices (screen, obj->screenIndex, screen->objectCount - 1);
   }
}

/*
 * This 'lowers' an object.
 */
void lowerCDKObject (EObjectType cdktype, void *object)
{
   CDKOBJS *obj = (CDKOBJS *)object;

   if (validObjType (obj, cdktype))
   {
      CDKSCREEN *screen = obj->screen;
      swapCDKIndices (screen, obj->screenIndex, 0);
   }
}

/*
 * This calls refreshCDKScreen. (made consistent with widgets)
 */
void drawCDKScreen (CDKSCREEN *cdkscreen)
{
   refreshCDKScreen (cdkscreen);
}

/*
 * Refresh one CDK window.
 * FIXME: this should be rewritten to use the panel library, so it would not
 * be necessary to touch the window to ensure that it covers other windows.
 */
void refreshCDKWindow (WINDOW *win)
{
   touchwin (win);
   wrefresh (win);
}

/*
 * This refreshes all the objects in the screen.
 */
void refreshCDKScreen (CDKSCREEN *cdkscreen)
{
   int objectCount = cdkscreen->objectCount;
   int x;
   int focused = -1;
   int visible = -1;

   refreshCDKWindow (cdkscreen->window);

   /* We erase all the invisible objects, then only
    * draw it all back, so that the objects
    * can overlap, and the visible ones will always
    * be drawn after all the invisible ones are erased */
   for (x = 0; x < objectCount; x++)
   {
      CDKOBJS *obj = cdkscreen->object[x];

      if (validObjType (obj, ObjTypeOf (obj)))
      {
	 if (obj->isVisible)
	 {
	    if (visible < 0)
	       visible = x;
	    if (obj->hasFocus && focused < 0)
	       focused = x;
	 }
	 else
	 {
	    obj->fn->eraseObj (obj);
	 }
      }
   }

   for (x = 0; x < objectCount; x++)
   {
      CDKOBJS *obj = cdkscreen->object[x];

      if (validObjType (obj, ObjTypeOf (obj)))
      {
	 obj->hasFocus = (x == focused);

	 if (obj->isVisible)
	 {
	    obj->fn->drawObj (obj, obj->box);
	 }
      }
   }
}

/*
 * This clears all the objects in the screen.
 */
void eraseCDKScreen (CDKSCREEN *cdkscreen)
{
   int objectCount = cdkscreen->objectCount;
   int x;

   /* We just call the drawObject function. */
   for (x = 0; x < objectCount; x++)
   {
      CDKOBJS *obj = cdkscreen->object[x];

      if (validObjType (obj, ObjTypeOf (obj)))
      {
	 obj->fn->eraseObj (obj);
      }
   }

   /* Refresh the screen. */
   wrefresh (cdkscreen->window);
}

/*
 * Destroy all of the objects on a screen
 */
void destroyCDKScreenObjects (CDKSCREEN *cdkscreen)
{
   int x;

   for (x = 0; x < cdkscreen->objectCount; x++)
   {
      CDKOBJS *obj = cdkscreen->object[x];
      int before = cdkscreen->objectCount;

      if (validObjType (obj, ObjTypeOf (obj)))
      {
	 MethodPtr (obj, eraseObj) (obj);
	 _destroyCDKObject (obj);
	 x -= (cdkscreen->objectCount - before);
      }
   }
}

/*
 * This destroys a CDK screen.
 */
void destroyCDKScreen (CDKSCREEN *screen)
{
   ALL_SCREENS *p, *q;

   for (p = all_screens, q = 0; p != 0; q = p, p = p->link)
   {
      if (screen == p->screen)
      {
	 if (q != 0)
	    q->link = p->link;
	 else
	    all_screens = p->link;
	 free (p);
	 free (screen);
	 break;
      }
   }
}

/*
 * This is added to remain consistent.
 */
void endCDK (void)
{
   /* Turn echoing back on. */
   echo ();

   /* Turn off cbreak. */
   nocbreak ();

   /* End the curses windows. */
   endwin ();

#ifdef HAVE_XCURSES
   XCursesExit ();
#endif
}
