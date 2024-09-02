#define TRACE
#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 00:41:07 $
 * $Revision: 1.86 $
 */

/*
 * Declare file local prototypes.
 */
/* *INDENT-OFF* */
static BINDFN_PROTO (completeFilenameCB);
static BINDFN_PROTO (displayFileInfoCB);
static BINDFN_PROTO (fselectAdjustScrollCB);
static char *contentToPath (CDKFSELECT *fselect, char *content);
static char *errorMessage (const char *format);
static char *expandTilde (const char *filename);
static char *format1Date (const char *format, time_t value);
static char *format1Number (const char *format, long value);
static char *format1String (const char *format, const char *string);
static char *format3String (const char *format, const char *s1, const char *s2, const char *s3);
static char *format1StrVal (const char *format, const char *string, int value);
static char *trim1Char (char *source);
static int createList (CDKFSELECT *widget, CDK_CSTRING2 list, int listSize);
static void setPWD (CDKFSELECT *fselect);
/* *INDENT-ON* */

DeclareSetXXchar (static, _setMy);
DeclareCDKObjects (FSELECT, Fselect, _setMy, String);

/*
 * This creates a file selection widget.
 */
CDKFSELECT *newCDKFselect (CDKSCREEN *cdkscreen,
			   int xplace,
			   int yplace,
			   int height,
			   int width,
			   const char *title,
			   const char *label,
			   chtype fieldAttribute,
			   chtype fillerChar,
			   chtype highlight,
			   const char *dAttribute,
			   const char *fAttribute,
			   const char *lAttribute,
			   const char *sAttribute,
			   boolean Box,
			   boolean shadow)
{
   /* *INDENT-EQLS* */
   CDKFSELECT *fselect  = 0;
   int parentWidth      = getmaxx (cdkscreen->window);
   int parentHeight     = getmaxy (cdkscreen->window);
   int boxWidth;
   int boxHeight;
   int xpos             = xplace;
   int ypos             = yplace;
   int tempWidth        = 0;
   int tempHeight       = 0;
   int labelLen, junk;
   chtype *chtypeString;
   int x;
   /* *INDENT-OFF* */
   static const struct
   {
      int from;
      int to;
   } bindings[] =
   {
      { CDK_BACKCHAR,	KEY_PPAGE },
      { CDK_FORCHAR,	KEY_NPAGE },
   };
   /* *INDENT-ON* */

   if ((fselect = newCDKObject (CDKFSELECT, &my_funcs)) == 0)
        return (0);

   setCDKFselectBox (fselect, Box);

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

   /* Rejustify the x and y positions if we need to. */
   alignxy (cdkscreen->window, &xpos, &ypos, boxWidth, boxHeight);

   /* Make sure the box isn't too small. */
   boxWidth = (boxWidth < 15 ? 15 : boxWidth);
   boxHeight = (boxHeight < 6 ? 6 : boxHeight);

   /* Make the file selector window. */
   fselect->win = newwin (boxHeight, boxWidth, ypos, xpos);

   /* Is the window null? */
   if (fselect->win == 0)
   {
      destroyCDKObject (fselect);
      return (0);
   }
   keypad (fselect->win, TRUE);

   /* *INDENT-EQLS* Set some variables. */
   ScreenOf (fselect)           = cdkscreen;
   fselect->parent              = cdkscreen->window;
   fselect->dirAttribute        = copyChar (dAttribute);
   fselect->fileAttribute       = copyChar (fAttribute);
   fselect->linkAttribute       = copyChar (lAttribute);
   fselect->sockAttribute       = copyChar (sAttribute);
   fselect->highlight           = highlight;
   fselect->fillerCharacter     = fillerChar;
   fselect->fieldAttribute      = fieldAttribute;
   fselect->boxHeight           = boxHeight;
   fselect->boxWidth            = boxWidth;
   fselect->fileCounter         = 0;
   fselect->pwd                 = 0;
   initExitType (fselect);
   ObjOf (fselect)->inputWindow = fselect->win;
   fselect->shadow              = shadow;
   fselect->shadowWin           = 0;

   /* Get the present working directory. */
   setPWD (fselect);

   /* Get the contents of the current directory. */
   setCDKFselectDirContents (fselect);

   /* Create the entry field in the selector. */
   chtypeString = char2Chtype (label, &labelLen, &junk);
   freeChtype (chtypeString);
   tempWidth = (isFullWidth (width)
		? FULL
		: boxWidth - 2 - labelLen);
   fselect->entryField = newCDKEntry (cdkscreen,
				      getbegx (fselect->win),
				      getbegy (fselect->win),
				      title, label,
				      fieldAttribute, fillerChar,
				      vMIXED, tempWidth, 0, 512,
				      Box, FALSE);

   /* Make sure the widget was created. */
   if (fselect->entryField == 0)
   {
      destroyCDKObject (fselect);
      return (0);
   }

   /* Set the lower left/right characters of the entry field. */
   setCDKEntryLLChar (fselect->entryField, ACS_LTEE);
   setCDKEntryLRChar (fselect->entryField, ACS_RTEE);

   /* Define the callbacks for the entry field. */
   bindCDKObject (vENTRY,
		  fselect->entryField,
		  KEY_UP,
		  fselectAdjustScrollCB,
		  fselect);
   bindCDKObject (vENTRY,
		  fselect->entryField,
		  KEY_PPAGE,
		  fselectAdjustScrollCB,
		  fselect);
   bindCDKObject (vENTRY,
		  fselect->entryField,
		  KEY_DOWN,
		  fselectAdjustScrollCB,
		  fselect);
   bindCDKObject (vENTRY,
		  fselect->entryField,
		  KEY_NPAGE,
		  fselectAdjustScrollCB,
		  fselect);
   bindCDKObject (vENTRY,
		  fselect->entryField,
		  KEY_TAB,
		  completeFilenameCB,
		  fselect);
   bindCDKObject (vENTRY,
		  fselect->entryField,
		  CTRL ('^'),
		  displayFileInfoCB,
		  fselect);

   /* Put the current working directory in the entry field. */
   setCDKEntryValue (fselect->entryField, fselect->pwd);

   /* Create the scrolling list in the selector. */
   tempHeight = getmaxy (fselect->entryField->win) - BorderOf (fselect);
   tempWidth = (isFullWidth (width)
		? FULL
		: boxWidth - 1);
   fselect->scrollField = newCDKScroll (cdkscreen,
					getbegx (fselect->win),
					getbegy (fselect->win) + tempHeight,
					RIGHT,
					boxHeight - tempHeight,
					tempWidth,
					0,
					(CDK_CSTRING2)fselect->dirContents,
					fselect->fileCounter,
					NONUMBERS, fselect->highlight,
					Box, FALSE);

   /* Set the lower left/right characters of the entry field. */
   setCDKScrollULChar (fselect->scrollField, ACS_LTEE);
   setCDKScrollURChar (fselect->scrollField, ACS_RTEE);

   /* Do we want a shadow? */
   if (shadow)
   {
      fselect->shadowWin = newwin (boxHeight, boxWidth, ypos + 1, xpos + 1);
   }

   /* Setup the key bindings. */
   for (x = 0; x < (int)SIZEOF (bindings); ++x)
      bindCDKObject (vFSELECT,
		     fselect,
		     (chtype)bindings[x].from,
		     getcCDKBind,
		     (void *)(long)bindings[x].to);

   registerCDKObject (cdkscreen, vFSELECT, fselect);

   return (fselect);
}

/*
 * This erases the file selector from the screen.
 */
static void _eraseCDKFselect (CDKOBJS *object)
{
   if (validCDKObject (object))
   {
      CDKFSELECT *fselect = (CDKFSELECT *)object;

      eraseCDKScroll (fselect->scrollField);
      eraseCDKEntry (fselect->entryField);
      eraseCursesWindow (fselect->win);
   }
}

/*
 * This moves the fselect field to the given location.
 */
static void _moveCDKFselect (CDKOBJS *object,
			     int xplace,
			     int yplace,
			     boolean relative,
			     boolean refresh_flag)
{
   CDKFSELECT *fselect = (CDKFSELECT *)object;
   /* *INDENT-EQLS* */
   int currentX = getbegx (fselect->win);
   int currentY = getbegy (fselect->win);
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
      xpos = getbegx (fselect->win) + xplace;
      ypos = getbegy (fselect->win) + yplace;
   }

   /* Adjust the window if we need to. */
   alignxy (WindowOf (fselect), &xpos, &ypos, fselect->boxWidth, fselect->boxHeight);

   /* Get the difference. */
   xdiff = currentX - xpos;
   ydiff = currentY - ypos;

   /* Move the window to the new location. */
   moveCursesWindow (fselect->win, -xdiff, -ydiff);
   moveCursesWindow (fselect->shadowWin, -xdiff, -ydiff);

   /* Move the sub-widgets. */
   moveCDKEntry (fselect->entryField, xplace, yplace, relative, FALSE);
   moveCDKScroll (fselect->scrollField, xplace, yplace, relative, FALSE);

   /* Redraw the window, if they asked for it. */
   if (refresh_flag)
   {
      drawCDKFselect (fselect, ObjOf (fselect)->box);
   }
}

/*
 * The fselect's focus resides in the entry widget.  But the scroll widget
 * will not draw items highlighted unless it has focus.  Temporarily adjust the
 * focus of the scroll widget when drawing on it to get the right highlighting.
 */
#define SaveFocus(widget) \
   boolean save = HasFocusObj (ObjOf (widget->scrollField)); \
   HasFocusObj (ObjOf (widget->scrollField)) = \
   HasFocusObj (ObjOf (widget->entryField))

#define RestoreFocus(widget) \
   HasFocusObj (ObjOf (widget->scrollField)) = save

static void drawMyScroller (CDKFSELECT *widget)
{
   SaveFocus (widget);
   drawCDKScroll (widget->scrollField, ObjOf (widget->scrollField)->box);
   RestoreFocus (widget);
}

static void injectMyScroller (CDKFSELECT *widget, chtype key)
{
   SaveFocus (widget);
   (void)injectCDKScroll (widget->scrollField, key);
   RestoreFocus (widget);
}

/*
 * This draws the file selector widget.
 */
static void _drawCDKFselect (CDKOBJS *object, boolean Box GCC_UNUSED)
{
   CDKFSELECT *fselect = (CDKFSELECT *)object;

   /* Draw in the shadow if we need to. */
   if (fselect->shadowWin != 0)
   {
      drawShadow (fselect->shadowWin);
   }

   /* Draw in the entry field. */
   drawCDKEntry (fselect->entryField, ObjOf (fselect->entryField)->box);

   /* Draw in the scroll field. */
   drawMyScroller (fselect);
}

/*
 * This means you want to use the given file selector. It takes input
 * from the keyboard, and when it's done, it fills the entry info
 * element of the structure with what was typed.
 */
char *activateCDKFselect (CDKFSELECT *fselect, chtype *actions)
{
   boolean functionKey;
   char *ret = 0;

   /* Draw the widget. */
   drawCDKFselect (fselect, ObjOf (fselect)->box);

   if (actions == 0)
   {
      for (;;)
      {
	 chtype input = (chtype)getchCDKObject (ObjOf (fselect->entryField), &functionKey);

	 /* Inject the character into the widget. */
	 ret = injectCDKFselect (fselect, input);
	 if (fselect->exitType != vEARLY_EXIT)
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
	 ret = injectCDKFselect (fselect, actions[x]);
	 if (fselect->exitType != vEARLY_EXIT)
	 {
	    return ret;
	 }
      }
   }

   /* Set the exit type and exit. */
   setExitType (fselect, 0);
   return 0;
}

/*
 * This injects a single character into the file selector.
 */
static int _injectCDKFselect (CDKOBJS *object, chtype input)
{
   CDKFSELECT *fselect = (CDKFSELECT *)object;
   char *filename;
   boolean file;
   char *ret = unknownString;
   bool complete = FALSE;

   /* Let the user play. */
   filename = injectCDKEntry (fselect->entryField, input);

   /* Copy the entry field exitType to the fileselector. */
   copyExitType (fselect, fselect->entryField);

   /* If we exited early, make sure we don't interpret it as a file. */
   if (fselect->exitType == vEARLY_EXIT)
   {
      return 0;
   }

   /* Can we change into the directory? */
   file = chdir (filename);
   if (chdir (fselect->pwd) != 0)
   {
      return 0;
   }

   /* If it's not a directory, return the filename. */
   if (file != 0)
   {
      /* It's a regular file, create the full path. */
      fselect->pathname = copyChar (filename);

      /* Return the complete pathname. */
      ret = (fselect->pathname);
      complete = TRUE;
   }
   else
   {
      /* Set the file selector information. */
      setCDKFselect (fselect, filename,
		     fselect->fieldAttribute, fselect->fillerCharacter,
		     fselect->highlight,
		     fselect->dirAttribute, fselect->fileAttribute,
		     fselect->linkAttribute, fselect->sockAttribute,
		     ObjOf (fselect)->box);

      /* Redraw the scrolling list. */
      drawMyScroller (fselect);
   }

   if (!complete)
      setExitType (fselect, 0);

   ResultOf (fselect).valueString = ret;
   return (ret != unknownString);
}

/*
 * This function sets the information inside the file selector.
 */
void setCDKFselect (CDKFSELECT *fselect,
		    const char *directory,
		    chtype fieldAttrib,
		    chtype filler,
		    chtype highlight,
		    const char *dirAttribute,
		    const char *fileAttribute,
		    const char *linkAttribute,
		    const char *sockAttribute,
		    boolean Box GCC_UNUSED)
{
   /* *INDENT-EQLS* */
   CDKSCROLL *fscroll   = fselect->scrollField;
   CDKENTRY *fentry     = fselect->entryField;
   char *tempDir        = 0;

   /* Keep the info sent to us. */
   fselect->fieldAttribute = fieldAttrib;
   fselect->fillerCharacter = filler;
   fselect->highlight = highlight;

   /* Set the attributes of the entry field/scrolling list. */
   setCDKEntryFillerChar (fentry, filler);
   setCDKScrollHighlight (fscroll, highlight);

   /* Only do the directory stuff if the directory is not null. */
   if (directory != 0)
   {
      char *newDirectory;

      /* Try to expand the directory if it starts with a ~ */
      if ((tempDir = expandTilde (directory)) != 0)
      {
	 newDirectory = tempDir;
      }
      else
      {
	 newDirectory = copyChar (directory);
      }

      /* Change directories. */
      if (chdir (newDirectory) != 0)
      {
	 char *mesg[10];

	 Beep ();

	 /* Could not get into the directory, pop up a little message. */
	 mesg[0] = format1String ("<C>Could not change into %s", newDirectory);
	 mesg[1] = errorMessage ("<C></U>%s");
	 mesg[2] = copyChar (" ");
	 mesg[3] = copyChar ("<C>Press Any Key To Continue.");

	 /* Pop Up a message. */
	 popupLabel (ScreenOf (fselect), (CDK_CSTRING2)mesg, 4);

	 /* Clean up some memory. */
	 freeCharList (mesg, 4);

	 /* Get out of here. */
	 eraseCDKFselect (fselect);
	 drawCDKFselect (fselect, ObjOf (fselect)->box);
	 freeChar (newDirectory);
	 return;
      }
      freeChar (newDirectory);
   }

   /*
    * If the information coming in is the same as the information
    * that is already there, there is no need to destroy it.
    */
   if (fselect->pwd != directory)
   {
      setPWD (fselect);
   }
   if (fselect->fileAttribute != fileAttribute)
   {
      /* Remove the old pointer and set the new value. */
      freeChar (fselect->fileAttribute);
      fselect->fileAttribute = copyChar (fileAttribute);
   }
   if (fselect->dirAttribute != dirAttribute)
   {
      /* Remove the old pointer and set the new value. */
      freeChar (fselect->dirAttribute);
      fselect->dirAttribute = copyChar (dirAttribute);
   }
   if (fselect->linkAttribute != linkAttribute)
   {
      /* Remove the old pointer and set the new value. */
      freeChar (fselect->linkAttribute);
      fselect->linkAttribute = copyChar (linkAttribute);
   }
   if (fselect->sockAttribute != sockAttribute)
   {
      /* Remove the old pointer and set the new value. */
      freeChar (fselect->sockAttribute);
      fselect->sockAttribute = copyChar (sockAttribute);
   }

   /* Set the contents of the entry field. */
   setCDKEntryValue (fentry, fselect->pwd);
   drawCDKEntry (fentry, ObjOf (fentry)->box);

   /* Get the directory contents. */
   if (setCDKFselectDirContents (fselect) == 0)
   {
      Beep ();
      return;
   }

   /* Set the values in the scrolling list. */
   setCDKScrollItems (fscroll,
		      (CDK_CSTRING2)fselect->dirContents,
		      fselect->fileCounter,
		      FALSE);
}

/*
 * This creates a list of the files in the current directory.
 */
int setCDKFselectDirContents (CDKFSELECT *fselect)
{
   struct stat fileStat;
   char **dirList = 0;
   int fileCount;
   int x = 0;

   /* Get the directory contents. */
   fileCount = CDKgetDirectoryContents (fselect->pwd, &dirList);
   if (fileCount <= 0)
   {
      /* We couldn't read the directory. Return. */
      CDKfreeStrings (dirList);
      return 0;
   }

   /* Clean out the old directory list. */
   CDKfreeStrings (fselect->dirContents);
   fselect->dirContents = dirList;
   fselect->fileCounter = fileCount;

   /* Set the properties of the files. */
   for (x = 0; x < fselect->fileCounter; x++)
   {
      char *oldItem;
      const char *attr = "";
      const char *mode = "?";

      /* FIXME: access() would give a more correct answer */
      if (lstat (dirList[x], &fileStat) == 0)
      {
	 mode = " ";
	 if ((fileStat.st_mode & S_IXUSR) != 0)
	 {
	    mode = "*";
	 }
#if defined (S_IXGRP) && defined (S_IXOTH)
	 else if (((fileStat.st_mode & S_IXGRP) != 0) ||
		  ((fileStat.st_mode & S_IXOTH) != 0))
	 {
	    mode = "*";
	 }
#endif
      }

      switch (mode2Filetype (fileStat.st_mode))
      {
      case 'l':
	 attr = fselect->linkAttribute;
	 mode = "@";
	 break;
      case '@':
	 attr = fselect->sockAttribute;
	 mode = "&";
	 break;
      case '-':
	 attr = fselect->fileAttribute;
	 break;
      case 'd':
	 attr = fselect->dirAttribute;
	 mode = "/";
	 break;
      default:
	 break;
      }
      oldItem = dirList[x];
      fselect->dirContents[x] = format3String ("%s%s%s", attr, dirList[x], mode);
      free (oldItem);
   }
   return 1;
}

char **getCDKFselectDirContents (CDKFSELECT *fselect, int *count)
{
   (*count) = fselect->fileCounter;
   return fselect->dirContents;
}

/*
 * This sets the current directory of the file selector.
 */
int setCDKFselectDirectory (CDKFSELECT *fselect, const char *directory)
{
   /* *INDENT-EQLS* */
   CDKENTRY *fentry     = fselect->entryField;
   CDKSCROLL *fscroll   = fselect->scrollField;
   int result           = 1;

   /*
    * If the directory supplied is the same as what is already
    * there, return.
    */
   if (fselect->pwd != directory)
   {
      /* Try to chdir into the given directory. */
      if (chdir (directory) != 0)
      {
	 result = 0;
      }
      else
      {
	 setPWD (fselect);

	 /* Set the contents of the entry field. */
	 setCDKEntryValue (fentry, fselect->pwd);
	 drawCDKEntry (fentry, ObjOf (fentry)->box);

	 /* Get the directory contents. */
	 if (setCDKFselectDirContents (fselect) == 0)
	 {
	    result = 0;
	 }
	 else
	 {
	    /* Set the values in the scrolling list. */
	    setCDKScrollItems (fscroll,
			       (CDK_CSTRING2)fselect->dirContents,
			       fselect->fileCounter,
			       FALSE);
	 }
      }
   }
   return result;
}
char *getCDKFselectDirectory (CDKFSELECT *fselect)
{
   return fselect->pwd;
}

/*
 * This sets the filler character of the entry field.
 */
void setCDKFselectFillerChar (CDKFSELECT *fselect, chtype filler)
{
   CDKENTRY *fentry = fselect->entryField;
   fselect->fillerCharacter = filler;
   setCDKEntryFillerChar (fentry, filler);
}
chtype getCDKFselectFillerChar (CDKFSELECT *fselect)
{
   return fselect->fillerCharacter;
}

/*
 * This sets the highlight bar of the scrolling list.
 */
void setCDKFselectHighlight (CDKFSELECT *fselect, chtype highlight)
{
   CDKSCROLL *fscroll = (CDKSCROLL *)fselect->scrollField;
   fselect->highlight = highlight;
   setCDKScrollHighlight (fscroll, highlight);

}
chtype getCDKFselectHighlight (CDKFSELECT *fselect)
{
   return fselect->highlight;
}

/*
 * This sets the attribute of the directory attribute in the
 * scrolling list.
 */
void setCDKFselectDirAttribute (CDKFSELECT *fselect, const char *attribute)
{
   /* Make sure they are not the same. */
   if (fselect->dirAttribute != attribute)
   {
      freeChar (fselect->dirAttribute);
      fselect->dirAttribute = copyChar (attribute);

      setCDKFselectDirContents (fselect);
   }
}
char *getCDKFselectDirAttribute (CDKFSELECT *fselect)
{
   return fselect->dirAttribute;
}

/*
 * This sets the attribute of the link attribute in the
 * scrolling list.
 */
void setCDKFselectLinkAttribute (CDKFSELECT *fselect, const char *attribute)
{
   /* Make sure they are not the same. */
   if (fselect->linkAttribute != attribute)
   {
      freeChar (fselect->linkAttribute);
      fselect->linkAttribute = copyChar (attribute);

      setCDKFselectDirContents (fselect);
   }
}
char *getCDKFselectLinkAttribute (CDKFSELECT *fselect)
{
   return fselect->linkAttribute;
}

/*
 * This sets the attribute of the link attribute in the
 * scrolling list.
 */
void setCDKFselectSocketAttribute (CDKFSELECT *fselect, const char *attribute)
{
   /* Make sure they are not the same. */
   if (fselect->sockAttribute != attribute)
   {
      freeChar (fselect->sockAttribute);
      fselect->sockAttribute = copyChar (attribute);

      setCDKFselectDirContents (fselect);
   }
}
char *getCDKFselectSocketAttribute (CDKFSELECT *fselect)
{
   return fselect->sockAttribute;
}

/*
 * This sets the attribute of the link attribute in the
 * scrolling list.
 */
void setCDKFselectFileAttribute (CDKFSELECT *fselect, const char *attribute)
{
   /* Make sure they are not the same. */
   if (fselect->fileAttribute != attribute)
   {
      freeChar (fselect->fileAttribute);
      fselect->fileAttribute = copyChar (attribute);

      setCDKFselectDirContents (fselect);
   }
}
char *getCDKFselectFileAttribute (CDKFSELECT *fselect)
{
   return fselect->fileAttribute;
}

/*
 * This sets the box attribute of the widget.
 */
void setCDKFselectBox (CDKFSELECT *fselect, boolean Box)
{
   ObjOf (fselect)->box = Box;
   ObjOf (fselect)->borderSize = Box ? 1 : 0;
}
boolean getCDKFselectBox (CDKFSELECT *fselect)
{
   return ObjOf (fselect)->box;
}

/*
 * This sets the contents of the widget
 */
void setCDKFselectContents (CDKFSELECT *widget,
			    CDK_CSTRING2 list,
			    int listSize)
{
   /* *INDENT-EQLS* */
   CDKSCROLL *scrollp   = widget->scrollField;
   CDKENTRY *entry      = widget->entryField;

   if (!createList (widget, list, listSize))
      return;

   /* Set the information in the scrolling list. */
   setCDKScroll (scrollp,
		 (CDK_CSTRING2)widget->dirContents,
		 widget->fileCounter,
		 NONUMBERS,
		 scrollp->highlight,
		 ObjOf (scrollp)->box);

   /* Clean out the entry field. */
   setCDKFselectCurrentItem (widget, 0);
   cleanCDKEntry (entry);

   /* Redraw the widget. */
   eraseCDKFselect (widget);
   drawCDKFselect (widget, ObjOf (widget)->box);
}

char **getCDKFselectContents (CDKFSELECT *widget,
			      int *size)
{
   (*size) = widget->fileCounter;
   return widget->dirContents;
}

/*
 * Get/set the current position in the scroll-widget.
 */
int getCDKFselectCurrentItem (CDKFSELECT *widget)
{
   return getCDKScrollCurrent (widget->scrollField);
}

void setCDKFselectCurrentItem (CDKFSELECT *widget,
			       int item)
{
   if (widget->fileCounter != 0)
   {
      char *data;

      setCDKScrollCurrent (widget->scrollField, item);

      data = contentToPath (widget,
			    widget->dirContents[getCDKScrollCurrentItem (widget->scrollField)]);
      setCDKEntryValue (widget->entryField, data);

      free (data);
   }
}

/*
 * These functions set the drawing characters of the widget.
 */
static void _setMyULchar (CDKOBJS *object, chtype character)
{
   CDKFSELECT *fselect = (CDKFSELECT *)object;

   setCDKEntryULChar (fselect->entryField, character);
}
static void _setMyURchar (CDKOBJS *object, chtype character)
{
   CDKFSELECT *fselect = (CDKFSELECT *)object;

   setCDKEntryURChar (fselect->entryField, character);
}
static void _setMyLLchar (CDKOBJS *object, chtype character)
{
   CDKFSELECT *fselect = (CDKFSELECT *)object;

   setCDKScrollLLChar (fselect->scrollField, character);
}
static void _setMyLRchar (CDKOBJS *object, chtype character)
{
   CDKFSELECT *fselect = (CDKFSELECT *)object;

   setCDKScrollLRChar (fselect->scrollField, character);
}
static void _setMyVTchar (CDKOBJS *object, chtype character)
{
   CDKFSELECT *fselect = (CDKFSELECT *)object;

   setCDKEntryVerticalChar (fselect->entryField, character);
   setCDKScrollVerticalChar (fselect->scrollField, character);
}
static void _setMyHZchar (CDKOBJS *object, chtype character)
{
   CDKFSELECT *fselect = (CDKFSELECT *)object;

   setCDKEntryHorizontalChar (fselect->entryField, character);
   setCDKScrollHorizontalChar (fselect->scrollField, character);
}
static void _setMyBXattr (CDKOBJS *object, chtype character)
{
   CDKFSELECT *fselect = (CDKFSELECT *)object;

   setCDKEntryBoxAttribute (fselect->entryField, character);
   setCDKScrollBoxAttribute (fselect->scrollField, character);
}

/*
 * This sets the background attribute of the widget.
 */
static void _setBKattrFselect (CDKOBJS *object, chtype attrib)
{
   if (object != 0)
   {
      CDKFSELECT *widget = (CDKFSELECT *)object;

      setCDKEntryBackgroundAttrib (widget->entryField, attrib);
      setCDKScrollBackgroundAttrib (widget->scrollField, attrib);
   }
}

/*
 * This destroys the file selector.
 */
static void _destroyCDKFselect (CDKOBJS *object)
{
   if (object != 0)
   {
      CDKFSELECT *fselect = (CDKFSELECT *)object;

      cleanCDKObjectBindings (vFSELECT, fselect);

      /* Free up the character pointers. */
      freeChar (fselect->pwd);
      freeChar (fselect->pathname);
      freeChar (fselect->dirAttribute);
      freeChar (fselect->fileAttribute);
      freeChar (fselect->linkAttribute);
      freeChar (fselect->sockAttribute);
      CDKfreeStrings (fselect->dirContents);

      /* Destroy the other Cdk objects. */
      destroyCDKScroll (fselect->scrollField);
      destroyCDKEntry (fselect->entryField);

      /* Free up the window pointers. */
      deleteCursesWindow (fselect->shadowWin);
      deleteCursesWindow (fselect->win);

      /* Clean the key bindings. */
      /* Unregister the object. */
      unregisterCDKObject (vFSELECT, fselect);
   }
}

/*
 ********************************
 * Callback functions.
 ********************************
 */

/*
 * This is a callback to the scrolling list which displays information
 * about the current file. (and the whole directory as well)
 */
static int displayFileInfoCB (EObjectType objectType GCC_UNUSED,
			      void *object,
			      void *clientData,
			      chtype key GCC_UNUSED)
{
   CDKENTRY *entry = (CDKENTRY *)object;
   CDKFSELECT *fselect = (CDKFSELECT *)clientData;
   CDKLABEL *infoLabel;
   struct stat fileStat;
#ifdef HAVE_PWD_H
   struct passwd *pwEnt;
   struct group *grEnt;
#endif
   char *filename;
   const char *filetype;
   char *mesg[10];
   char stringMode[15];
   int intMode;
   boolean functionKey;

   filename = fselect->entryField->info;

   if (lstat (filename, &fileStat) == 0)
   {
      switch (mode2Filetype (fileStat.st_mode))
      {
      case 'l':
	 filetype = "Symbolic Link";
	 break;
      case '@':
	 filetype = "Socket";
	 break;
      case '-':
	 filetype = "Regular File";
	 break;
      case 'd':
	 filetype = "Directory";
	 break;
      case 'c':
	 filetype = "Character Device";
	 break;
      case 'b':
	 filetype = "Block Device";
	 break;
      case '&':
	 filetype = "FIFO Device";
	 break;
      default:
	 filetype = "Unknown";
	 break;
      }
   }
   else
   {
      filetype = "Unknown";
   }

   /* Get the user name and group name. */
#ifdef HAVE_PWD_H
   pwEnt = getpwuid (fileStat.st_uid);
   grEnt = getgrgid (fileStat.st_gid);
#endif

   /* Convert the mode_t type to both string and int. */
   intMode = mode2Char (stringMode, fileStat.st_mode);

   /* Create the message. */
   mesg[0] = format1String ("Directory  : </U>%s", fselect->pwd);
   mesg[1] = format1String ("Filename   : </U>%s", filename);
#ifdef HAVE_PWD_H
   mesg[2] = format1StrVal ("Owner      : </U>%s<!U> (%d)",
			    pwEnt->pw_name,
			    (int)fileStat.st_uid);
   mesg[3] = format1StrVal ("Group      : </U>%s<!U> (%d)",
			    grEnt->gr_name,
			    (int)fileStat.st_gid);
#else
   mesg[2] = format1Number ("Owner      : (%ld)", (long)fileStat.st_uid);
   mesg[3] = format1Number ("Group      : (%ld)", (long)fileStat.st_gid);
#endif
   mesg[4] = format1StrVal ("Permissions: </U>%s<!U> (%o)", stringMode, intMode);
   mesg[5] = format1Number ("Size       : </U>%ld<!U> bytes", (long)fileStat.st_size);
   mesg[6] = format1Date ("Last Access: </U>%s", fileStat.st_atime);
   mesg[7] = format1Date ("Last Change: </U>%s", fileStat.st_ctime);
   mesg[8] = format1String ("File Type  : </U>%s", filetype);

   /* Create the pop up label. */
   infoLabel = newCDKLabel (entry->obj.screen,
			    CENTER, CENTER,
			    (CDK_CSTRING2)mesg, 9,
			    TRUE, FALSE);
   drawCDKLabel (infoLabel, TRUE);
   getchCDKObject (ObjOf (infoLabel), &functionKey);

   /* Clean up some memory. */
   destroyCDKLabel (infoLabel);
   freeCharList (mesg, 9);

   /* Redraw the file selector. */
   drawCDKFselect (fselect, ObjOf (fselect)->box);
   return (TRUE);
}

static char *make_pathname (const char *directory, const char *filename)
{
   size_t need = strlen (filename) + 2;
   bool root = (strcmp (directory, "/") == 0);
   char *result;

   if (!root)
      need += strlen (directory);
   if ((result = (char *)malloc (need)) != 0)
   {
      if (root)
	 sprintf (result, "/%s", filename);
      else
	 sprintf (result, "%s/%s", directory, filename);
   }
   return result;
}

/*
 * Return the plain string that corresponds to an item in dirContents[].
 */
static char *contentToPath (CDKFSELECT *fselect, char *content)
{
   chtype *tempChtype;
   char *tempChar;
   char *result;
   int j, j2;

   tempChtype = char2Chtype (content, &j, &j2);
   tempChar = chtype2Char (tempChtype);
   trim1Char (tempChar);	/* trim the 'mode' stored on the end */

   /* Create the pathname. */
   result = make_pathname (fselect->pwd, tempChar);

   /* Clean up. */
   freeChtype (tempChtype);
   freeChar (tempChar);
   return result;
}

/*
 * This tries to complete the filename.
 */
static int completeFilenameCB (EObjectType objectType GCC_UNUSED,
			       void *object GCC_UNUSED,
			       void *clientData,
			       chtype key GCC_UNUSED)
{
   /* *INDENT-EQLS* */
   CDKFSELECT *fselect  = (CDKFSELECT *)clientData;
   CDKSCROLL *scrollp   = fselect->scrollField;
   CDKENTRY *entry      = fselect->entryField;
   char *filename       = copyChar (entry->info);
   char *mydirname      = dirName (filename);
   char *newFilename    = 0;
   size_t filenameLen   = 0;
   int isDirectory;
   char **list;

   /* Make sure the filename is not null/empty. */
   if (filename == 0 ||
       (filenameLen = strlen (filename)) == 0)
   {
      Beep ();
      freeChar (filename);
      freeChar (mydirname);
      return (TRUE);
   }

   /* Try to expand the filename if it starts with a ~ */
   if ((newFilename = expandTilde (filename)) != 0)
   {
      freeChar (filename);
      filename = newFilename;
      setCDKEntryValue (entry, filename);
      drawCDKEntry (entry, ObjOf (entry)->box);
   }

   /* Make sure we can change into the directory. */
   isDirectory = chdir (filename);
   if (chdir (fselect->pwd) != 0)
   {
      freeChar (filename);
      freeChar (mydirname);
      return FALSE;
   }

   setCDKFselect (fselect,
		  isDirectory ? mydirname : filename,
		  fselect->fieldAttribute,
		  fselect->fillerCharacter,
		  fselect->highlight,
		  fselect->dirAttribute,
		  fselect->fileAttribute,
		  fselect->linkAttribute,
		  fselect->sockAttribute,
		  ObjOf (fselect)->box);
   freeChar (mydirname);

   /* If we can, change into the directory. */
   if (isDirectory)
   {
      /*
       * Set the entry field with the filename so the current
       * filename selection shows up.
       */
      setCDKEntryValue (entry, filename);
      drawCDKEntry (entry, ObjOf (entry)->box);
   }

   /* Create the file list. */
   if ((list = typeMallocN (char *, fselect->fileCounter)) != 0)
   {
      int Index, x;

      for (x = 0; x < fselect->fileCounter; x++)
      {
	 list[x] = contentToPath (fselect, fselect->dirContents[x]);
      }

      /* Look for a unique filename match. */
      Index = searchList ((CDK_CSTRING2)list, fselect->fileCounter, filename);

      /* If the index is less than zero, return we didn't find a match. */
      if (Index < 0)
      {
	 Beep ();
      }
      else
      {
	 /* Move to the current item in the scrolling list. */
	 int difference = Index - scrollp->currentItem;
	 int absoluteDifference = abs (difference);
	 if (difference < 0)
	 {
	    for (x = 0; x < absoluteDifference; x++)
	    {
	       injectMyScroller (fselect, KEY_UP);
	    }
	 }
	 else if (difference > 0)
	 {
	    for (x = 0; x < absoluteDifference; x++)
	    {
	       injectMyScroller (fselect, KEY_DOWN);
	    }
	 }
	 drawMyScroller (fselect);

	 /* Ok, we found a match, is the next item similar? */
	 if (Index + 1 < fselect->fileCounter &&
	     0 != list[Index + 1] &&
	     0 == strncmp (list[Index + 1], filename, filenameLen))
	 {
	    int currentIndex = Index;
	    int baseChars = (int)filenameLen;
	    int matches = 0;

	    /* Determine the number of files which match. */
	    while (currentIndex < fselect->fileCounter)
	    {
	       if (list[currentIndex] != 0)
	       {
		  if (strncmp (list[currentIndex], filename, filenameLen) == 0)
		  {
		     matches++;
		  }
	       }
	       currentIndex++;
	    }

	    /* Start looking for the common base characters. */
	    for (;;)
	    {
	       int secondaryMatches = 0;
	       for (x = Index; x < Index + matches; x++)
	       {
		  if (list[Index][baseChars] == list[x][baseChars])
		  {
		     secondaryMatches++;
		  }
	       }

	       if (secondaryMatches != matches)
	       {
		  Beep ();
		  break;
	       }

	       /* Inject the character into the entry field. */
	       (void)injectCDKEntry (fselect->entryField,
				     (chtype)list[Index][baseChars]);
	       baseChars++;
	    }
	 }
	 else
	 {
	    /* Set the entry field with the found item. */
	    setCDKEntryValue (entry, list[Index]);
	    drawCDKEntry (entry, ObjOf (entry)->box);
	 }
      }
      freeCharList (list, (unsigned)fselect->fileCounter);
      free (list);
   }
   freeChar (filename);
   return (TRUE);
}

/*
 * This allows the user to delete a file.
 */
void deleteFileCB (EObjectType objectType GCC_UNUSED, void *object, void *clientData)
{
   CDKSCROLL *fscroll = (CDKSCROLL *)object;
   CDKFSELECT *fselect = (CDKFSELECT *)clientData;
   const char *buttons[] =
   {
      "No",
      "Yes"
   };
   CDKDIALOG *question;
   char *mesg[10], *filename;

   /* Get the filename which is to be deleted. */
   filename = chtype2Char (fscroll->item[fscroll->currentItem]);
   trim1Char (filename);

   /* Create the dialog message. */
   mesg[0] = copyChar ("<C>Are you sure you want to delete the file:");
   mesg[1] = format1String ("<C></U>\"%s\"?", filename);

   /* Create the dialog box. */
   question = newCDKDialog (ScreenOf (fselect), CENTER, CENTER,
			    (CDK_CSTRING2)mesg, 2,
			    (CDK_CSTRING2)buttons, 2,
			    A_REVERSE, TRUE, TRUE, FALSE);
   freeCharList (mesg, 2);

   /* If the said yes then try to nuke it. */
   if (activateCDKDialog (question, 0) == 1)
   {
      /* If we were successful, reload the scrolling list. */
      if (unlink (filename) == 0)
      {
	 /* Set the file selector information. */
	 setCDKFselect (fselect, fselect->pwd,
			fselect->fieldAttribute,
			fselect->fillerCharacter,
			fselect->highlight,
			fselect->dirAttribute,
			fselect->fileAttribute,
			fselect->linkAttribute,
			fselect->sockAttribute,
			ObjOf (fselect)->box);
      }
      else
      {
	 /* Pop up a message. */
	 mesg[0] = copyChar (errorMessage ("<C>Cannot delete file: %s"));
	 mesg[1] = copyChar (" ");
	 mesg[2] = copyChar ("<C>Press any key to continue.");
	 popupLabel (ScreenOf (fselect), (CDK_CSTRING2)mesg, 3);
	 freeCharList (mesg, 3);
      }
   }

   /* Clean up. */
   destroyCDKDialog (question);

   /* Redraw the file selector. */
   drawCDKFselect (fselect, ObjOf (fselect)->box);
}

/*
 * This function sets the pre-process function.
 */
void setCDKFselectPreProcess (CDKFSELECT *fselect, PROCESSFN callback, void *data)
{
   setCDKEntryPreProcess (fselect->entryField, callback, data);
   setCDKScrollPreProcess (fselect->scrollField, callback, data);
}

/*
 * This function sets the post-process function.
 */
void setCDKFselectPostProcess (CDKFSELECT *fselect, PROCESSFN callback, void *data)
{
   setCDKEntryPostProcess (fselect->entryField, callback, data);
   setCDKScrollPostProcess (fselect->scrollField, callback, data);
}

/*
 * Start of callback functions.
 */
static int fselectAdjustScrollCB (EObjectType objectType GCC_UNUSED,
				  void *object GCC_UNUSED,
				  void *clientData,
				  chtype key)
{
   /* *INDENT-EQLS* */
   CDKFSELECT *fselect  = (CDKFSELECT *)clientData;
   CDKSCROLL *scrollp   = (CDKSCROLL *)fselect->scrollField;
   CDKENTRY *entry      = (CDKENTRY *)fselect->entryField;

   if (scrollp->listSize > 0)
   {
      char *current;
      char *temp;

      /* Move the scrolling list. */
      injectMyScroller (fselect, key);

      /* Get the currently highlighted filename. */
      current = chtype2Char (scrollp->item[scrollp->currentItem]);
      trim1Char (current);

      temp = make_pathname (fselect->pwd, current);

      /* Set the value in the entry field. */
      setCDKEntryValue (entry, temp);
      drawCDKEntry (entry, ObjOf (entry)->box);

      freeChar (current);
      freeChar (temp);
      return (TRUE);
   }
   Beep ();
   return (FALSE);
}

/*
 * trim the 'mode' from a copy of a dirContents[] entry.
 */
static char *trim1Char (char *source)
{
   size_t len;

   if ((len = strlen (source)) != 0)
      source[--len] = '\0';
   return source;
}

static char *format1Date (const char *format, time_t value)
{
   char *result;
   char *temp = ctime (&value);

   if ((result = (char *)malloc (strlen (format) + strlen (temp) + 1)) != 0)
   {
      sprintf (result, format, trim1Char (temp));
   }
   return result;
}

static char *format1Number (const char *format, long value)
{
   char *result;

   if ((result = (char *)malloc (strlen (format) + 20)) != 0)
      sprintf (result, format, value);
   return result;
}

static char *format3String (const char *format,
			    const char *s1,
			    const char *s2,
			    const char *s3)
{
   char *result;

   if ((result = (char *)malloc (strlen (format) +
				 strlen (s1) +
				 strlen (s2) +
				 strlen (s3))) != 0)
      sprintf (result, format, s1, s2, s3);
   return result;
}

static char *format1String (const char *format, const char *string)
{
   char *result;

   if ((result = (char *)malloc (strlen (format) + strlen (string))) != 0)
      sprintf (result, format, string);
   return result;
}

static char *format1StrVal (const char *format, const char *string, int value)
{
   char *result;

   if ((result = (char *)malloc (strlen (format) + strlen (string) + 20)) != 0)
      sprintf (result, format, string, value);
   return result;
}

static char *errorMessage (const char *format)
{
   char *message;
#ifdef HAVE_STRERROR
   message = strerror (errno);
#else
   message = "Unknown reason.";
#endif
   return format1String (format, message);
}

/*
 * This takes a ~ type account name and returns the full pathname.
 */
static char *expandTilde (const char *filename)
{
   char *result = 0;
   char *account;
   char *pathname;
   int len;

   /* Make sure the filename is not null/empty, and begins with a tilde */
   if ((filename != 0) &&
       (len = (int)strlen (filename)) != 0 &&
       filename[0] == '~' &&
       (account = copyChar (filename)) != 0 &&
       (pathname = copyChar (filename)) != 0)
   {
      bool slash = FALSE;
      const char *home;
      int x;
      int len_a = 0;
      int len_p = 0;
      struct passwd *accountInfo;

      /* Find the account name in the filename. */
      for (x = 1; x < len; x++)
      {
	 if (filename[x] == '/' && !slash)
	 {
	    slash = TRUE;
	 }
	 else if (slash)
	 {
	    pathname[len_p++] = filename[x];
	 }
	 else
	 {
	    account[len_a++] = filename[x];
	 }
      }
      account[len_a] = '\0';
      pathname[len_p] = '\0';

      home = 0;
#ifdef HAVE_PWD_H
      if (strlen (account) != 0 &&
	  (accountInfo = getpwnam (account)) != 0)
      {
	 home = accountInfo->pw_dir;
      }
#endif
      if (home == 0 || *home == '\0')
	 home = getenv ("HOME");
      if (home == 0 || *home == '\0')
	 home = "/";

      /*
       * Construct the full pathname. We do this because someone
       * may have a pathname at the end of the account name
       * and we want to keep it.
       */
      result = make_pathname (home, pathname);

      freeChar (account);
      freeChar (pathname);
   }
   return result;
}

/*
 * Store the name of the current working directory.
 */
static void setPWD (CDKFSELECT *fselect)
{
   char buffer[512];
   freeChar (fselect->pwd);
   if (getcwd (buffer, sizeof (buffer)) == 0)
      strcpy (buffer, ".");
   fselect->pwd = copyChar (buffer);
}

static void destroyInfo (CDKFSELECT *widget)
{
   CDKfreeStrings (widget->dirContents);
   widget->dirContents = 0;

   widget->fileCounter = 0;
}

static int createList (CDKFSELECT *widget, CDK_CSTRING2 list, int listSize)
{
   int status = 0;

   if (listSize >= 0)
   {
      char **newlist = typeCallocN (char *, listSize + 1);

      if (newlist != 0)
      {
	 int x;

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
	    destroyInfo (widget);
	    widget->fileCounter = listSize;
	    widget->dirContents = newlist;
	 }
	 else
	 {
	    CDKfreeStrings (newlist);
	 }
      }
   }
   else
   {
      destroyInfo (widget);
      status = TRUE;
   }
   return status;
}

static void _focusCDKFselect (CDKOBJS *object)
{
   CDKFSELECT *widget = (CDKFSELECT *)object;

   FocusObj (ObjOf (widget->entryField));
}

static void _unfocusCDKFselect (CDKOBJS *object)
{
   CDKFSELECT *widget = (CDKFSELECT *)object;

   UnfocusObj (ObjOf (widget->entryField));
}

dummyRefreshData (Fselect)

dummySaveData (Fselect)
