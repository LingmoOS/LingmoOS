/* $Id: scroll_ex.c,v 1.27 2021/01/09 22:42:39 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "scroll_ex";
#endif

static char *newLabel (const char *prefix)
{
   static int count;
   static char result[80];
   sprintf (result, "%s%d", prefix, ++count);
   return result;
}

static int addItemCB (EObjectType cdktype GCC_UNUSED, void *object,
		      void *clientData GCC_UNUSED,
		      chtype input GCC_UNUSED)
{
   CDKSCROLL *s = (CDKSCROLL *)object;

   addCDKScrollItem (s, newLabel ("add"));

   refreshCDKScreen (ScreenOf (s));

   return (TRUE);
}

static int insItemCB (EObjectType cdktype GCC_UNUSED, void *object,
		      void *clientData GCC_UNUSED,
		      chtype input GCC_UNUSED)
{
   CDKSCROLL *s = (CDKSCROLL *)object;

   insertCDKScrollItem (s, newLabel ("insert"));

   refreshCDKScreen (ScreenOf (s));

   return (TRUE);
}

static int delItemCB (EObjectType cdktype GCC_UNUSED, void *object,
		      void *clientData GCC_UNUSED,
		      chtype input GCC_UNUSED)
{
   CDKSCROLL *s = (CDKSCROLL *)object;

   deleteCDKScrollItem (s, getCDKScrollCurrent (s));

   refreshCDKScreen (ScreenOf (s));

   return (TRUE);
}

/*
 * This program demonstrates the Cdk scrolling list widget.
 *
 * Options (in addition to normal CLI parameters):
 *	-c	create the data after the widget
 *	-s SPOS	location for the scrollbar
 *	-t TEXT	title for the widget
 */
int main (int argc, char **argv)
{
   /* Declare variables. */
   CDKSCREEN *cdkscreen = 0;
   CDKSCROLL *scrollList = 0;
   const char *title = "<C></5>Pick a file";
   char **item = 0;
   const char *mesg[5];
   char temp[256];
   int selection, count;

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, "acs:t:" CDK_CLI_PARAMS);

   cdkscreen = initCDKScreen (NULL);

   /* Set up CDK Colors. */
   initCDKColor ();

   /* Use the current directory list to fill the radio list. */
   count = CDKgetDirectoryContents (".", &item);

   /* Create the scrolling list. */
   scrollList = newCDKScroll (cdkscreen,
			      CDKparamValue (&params, 'X', CENTER),
			      CDKparamValue (&params, 'Y', CENTER),
			      CDKparsePosition (CDKparamString2 (&params,
								 's',
								 "RIGHT")),
			      CDKparamValue (&params, 'H', 10),
			      CDKparamValue (&params, 'W', 50),
			      CDKparamString2 (&params, 't', title),
			      (CDKparamNumber (&params, 'c')
			       ? 0
			       : (CDK_CSTRING2)item),
			      (CDKparamNumber (&params, 'c')
			       ? 0
			       : count),
			      TRUE,
			      A_REVERSE,
			      CDKparamValue (&params, 'N', TRUE),
			      CDKparamValue (&params, 'S', FALSE));

   /* Is the scrolling list null? */
   if (scrollList == 0)
   {
      /* Exit CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot make scrolling list. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   if (CDKparamNumber (&params, 'c'))
   {
      setCDKScrollItems (scrollList, (CDK_CSTRING2)item, count, TRUE);
   }
   if (CDKparamNumber (&params, 'a'))
   {
      drawCDKScroll (scrollList, 1);

      setCDKScrollPosition (scrollList, 10);
      drawCDKScroll (scrollList, 1);
      napms (3000);

      setCDKScrollPosition (scrollList, 20);
      drawCDKScroll (scrollList, 1);
      napms (3000);

      setCDKScrollPosition (scrollList, 30);
      drawCDKScroll (scrollList, 1);
      napms (3000);

      setCDKScrollPosition (scrollList, 70);
      drawCDKScroll (scrollList, 1);
      napms (3000);
   }
   bindCDKObject (vSCROLL, scrollList, 'a', addItemCB, NULL);
   bindCDKObject (vSCROLL, scrollList, 'i', insItemCB, NULL);
   bindCDKObject (vSCROLL, scrollList, 'd', delItemCB, NULL);

   /* Activate the scrolling list. */

   selection = activateCDKScroll (scrollList, 0);

   /* Determine how the widget was exited. */
   if (scrollList->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No file selected.";
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);
   }
   else if (scrollList->exitType == vNORMAL)
   {
      char *theItem = chtype2Char (scrollList->item[selection]);
      mesg[0] = "<C>You selected the following file";
      sprintf (temp, "<C>%.*s", (int)(sizeof (temp) - 20), theItem);
      mesg[1] = temp;
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);
      freeChar (theItem);
   }

   /* Clean up. */
   CDKfreeStrings (item);
   destroyCDKScroll (scrollList);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}
