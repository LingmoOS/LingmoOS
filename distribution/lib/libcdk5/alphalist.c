#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2016/11/20 18:39:12 $
 * $Revision: 1.111 $
 */

/*
 * Declare file local prototypes.
 */
static BINDFN_PROTO (adjustAlphalistCB);
static BINDFN_PROTO (completeWordCB);
static int preProcessEntryField (EObjectType, void *, void *, chtype);
static int createList (CDKALPHALIST *alphalist, CDK_CSTRING *list, int listSize);

DeclareSetXXchar (static, _setMy);
DeclareCDKObjects (ALPHALIST, Alphalist, _setMy, String);

/*
 * This creates the alphalist widget.
 */
CDKALPHALIST *newCDKAlphalist (CDKSCREEN *cdkscreen,
			       int xplace,
			       int yplace,
			       int height,
			       int width,
			       const char *title,
			       const char *label,
			       CDK_CSTRING *list,
			       int listSize,
			       chtype fillerChar,
			       chtype highlight,
			       boolean Box,
			       boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKALPHALIST *alphalist      = 0;
   int parentWidth              = getmaxx (cdkscreen->window);
   int parentHeight             = getmaxy (cdkscreen->window);
   int boxWidth;
   int boxHeight;
   int xpos                     = xplace;
   int ypos                     = yplace;
   int tempWidth                = 0;
   int tempHeight               = 0;
   int labelLen                 = 0;
   int x, junk2;
   /* *INDENT-OFF* */
   static const struct { int from; int to; } bindings[] = {
      { CDK_BACKCHAR,	KEY_PPAGE },
      { CDK_FORCHAR,	KEY_NPAGE },
   };
   /* *INDENT-ON* */

   if ((alphalist = newCDKObject (CDKALPHALIST, &my_funcs)) == 0
       || !createList (alphalist, list, listSize))
   {
      destroyCDKObject (alphalist);
      return (0);
   }

   setCDKAlphalistBox (alphalist, Box);

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

   /* Translate the label char *pointer to a chtype pointer. */
   if (label != 0)
   {
      chtype *chtypeLabel = char2Chtype (label, &labelLen, &junk2);
      freeChtype (chtypeLabel);
   }

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* Make the file selector window. */
   alphalist->win = newwin (boxHeight, boxWidth, ypos, xpos);

   if (alphalist->win == 0)
   {
      destroyCDKObject (alphalist);
      return (0);
   }
   keypad (alphalist->win, TRUE);

   /* *INDENT-EQLS* Set some variables. */
   ScreenOf (alphalist)         = cdkscreen;
   alphalist->parent            = cdkscreen->window;
   alphalist->highlight         = highlight;
   alphalist->fillerChar        = fillerChar;
   alphalist->boxHeight         = boxHeight;
   alphalist->boxWidth          = boxWidth;
   initExitType (alphalist);
   alphalist->shadow            = shadow;
   alphalist->shadowWin         = 0;

   /* Do we want a shadow? */
   if (shadow)
   {
      alphalist->shadowWin = newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   /* Create the entry field. */
   tempWidth = (isFullWidth (width)
		? FULL
		: boxWidth - 2 - labelLen);
   alphalist->entryField = newCDKEntry (cdkscreen,
					getbegx (alphalist->win),
					getbegy (alphalist->win),
					title, label,
					A_NORMAL, fillerChar,
					vMIXED, tempWidth, 0, 512,
					Box, FALSE);
   if (alphalist->entryField == 0)
   {
      destroyCDKObject (alphalist);
      return (0);
   }
   setCDKEntryLLChar (alphalist->entryField, ACS_LTEE);
   setCDKEntryLRChar (alphalist->entryField, ACS_RTEE);

   /* Set the key bindings for the entry field. */
   bindCDKObject (vENTRY,
		  alphalist->entryField,
		  KEY_UP,
		  adjustAlphalistCB,
		  alphalist);
   bindCDKObject (vENTRY,
		  alphalist->entryField,
		  KEY_DOWN,
		  adjustAlphalistCB,
		  alphalist);
   bindCDKObject (vENTRY,
		  alphalist->entryField,
		  KEY_NPAGE,
		  adjustAlphalistCB,
		  alphalist);
   bindCDKObject (vENTRY,
		  alphalist->entryField,
		  KEY_PPAGE,
		  adjustAlphalistCB,
		  alphalist);
   bindCDKObject (vENTRY,
		  alphalist->entryField,
		  KEY_TAB,
		  completeWordCB,
		  alphalist);

   /* Set up the post-process function for the entry field. */
   setCDKEntryPreProcess (alphalist->entryField, preProcessEntryField, alphalist);

   /*
    * Create the scrolling list.  It overlaps the entry field by one line if
    * we are using box-borders.
    */
   tempHeight = getmaxy (alphalist->entryField->win) - BorderOf (alphalist);
   tempWidth = (isFullWidth (width)
		? FULL
		: boxWidth - 1);
   alphalist->scrollField = newCDKScroll (cdkscreen,
					  getbegx (alphalist->win),
					  getbegy (alphalist->entryField->win)
					  + tempHeight,
					  RIGHT,
					  boxHeight - tempHeight,
					  tempWidth,
					  0, (CDK_CSTRING2)list, listSize,
					  NONUMBERS, A_REVERSE,
					  Box, FALSE);
   setCDKScrollULChar (alphalist->scrollField, ACS_LTEE);
   setCDKScrollURChar (alphalist->scrollField, ACS_RTEE);

   /* Setup the key bindings. */
   for (x = 0; x < (int)SIZEOF (bindings); ++x)
      bindCDKObject (vALPHALIST,
		     alphalist,
		     (chtype)bindings[x].from,
		     getcCDKBind,
		     (void *)(long)bindings[x].to);

   registerCDKObject (cdkscreen, vALPHALIST, alphalist);

   return (alphalist);
}

/*
 * This erases the file selector from the screen.
 */
static void _eraseCDKAlphalist (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKALPHALIST *alphalist = (CDKALPHALIST *)object;

      eraseCDKScroll (alphalist->scrollField);
      eraseCDKEntry (alphalist->entryField);

      eraseCursesWindow (alphalist->shadowWin);
      eraseCursesWindow (alphalist->win);
   }
}

/*
 * This moves the alphalist field to the given location.
 */
static void _moveCDKAlphalist (CDKOBJS *object,
			       int xplace,
			       int yplace,
			       boolean relative,
			       boolean refresh_flag)
{
   CDKALPHALIST *alphalist = (CDKALPHALIST *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx (alphalist->win);
   int currentY = getbegy (alphalist->win);
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
      xpos = getbegx (alphalist->win) + xplace;
      ypos = getbegy (alphalist->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (alphalist), &xpos, &ypos, alphalist->boxWidth, alphalist->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (alphalist->win, -xdiff, -ydiff);
   moveCursesWindow (alphalist->shadowWin, -xdiff, -ydiff);

   /* Move the sub-widgets. */
   moveCDKEntry (alphalist->entryField, xplace, yplace, relative, FALSE);
   moveCDKScroll (alphalist->scrollField, xplace, yplace, relative, FALSE);

   /* Touch the windows so they 'move'. */
   refreshCDKWindow (WindowOf (alphalist));

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKAlphalist (alphalist, ObjOf (alphalist)->box);
   }
}

/*
 * The alphalist's focus resides in the entry widget.  But the scroll widget
 * will not draw items highlighted unless it has focus.  Temporarily adjust the
 * focus of the scroll widget when drawing on it to get the right highlighting.
 */
#define SaveFocus(widget) \
   boolean save = HasFocusObj (ObjOf (widget->scrollField)); \
   HasFocusObj (ObjOf (widget->scrollField)) = \
   HasFocusObj (ObjOf (widget->entryField))

#define RestoreFocus(widget) \
   HasFocusObj (ObjOf (widget->scrollField)) = save

static void drawMyScroller (CDKALPHALIST *widget)
{
   SaveFocus (widget);
   drawCDKScroll (widget->scrollField, ObjOf (widget->scrollField)->box);
   RestoreFocus (widget);
}

static void injectMyScroller (CDKALPHALIST *widget, chtype key)
{
   SaveFocus (widget);
   (void)injectCDKScroll (widget->scrollField, key);
   RestoreFocus (widget);
}

/*
 * This draws the file selector widget.
 */
static void _drawCDKAlphalist (CDKOBJS *obj, boolean Box GCC_UNUSED)
{
   CDKALPHALIST *alphalist = (CDKALPHALIST *)obj;

   /* Does this widget have a shadow? */
   if (alphalist->shadowWin != 0)
   {
      drawShadow (alphalist->shadowWin);
   }

   /* Draw in the entry field. */
   drawCDKEntry (alphalist->entryField, ObjOf (alphalist->entryField)->box);

   /* Draw in the scroll field. */
   drawMyScroller (alphalist);
}

/*
 * This activates the file selector.
 */
char *activateCDKAlphalist (CDKALPHALIST *alphalist, chtype *actions)
{
   char *ret = 0;

   /* Draw the widget. */
   drawCDKAlphalist (alphalist, ObjOf (alphalist)->box);

   /* Activate the widget. */
   ret = activateCDKEntry (alphalist->entryField, actions);

   /* Copy the exit type from the entry field. */
   copyExitType (alphalist, alphalist->entryField);

   /* Determine the exit status. */
   if (alphalist->exitType != vEARLY_EXIT)
   {
      return ret;
   }
   return 0;
}

/*
 * This injects a single character into the alphalist.
 */
static int _injectCDKAlphalist (CDKOBJS *object, chtype input)
{
   CDKALPHALIST *alphalist = (CDKALPHALIST *)object;
   char *ret;

   /* Draw the widget. */
   drawCDKAlphalist (alphalist, ObjOf (alphalist)->box);

   /* Inject a character into the widget. */
   ret = injectCDKEntry (alphalist->entryField, input);

   /* Copy the exit type from the entry field. */
   copyExitType (alphalist, alphalist->entryField);

   /* Determine the exit status. */
   if (alphalist->exitType == vEARLY_EXIT)
      ret = unknownString;

   ResultOf (alphalist).valueString = ret;
   return (ret != unknownString);
}

/*
 * This sets multiple attributes of the widget.
 */
void setCDKAlphalist (CDKALPHALIST *alphalist,
		      CDK_CSTRING *list,
		      int listSize,
		      chtype fillerChar,
		      chtype highlight,
		      boolean Box)
{
   setCDKAlphalistContents (alphalist, list, listSize);
   setCDKAlphalistFillerChar (alphalist, fillerChar);
   setCDKAlphalistHighlight (alphalist, highlight);
   setCDKAlphalistBox (alphalist, Box);
}

/*
 * This function sets the information inside the file selector.
 */
void setCDKAlphalistContents (CDKALPHALIST *widget, CDK_CSTRING *list, int listSize)
{
   CDKSCROLL *scrollp = widget->scrollField;
   CDKENTRY *entry = widget->entryField;

   if (!createList (widget, list, listSize))
      return;

   /* Set the information in the scrolling list. */
   setCDKScroll (scrollp,
		 (CDK_CSTRING2)widget->list,
		 widget->listSize,
		 NONUMBERS,
		 scrollp->highlight,
		 ObjOf (scrollp)->box);

   /* Clean out the entry field. */
   setCDKAlphalistCurrentItem (widget, 0);
   cleanCDKEntry (entry);

   /* Redraw the widget. */
   eraseCDKAlphalist (widget);
   drawCDKAlphalist (widget, ObjOf (widget)->box);
}

/*
 * This returns the contents of the widget.
 */
char **getCDKAlphalistContents (CDKALPHALIST *widget, int *size)
{
   (*size) = widget->listSize;
   return widget->list;
}

/*
 * Get/set the current position in the scroll-widget.
 */
int getCDKAlphalistCurrentItem (CDKALPHALIST *widget)
{
   return getCDKScrollCurrent (widget->scrollField);
}

void setCDKAlphalistCurrentItem (CDKALPHALIST *widget, int item)
{
   if (widget->listSize != 0)
   {
      setCDKScrollCurrent (widget->scrollField, item);
      setCDKEntryValue (widget->entryField,
			widget->list[getCDKScrollCurrentItem (widget->scrollField)]);
   }
}

/*
 * This sets the filler character of the entry field of the alphalist.
 */
void setCDKAlphalistFillerChar (CDKALPHALIST *alphalist, chtype fillerCharacter)
{
   CDKENTRY *entry = (CDKENTRY *)alphalist->entryField;

   alphalist->fillerChar = fillerCharacter;

   setCDKEntryFillerChar (entry, fillerCharacter);
}
chtype getCDKAlphalistFillerChar (CDKALPHALIST *alphalist)
{
   return alphalist->fillerChar;
}

/*
 * This sets the highlight bar attributes.
 */
void setCDKAlphalistHighlight (CDKALPHALIST *alphalist, chtype highlight)
{
   alphalist->highlight = highlight;
}
chtype getCDKAlphalistHighlight (CDKALPHALIST *alphalist)
{
   return alphalist->highlight;
}

/*
 * This sets whether or not the widget will be drawn with a box.
 */
void setCDKAlphalistBox (CDKALPHALIST *alphalist, boolean Box)
{
   ObjOf (alphalist)->box = Box;
   ObjOf (alphalist)->borderSize = Box ? 1 : 0;
}

boolean getCDKAlphalistBox (CDKALPHALIST *alphalist)
{
   return ObjOf (alphalist)->box;
}

/*
 * These functions set the drawing characters of the widget.
 */
static void _setMyULchar (CDKOBJS *object, chtype character)
{
   CDKALPHALIST *alphalist = (CDKALPHALIST *)object;

   setCDKEntryULChar (alphalist->entryField, character);
}
static void _setMyURchar (CDKOBJS *object, chtype character)
{
   CDKALPHALIST *alphalist = (CDKALPHALIST *)object;

   setCDKEntryURChar (alphalist->entryField, character);
}
static void _setMyLLchar (CDKOBJS *object, chtype character)
{
   CDKALPHALIST *alphalist = (CDKALPHALIST *)object;

   setCDKScrollLLChar (alphalist->scrollField, character);
}
static void _setMyLRchar (CDKOBJS *object, chtype character)
{
   CDKALPHALIST *alphalist = (CDKALPHALIST *)object;

   setCDKScrollLRChar (alphalist->scrollField, character);
}
static void _setMyVTchar (CDKOBJS *object, chtype character)
{
   CDKALPHALIST *alphalist = (CDKALPHALIST *)object;

   setCDKEntryVerticalChar (alphalist->entryField, character);
   setCDKScrollVerticalChar (alphalist->scrollField, character);
}
static void _setMyHZchar (CDKOBJS *object, chtype character)
{
   CDKALPHALIST *alphalist = (CDKALPHALIST *)object;

   setCDKEntryHorizontalChar (alphalist->entryField, character);
   setCDKScrollHorizontalChar (alphalist->scrollField, character);
}
static void _setMyBXattr (CDKOBJS *object, chtype character)
{
   CDKALPHALIST *alphalist = (CDKALPHALIST *)object;

   setCDKEntryBoxAttribute (alphalist->entryField, character);
   setCDKScrollBoxAttribute (alphalist->scrollField, character);
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattrAlphalist (CDKOBJS *obj, chtype attrib)
{
   CDKALPHALIST *alphalist = (CDKALPHALIST *)obj;

   setCDKEntryBackgroundAttrib (alphalist->entryField, attrib);
   setCDKScrollBackgroundAttrib (alphalist->scrollField, attrib);
}

static void destroyInfo (CDKALPHALIST *widget)
{
   CDKfreeStrings (widget->list);
   widget->list = 0;

   widget->listSize = 0;
}

/*
 * This destroys the file selector.
 */
static void _destroyCDKAlphalist (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKALPHALIST *alphalist = (CDKALPHALIST *)object;

      destroyInfo (alphalist);

      /* Clean the key bindings. */
      cleanCDKObjectBindings (vALPHALIST, alphalist);

      destroyCDKEntry (alphalist->entryField);
      destroyCDKScroll (alphalist->scrollField);

      /* Free up the window pointers. */
      deleteCursesWindow (alphalist->shadowWin);
      deleteCursesWindow (alphalist->win);

      /* Unregister the object. */
      unregisterCDKObject (vALPHALIST, alphalist);
   }
}

/*
 * This function sets the pre-process function.
 */
void setCDKAlphalistPreProcess (CDKALPHALIST *alphalist,
				PROCESSFN callback,
				void *data)
{
   setCDKEntryPreProcess (alphalist->entryField, callback, data);
}

/*
 * This function sets the post-process function.
 */
void setCDKAlphalistPostProcess (CDKALPHALIST *alphalist,
				 PROCESSFN callback,
				 void *data)
{
   setCDKEntryPostProcess (alphalist->entryField, callback, data);
}

/*
 * Start of callback functions.
 */
static int adjustAlphalistCB (EObjectType objectType GCC_UNUSED, void
			      *object GCC_UNUSED,
			      void *clientData,
			      chtype key)
{
   /* *INDENT-EQLS* */
   CDKALPHALIST *alphalist = (CDKALPHALIST *)clientData;
   CDKSCROLL *scrollp      = alphalist->scrollField;
   CDKENTRY *entry         = alphalist->entryField;

   if (scrollp->listSize > 0)
   {
      char *current;

      /* Adjust the scrolling list. */
      injectMyScroller (alphalist, key);

      /* Set the value in the entry field. */
      current = chtype2Char (scrollp->item[scrollp->currentItem]);
      setCDKEntryValue (entry, current);
      drawCDKEntry (entry, ObjOf (entry)->box);
      freeChar (current);
      return (TRUE);
   }
   Beep ();
   return (FALSE);
}

/*
 * This is the heart-beat of the widget.
 */
static int preProcessEntryField (EObjectType cdktype GCC_UNUSED, void
				 *object GCC_UNUSED,
				 void *clientData,
				 chtype input)
{
   /* *INDENT-EQLS* */
   CDKALPHALIST *alphalist = (CDKALPHALIST *)clientData;
   CDKSCROLL *scrollp      = alphalist->scrollField;
   CDKENTRY *entry         = alphalist->entryField;
   int infoLen             = ((entry->info != 0)
			      ? (int)strlen (entry->info)
			      : 0);
   int result              = 1;
   bool empty              = FALSE;

   /* Make sure the entry field isn't empty. */
   if (entry->info == 0)
   {
      empty = TRUE;
   }
   else if (isCDKObjectBind (ObjTypeOf (alphalist), ObjOf (alphalist), input))
   {
      result = 1;		/* don't try to use this key in editing */
   }
   else if ((isChar (input) &&
	     (isalnum (CharOf (input)) ||
	      ispunct (input))) ||
	    input == KEY_BACKSPACE ||
	    input == KEY_DC)
   {
      int Index, difference, absoluteDifference, x;
      int currPos = (entry->screenCol + entry->leftChar);
      char *pattern = (char *)malloc ((size_t) infoLen + 2);

      if (pattern != 0)
      {
	 strcpy (pattern, entry->info);

	 if (input == KEY_BACKSPACE || input == KEY_DC)
	 {
	    if (input == KEY_BACKSPACE)
	       --currPos;
	    if (currPos >= 0)
	       strcpy (pattern + currPos, entry->info + currPos + 1);
	 }
	 else
	 {
	    pattern[currPos] = (char)input;
	    strcpy (pattern + currPos + 1, entry->info + currPos);
	 }
      }

      if (pattern == 0)
      {
	 Beep ();
      }
      else if (strlen (pattern) == 0)
      {
	 empty = TRUE;
      }
      else if ((Index = searchList ((CDK_CSTRING2)alphalist->list,
				    alphalist->listSize,
				    pattern)) >= 0)
      {
	 /* *INDENT-EQLS* */
	 difference           = Index - scrollp->currentItem;
	 absoluteDifference   = abs (difference);

	 /*
	  * If the difference is less than zero, then move up.
	  * Otherwise move down.
	  *
	  * If the difference is greater than 10 jump to the new
	  * index position.  Otherwise provide the nice scroll.
	  */
	 if (absoluteDifference <= 10)
	 {
	    for (x = 0; x < absoluteDifference; x++)
	    {
	       injectMyScroller (alphalist,
				 (chtype)((difference <= 0)
					  ? KEY_UP
					  : KEY_DOWN));
	    }
	 }
	 else
	 {
	    setCDKScrollPosition (scrollp, Index);
	 }
	 drawMyScroller (alphalist);
      }
      else
      {
	 Beep ();
	 result = 0;
      }

      if (pattern != 0)
	 free (pattern);
   }

   if (empty)
   {
      setCDKScrollPosition (scrollp, 0);
      drawMyScroller (alphalist);
   }
   return result;
}

/*
 * This tries to complete the word in the entry field.
 */
static int completeWordCB (EObjectType objectType GCC_UNUSED, void *object GCC_UNUSED,
			   void *clientData,
			   chtype key GCC_UNUSED)
{
   /* *INDENT-EQLS* */
   CDKALPHALIST *alphalist = (CDKALPHALIST *)clientData;
   CDKENTRY *entry         = (CDKENTRY *)alphalist->entryField;
   CDKSCROLL *scrollp      = 0;
   int wordLength          = 0;
   int Index               = 0;
   int ret                 = 0;
   char **altWords         = 0;

   if (entry->info == 0)
   {
      Beep ();
      return (TRUE);
   }
   wordLength = (int)strlen (entry->info);

   /* If the word length is equal to zero, just leave. */
   if (wordLength == 0)
   {
      Beep ();
      return (TRUE);
   }

   /* Look for a unique word match. */
   Index = searchList ((CDK_CSTRING2)alphalist->list, alphalist->listSize, entry->info);

   /* If the index is less than zero, return we didn't find a match. */
   if (Index < 0)
   {
      Beep ();
      return (TRUE);
   }

   /* Did we find the last word in the list? */
   if (Index == alphalist->listSize - 1)
   {
      setCDKEntryValue (entry, alphalist->list[Index]);
      drawCDKEntry (entry, ObjOf (entry)->box);
      return (TRUE);
   }

   /* Ok, we found a match, is the next item similar? */
   ret = strncmp (alphalist->list[Index + 1], entry->info, (size_t) wordLength);
   if (ret == 0)
   {
      int currentIndex = Index;
      int altCount = 0;
      unsigned used = 0;
      int selected;
      int height;
      int match;
      int x;

      /* Start looking for alternate words. */
      /* FIXME: bsearch would be more suitable */
      while ((currentIndex < alphalist->listSize)
	     && (strncmp (alphalist->list[currentIndex],
			  entry->info,
			  (size_t) wordLength) == 0))
      {
	 used = CDKallocStrings (&altWords,
				 alphalist->list[currentIndex++],
				 (unsigned)altCount++,
				 used);
      }

      /* Determine the height of the scrolling list. */
      height = (altCount < 8 ? altCount + 3 : 11);

      /* Create a scrolling list of close matches. */
      scrollp = newCDKScroll (entry->obj.screen,
			      CENTER, CENTER, RIGHT, height, -30,
			      "<C></B/5>Possible Matches.",
			      (CDK_CSTRING2)altWords, altCount,
			      NUMBERS, A_REVERSE, TRUE, FALSE);

      /* Allow them to select a close match. */
      match = activateCDKScroll (scrollp, 0);
      selected = scrollp->currentItem;

      /* Check how they exited the list. */
      if (scrollp->exitType == vESCAPE_HIT)
      {
	 /* Destroy the scrolling list. */
	 destroyCDKScroll (scrollp);

	 /* Clean up. */
	 CDKfreeStrings (altWords);

	 /* Beep at the user. */
	 Beep ();

	 /* Redraw the alphalist and return. */
	 drawCDKAlphalist (alphalist, ObjOf (alphalist)->box);
	 return (TRUE);
      }

      /* Destroy the scrolling list. */
      destroyCDKScroll (scrollp);

      /* Set the entry field to the selected value. */
      setCDKEntry (entry,
		   altWords[match],
		   entry->min,
		   entry->max,
		   ObjOf (entry)->box);

      /* Move the highlight bar down to the selected value. */
      for (x = 0; x < selected; x++)
      {
	 injectMyScroller (alphalist, KEY_DOWN);
      }

      /* Clean up. */
      CDKfreeStrings (altWords);

      /* Redraw the alphalist. */
      drawCDKAlphalist (alphalist, ObjOf (alphalist)->box);
   }
   else
   {
      /* Set the entry field with the found item. */
      setCDKEntry (entry,
		   alphalist->list[Index],
		   entry->min,
		   entry->max,
		   ObjOf (entry)->box);
      drawCDKEntry (entry, ObjOf (entry)->box);
   }
   return (TRUE);
}

static int createList (CDKALPHALIST *alphalist, CDK_CSTRING *list, int listSize)
{
   int status = 0;

   if (listSize >= 0)
   {
      char **newlist = typeCallocN (char *, listSize + 1);

      if (newlist != 0)
      {
	 int x;

	 /*
	  * We'll sort the list before we use it.  It would have been better to
	  * declare list[] const and only modify the copy, but there may be
	  * clients that rely on the old behavior.
	  */
	 sortList (list, listSize);

	 /* Copy in the new information. */
	 status = 1;
	 for (x = 0; x < listSize; x++)
	 {
	    if ((newlist[x] = copyChar (list[x])) == 0)
	    {
	       status = 0;
	       break;
	    }
	 }
	 if (status)
	 {
	    destroyInfo (alphalist);
	    alphalist->listSize = listSize;
	    alphalist->list = newlist;
	 }
	 else
	 {
	    CDKfreeStrings (newlist);
	 }
      }
   }
   else
   {
      destroyInfo (alphalist);
      status = TRUE;
   }
   return status;
}

static void _focusCDKAlphalist (CDKOBJS *object)
{
   CDKALPHALIST *widget = (CDKALPHALIST *)object;

   FocusObj (ObjOf (widget->entryField));
}

static void _unfocusCDKAlphalist (CDKOBJS *object)
{
   CDKALPHALIST *widget = (CDKALPHALIST *)object;

   UnfocusObj (ObjOf (widget->entryField));
}

dummyRefreshData (Alphalist)

dummySaveData (Alphalist)
