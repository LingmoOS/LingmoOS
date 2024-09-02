/* $Id: dialog_ex.c,v 1.14 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "dialog_ex";
#endif

/*
 * This program demonstrates the Cdk dialog widget.
 */
int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;
   CDKDIALOG *question  = 0;
   const char *buttons[] =
   {"</B/24>Ok", "</B16>Cancel"};
   const char *message[10];
   const char *mesg[3];
   char temp[100];
   int selection;

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, CDK_MIN_PARAMS);

   cdkscreen = initCDKScreen (NULL);

   /* Start color. */
   initCDKColor ();

   /* Create the message within the dialog box. */
   message[0] = "<C></U>Dialog Widget Demo";
   message[1] = " ";
   message[2] = "<C>The dialog widget allows the programmer to create";
   message[3] = "<C>a popup dialog box with buttons. The dialog box";
   message[4] = "<C>can contain </B/32>colours<!B!32>, </R>character attributes<!R>";
   message[5] = "<R>and even be right justified.";
   message[6] = "<L>and left.";

   /* Create the dialog box. */
   question = newCDKDialog (cdkscreen,
			    CDKparamValue (&params, 'X', CENTER),
			    CDKparamValue (&params, 'Y', CENTER),
			    (CDK_CSTRING2) message, 7,
			    (CDK_CSTRING2) buttons, 2,
			    COLOR_PAIR (2) | A_REVERSE,
			    TRUE,
			    CDKparamValue (&params, 'N', TRUE),
			    CDKparamValue (&params, 'S', FALSE));

   /* Check if we got a null value back. */
   if (question == 0)
   {
      /* Shut down Cdk. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot create the dialog box. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Activate the dialog box. */
   selection = activateCDKDialog (question, 0);

   /* Tell them what was selected. */
   if (question->exitType == vESCAPE_HIT)
   {
      mesg[0] = "<C>You hit escape. No button selected.";
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }
   else if (question->exitType == vNORMAL)
   {
      sprintf (temp, "<C>You selected button #%d", selection);
      mesg[0] = temp;
      mesg[1] = "";
      mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }

   /* Clean up. */
   destroyCDKDialog (question);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}
