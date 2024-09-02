/* $Id: preprocess_ex.c,v 1.20 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "preprocess_ex";
#endif

static BINDFN_PROTO (entryPreProcessCB);

/*
 * This demonstrates the Cdk preprocess feature.
 */
int main (void)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;
   CDKENTRY *widget     = 0;
   const char *title    = "<C>Type in anything you want\n<C>but the dreaded letter </B>G<!B>!";
   char *info;
   const char *mesg[10];
   char temp[256];

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK colors. */
   initCDKColor ();

   /* Create the entry field widget. */
   widget = newCDKEntry (cdkscreen, CENTER, CENTER,
			 title, 0, A_NORMAL, '.', vMIXED,
			 40, 0, 256, TRUE, FALSE);

   /* Is the widget null? */
   if (widget == 0)
   {
      /* Clean up. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot create the entry box. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   setCDKEntryPreProcess (widget, entryPreProcessCB, 0);

   /* Activate the entry field. */
   info = activateCDKEntry (widget, 0);

   /* Tell them what they typed. */
   if (widget->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No information passed back.";
      mesg[1] = "",
	 mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 3);
   }
   else if (widget->exitType == vNORMAL)
   {
      mesg[0] = "<C>You typed in the following";
      sprintf (temp, "<C>(%.*s)", (int)(sizeof (temp) - 20), info);
      mesg[1] = temp;
      mesg[2] = "";
      mesg[3] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2)mesg, 4);
   }

   /* Clean up and exit. */
   destroyCDKEntry (widget);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

static int entryPreProcessCB (EObjectType cdkType GCC_UNUSED, void *object,
			      void *clientData GCC_UNUSED,
			      chtype input)
{
   CDKENTRY *entry = (CDKENTRY *)object;

   /* Check the input. */
   if ((input == 'g') || (input == 'G'))
   {
      /* *INDENT-EQLS* */
      CDKDIALOG *widget     = 0;
      int buttonCount       = 1;
      int lines             = 0;
      const char *buttons[] =
      {
	 "OK"
      };
      const char *mesg[5];

      mesg[lines++] = "<C><#HL(30)>";
      mesg[lines++] = "<C>I told you </B>NOT<!B> to type G";
      mesg[lines++] = "<C><#HL(30)>";

      widget = newCDKDialog (ScreenOf (entry), CENTER, CENTER,
			     (CDK_CSTRING2)mesg, lines,
			     (CDK_CSTRING2)buttons, buttonCount,
			     A_REVERSE, FALSE, TRUE, FALSE);
      activateCDKDialog (widget, 0);
      destroyCDKDialog (widget);
      drawCDKEntry (entry, ObjOf (entry)->box);
      return 0;
   }
   return 1;
}
