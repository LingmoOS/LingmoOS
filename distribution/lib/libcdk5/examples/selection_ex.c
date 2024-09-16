/* $Id: selection_ex.c,v 1.21 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "selection_ex";
#endif

/*
 * This program demonstrates the Cdk selection widget.
 *
 * Options (in addition to normal CLI parameters):
 *	-c	create the data after the widget
 *	-f TEXT title for a footer label
 *	-h TEXT title for a header label
 *	-s SPOS	location for the scrollbar
 *	-t TEXT	title for the widget
 *
 */
int main (int argc, char **argv)
{
   static const char *choices[] =
   {
      "   ",
      "-->"
   };
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen    = 0;
   CDKSELECTION *selection = 0;
   const char *title       = "<C></5>Pick one or more accounts.";
   char *title_string      = 0;
   char **item             = 0;
   char temp[256];
   const char *mesg[200];
#if defined (HAVE_PWD_H)
   struct passwd *ent;
#endif
   unsigned x, y;
   unsigned used           = 0;
   unsigned count          = 0;

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, "cf:h:s:t:" CDK_CLI_PARAMS);

   /* Use the account names to create a list. */
   count = 0;
#if defined (HAVE_PWD_H)
   while ((ent = getpwent ()) != 0)
   {
      used = CDKallocStrings (&item, ent->pw_name, count++, used);
   }
   endpwent ();
#endif
   count--;

   cdkscreen = initCDKScreen (NULL);

   /* Set up CDK Colors. */
   initCDKColor ();

   if ((title_string = CDKparamString2 (&params, 'h', 0)) != 0)
   {
      const char *list[2];
      CDKLABEL *header;

      list[0] = title_string;
      list[1] = 0;
      header = newCDKLabel (cdkscreen,
			    CDKparamValue (&params, 'X', CENTER),
			    CDKparamValue (&params, 'Y', TOP),
			    (CDK_CSTRING2)list, 1,
			    CDKparamValue (&params, 'N', TRUE),
			    CDKparamValue (&params, 'S', TRUE));
      if (header != 0)
	 activateCDKLabel (header, 0);
   }

   if ((title_string = CDKparamString2 (&params, 'f', 0)) != 0)
   {
      const char *list[2];
      CDKLABEL *footer;

      list[0] = title_string;
      list[1] = 0;
      footer = newCDKLabel (cdkscreen,
			    CDKparamValue (&params, 'X', CENTER),
			    CDKparamValue (&params, 'Y', BOTTOM),
			    (CDK_CSTRING2)list, 1,
			    CDKparamValue (&params, 'N', TRUE),
			    CDKparamValue (&params, 'S', TRUE));
      if (footer != 0)
	 activateCDKLabel (footer, 0);
   }
   /* Create the selection list. */
   selection = newCDKSelection (cdkscreen,
				CDKparamValue (&params, 'X', CENTER),
				CDKparamValue (&params, 'Y', CENTER),
				CDKparsePosition
				(CDKparamString2 (&params, 's', "RIGHT")),
				CDKparamValue (&params, 'H', 10),
				CDKparamValue (&params, 'W', 50),
				CDKparamString2 (&params, 't', title),
				(CDKparamNumber (&params, 'c')
				 ? 0
				 : (CDK_CSTRING2)item),
				(CDKparamNumber (&params, 'c')
				 ? 0
				 : (int)count),
				(CDK_CSTRING2)choices, 2,
				A_REVERSE,
				CDKparamValue (&params, 'N', TRUE),
				CDKparamValue (&params, 'S', FALSE));

   /* Is the selection list null? */
   if (selection == 0)
   {
      /* Exit CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot to create the selection list.\n");
      printf ("Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   if (CDKparamNumber (&params, 'c'))
   {
      setCDKSelectionItems (selection, (CDK_CSTRING2)item, (int)count);
   }

   /* Activate the selection list. */
   activateCDKSelection (selection, 0);

   /* Check the exit status of the widget. */
   if (selection->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No items selected.";
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);
   }
   else if (selection->exitType == vNORMAL)
   {
      mesg[0] = "<C>Here are the accounts you selected.";
      y = 1;
      for (x = 0; x < count; x++)
      {
	 if (selection->selections[x] == 1)
	 {
	    sprintf (temp, "<C></5>%.*s", (int)(sizeof (temp) - 20), item[x]);
	    mesg[y++] = copyChar (temp);
	 }
      }
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, (int)y);
   }

   /* Clean up. */
   CDKfreeStrings (item);
   destroyCDKSelection (selection);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}
