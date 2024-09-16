/* $Id: fselect_ex.c,v 1.27 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "fselect_ex";
#endif

/*
 * This program demonstrates the file selector and the viewer widget.
 */
static CDKSCREEN *cdkscreen = 0;
static char **myUserList = 0;
static int userSize;

typedef struct
{
   int deleted;			/* index in current list which is deleted */
   int original;		/* index in myUserList[] of deleted item */
   int position;		/* position before delete */
   int topline;			/* top-line before delete */
} UNDO;

static UNDO *myUndoList;
static int undoSize;

#define CB_PARAMS EObjectType cdktype GCC_UNUSED, void* object GCC_UNUSED, void* clientdata GCC_UNUSED, chtype key GCC_UNUSED

static void fill_undo (CDKFSELECT *widget, int deleted, char *data)
{
   int top = getCDKScrollCurrentTop (widget->scrollField);
   int item = getCDKFselectCurrentItem (widget);
   int n;

   myUndoList[undoSize].deleted = deleted;
   myUndoList[undoSize].topline = top;
   myUndoList[undoSize].original = -1;
   myUndoList[undoSize].position = item;
   for (n = 0; n < userSize; ++n)
   {
      if (!strcmp (myUserList[n], data))
      {
	 myUndoList[undoSize].original = n;
	 break;
      }
   }
   ++undoSize;
}

static int do_delete (CB_PARAMS)
{
   CDKFSELECT *widget = (CDKFSELECT *)clientdata;
   int size;
   char **list = getCDKFselectContents (widget, &size);
   int result = FALSE;

   if (size)
   {
      int save = getCDKScrollCurrentTop (widget->scrollField);
      int first = getCDKFselectCurrentItem (widget);
      int n;

      fill_undo (widget, first, list[first]);
      for (n = first; n < size; ++n)
	 list[n] = list[n + 1];
      setCDKFselectContents (widget, (CDK_CSTRING2)list, size - 1);
      setCDKScrollCurrentTop (widget->scrollField, save);
      setCDKFselectCurrentItem (widget, first);
      drawCDKFselect (widget, BorderOf (widget));
      result = TRUE;
   }
   return result;
}

static int do_delete1 (CB_PARAMS)
{
   CDKFSELECT *widget = (CDKFSELECT *)clientdata;
   int size;
   char **list = getCDKFselectContents (widget, &size);
   int result = FALSE;

   if (size)
   {
      int save = getCDKScrollCurrentTop (widget->scrollField);
      int first = getCDKFselectCurrentItem (widget);

      if (first-- > 0)
      {
	 int n;

	 fill_undo (widget, first, list[first]);
	 for (n = first; n < size; ++n)
	    list[n] = list[n + 1];
	 setCDKFselectContents (widget, (CDK_CSTRING2)list, size - 1);
	 setCDKScrollCurrentTop (widget->scrollField, save);
	 setCDKFselectCurrentItem (widget, first);
	 drawCDKFselect (widget, BorderOf (widget));
	 result = TRUE;
      }
   }
   return result;
}

static int do_help (CB_PARAMS)
{
   static const char *message[] =
   {
      "File Selection tests:",
      "",
      "F1 = help (this message)",
      "F2 = delete current item",
      "F3 = delete previous item",
      "F4 = reload all items",
      "F5 = undo deletion",
      0
   };
   popupLabel (cdkscreen,
	       (CDK_CSTRING2)message,
	       (int)CDKcountStrings ((CDK_CSTRING2)message));
   return TRUE;
}

static int do_reload (CB_PARAMS)
{
   int result = FALSE;

   if (userSize)
   {
      CDKFSELECT *widget = (CDKFSELECT *)clientdata;
      setCDKFselectContents (widget, (CDK_CSTRING2)myUserList, userSize);
      setCDKFselectCurrentItem (widget, 0);
      drawCDKFselect (widget, BorderOf (widget));
      result = TRUE;
   }
   return result;
}

static int do_undo (CB_PARAMS)
{
   int result = FALSE;

   if (undoSize > 0)
   {
      CDKFSELECT *widget = (CDKFSELECT *)clientdata;
      int size;
      int n;
      char **oldlist = getCDKFselectContents (widget, &size);
      char **newlist = (char **)malloc ((size_t) (++size + 1) * sizeof (char *));

      --undoSize;
      newlist[size] = 0;
      for (n = size - 1; n > myUndoList[undoSize].deleted; --n)
      {
	 newlist[n] = copyChar (oldlist[n - 1]);
      }
      newlist[n--] = copyChar (myUserList[myUndoList[undoSize].original]);
      while (n >= 0)
      {
	 newlist[n] = copyChar (oldlist[n]);
	 --n;
      }
      setCDKFselectContents (widget, (CDK_CSTRING2)newlist, size);
      setCDKScrollCurrentTop (widget->scrollField, myUndoList[undoSize].topline);
      setCDKFselectCurrentItem (widget, myUndoList[undoSize].position);
      drawCDKFselect (widget, BorderOf (widget));
      free (newlist);
      result = TRUE;
   }
   return result;
}

int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKVIEWER *example   = 0;
   CDKFSELECT *fSelect  = 0;
   const char *title    = "<C>Pick\n<C>A\n<C>File";
   const char *label    = "File: ";
   char **info          = 0;
   const char *button[5];
   const char *mesg[4];
   char *filename;
   char vTitle[256];
   char temp[256];
   int selected, lines;

   CDK_PARAMS params;
   char *directory;

   CDKparseParams (argc, argv, &params, "d:" CDK_CLI_PARAMS);
   directory = CDKparamString2 (&params, 'd', ".");

   /* Create the viewer buttons. */
   button[0] = "</5><OK><!5>";
   button[1] = "</5><Cancel><!5>";

   cdkscreen = initCDKScreen (NULL);

   /* Start color. */
   initCDKColor ();

   /* Get the filename. */
   fSelect = newCDKFselect (cdkscreen,
			    CDKparamValue (&params, 'X', CENTER),
			    CDKparamValue (&params, 'Y', CENTER),
			    CDKparamValue (&params, 'H', 20),
			    CDKparamValue (&params, 'W', 65),
			    title, label, A_NORMAL, '_', A_REVERSE,
			    "</5>", "</48>", "</N>", "</N>",
			    CDKparamValue (&params, 'N', TRUE),
			    CDKparamValue (&params, 'S', FALSE));

   if (fSelect == 0)
   {
      destroyCDKScreen (cdkscreen);
      endCDK ();

      fprintf (stderr, "Cannot create widget\n");
      ExitProgram (EXIT_FAILURE);
   }

   bindCDKObject (vFSELECT, fSelect, '?', do_help, NULL);
   bindCDKObject (vFSELECT, fSelect, KEY_F1, do_help, NULL);
   bindCDKObject (vFSELECT, fSelect, KEY_F2, do_delete, fSelect);
   bindCDKObject (vFSELECT, fSelect, KEY_F3, do_delete1, fSelect);
   bindCDKObject (vFSELECT, fSelect, KEY_F4, do_reload, fSelect);
   bindCDKObject (vFSELECT, fSelect, KEY_F5, do_undo, fSelect);

   /*
    * Set the starting directory. This is not necessary because when
    * the file selector starts it uses the present directory as a default.
    */
   setCDKFselect (fSelect, directory, A_NORMAL, ' ', A_REVERSE,
		  "</5>", "</48>", "</N>", "</N>", ObjOf (fSelect)->box);
   myUserList = copyCharList ((const char **)getCDKFselectContents (fSelect, &userSize));
   myUndoList = (UNDO *) malloc ((size_t) userSize * sizeof (UNDO));
   undoSize = 0;

   /* Activate the file selector. */
   filename = activateCDKFselect (fSelect, 0);

   /* Check how the person exited from the widget. */
   if (fSelect->exitType == vESCAPE_HIT)
   {
      /* Pop up a message for the user. */
      mesg[0] = "<C>Escape hit. No file selected.";
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);

      /* Exit CDK. */
      destroyCDKFselect (fSelect);
      destroyCDKScreen (cdkscreen);
      endCDK ();

      ExitProgram (EXIT_SUCCESS);
   }

   /* Create the file viewer to view the file selected. */
   example = newCDKViewer (cdkscreen, CENTER, CENTER, 20, -2,
			   (CDK_CSTRING2)button, 2, A_REVERSE, TRUE, FALSE);

   /* Could we create the viewer widget? */
   if (example == 0)
   {
      /* Exit CDK. */
      destroyCDKFselect (fSelect);
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Can't seem to create viewer. Is the window too small?\n");
      ExitProgram (EXIT_SUCCESS);
   }

   /* Open the file and read the contents. */
   lines = CDKreadFile (filename, &info);
   if (lines == -1)
   {
      filename = copyChar (filename);

      destroyCDKFselect (fSelect);
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Could not open \"%s\"\n", filename);

      ExitProgram (EXIT_FAILURE);
   }

   /* Set up the viewer title, and the contents to the widget. */
   sprintf (vTitle, "<C></B/21>Filename:<!21></22>%20s<!22!B>", filename);
   setCDKViewer (example, vTitle,
		 (CDK_CSTRING2)info, lines,
		 A_REVERSE, TRUE, TRUE, TRUE);

   CDKfreeStrings (info);

   /* Destroy the file selector widget. */
   destroyCDKFselect (fSelect);

   /* Activate the viewer widget. */
   selected = activateCDKViewer (example, 0);

   /* Check how the person exited from the widget. */
   if (example->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>Escape hit. No Button selected.";
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);
   }
   else if (example->exitType == vNORMAL)
   {
      sprintf (temp, "<C>You selected button %d", selected);
      mesg[0] = temp;
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);
   }

   /* Clean up. */
   destroyCDKViewer (example);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}
