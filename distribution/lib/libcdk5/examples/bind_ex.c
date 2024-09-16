/* $Id: bind_ex.c,v 1.20 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "bind_ex";
#endif

static int dialogHelpCB (EObjectType cdktype GCC_UNUSED,
			 void *object,
			 void *clientData GCC_UNUSED,
			 chtype key GCC_UNUSED)
{
   CDKDIALOG *dialog = (CDKDIALOG *)object;
   const char *mesg[5];

   /* Check which button we are on. */
   if (dialog->currentButton == 0)
   {
      mesg[0] = "<C></U>Help for </U>Who<!U>.";
      mesg[1] = "<C>When this button is picked the name of the current";
      mesg[2] = "<C>user is displayed on the screen in a popup window.";
      popupLabel (ScreenOf (dialog), (CDK_CSTRING2) mesg, 3);
   }
   else if (dialog->currentButton == 1)
   {
      mesg[0] = "<C></U>Help for </U>Time<!U>.";
      mesg[1] = "<C>When this button is picked the current time is";
      mesg[2] = "<C>displayed on the screen in a popup window.";
      popupLabel (ScreenOf (dialog), (CDK_CSTRING2) mesg, 3);
   }
   else if (dialog->currentButton == 2)
   {
      mesg[0] = "<C></U>Help for </U>Date<!U>.";
      mesg[1] = "<C>When this button is picked the current date is";
      mesg[2] = "<C>displayed on the screen in a popup window.";
      popupLabel (ScreenOf (dialog), (CDK_CSTRING2) mesg, 3);
   }
   else if (dialog->currentButton == 3)
   {
      mesg[0] = "<C></U>Help for </U>Quit<!U>.";
      mesg[1] = "<C>When this button is picked the dialog box is exited.";
      popupLabel (ScreenOf (dialog), (CDK_CSTRING2) mesg, 2);
   }
   return (FALSE);
}

int main (int argc, char **argv)
{
   /* *INDENT-OFF* */
   CDKSCREEN	*cdkscreen;
   CDKDIALOG	*question;
   const char	*buttons[40];
   const char	*message[40];
   const char	*info[5];
   char		*loginName = 0;
   char		temp[256];
   int		selection;
   time_t	clck;
   struct tm	*currentTime;
   CDK_PARAMS   params;
   /* *INDENT-ON* */

   CDKparseParams (argc, argv, &params, CDK_MIN_PARAMS);

   cdkscreen = initCDKScreen (NULL);

   /* Start color. */
   initCDKColor ();

   /* Set up the dialog box. */
   message[0] = "<C></U>Simple Command Interface";
   message[1] = "Pick the command you wish to run.";
   message[2] = "<C>Press </R>?<!R> for help.";
   buttons[0] = "Who";
   buttons[1] = "Time";
   buttons[2] = "Date";
   buttons[3] = "Quit";

   /* Create the dialog box. */
   question = newCDKDialog (cdkscreen,
			    CDKparamValue (&params, 'X', CENTER),
			    CDKparamValue (&params, 'Y', CENTER),
			    (CDK_CSTRING2) message, 3,
			    (CDK_CSTRING2) buttons, 4,
			    A_REVERSE,
			    TRUE,
			    CDKparamValue (&params, 'N', TRUE),
			    CDKparamValue (&params, 'S', FALSE));

   /* Check if we got a null value back. */
   if (question == (CDKDIALOG *)0)
   {
      destroyCDKScreen (cdkscreen);

      /* End curses... */
      endCDK ();

      printf ("Cannot create the dialog box. ");
      printf ("Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Create the key binding. */
   bindCDKObject (vDIALOG, question, '?', dialogHelpCB, 0);

   /* Activate the dialog box. */
   selection = 0;
   while (selection != 3)
   {
      /* Get the users button selection. */
      selection = activateCDKDialog (question, (chtype *)0);

      /* Check the results. */
      if (selection == 0)
      {
	 /* Get the users login name. */
	 info[0] = "<C>     </U>Login Name<!U>     ";
#if defined (HAVE_GETLOGIN)
	 loginName = getlogin ();
#endif
	 if (loginName == (char *)0)
	 {
	    strcpy (temp, "<C></R>Unknown");
	 }
	 else
	 {
	    sprintf (temp, "<C><%.*s>", (int)(sizeof (temp) - 10), loginName);
	 }
	 info[1] = temp;
	 popupLabel (ScreenOf (question), (CDK_CSTRING2) info, 2);
      }
      else if (selection == 1)
      {
	 time (&clck);
	 currentTime = localtime (&clck);
	 sprintf (temp, "<C>%2d:%02d:%02d",
		  currentTime->tm_hour,
		  currentTime->tm_min,
		  currentTime->tm_sec);
	 info[0] = "<C>   </U>Current Time<!U>   ";
	 info[1] = temp;
	 popupLabel (ScreenOf (question), (CDK_CSTRING2) info, 2);
      }
      else if (selection == 2)
      {
	 time (&clck);
	 currentTime = localtime (&clck);
	 sprintf (temp, "<C>%d/%02d/%02d",
		  currentTime->tm_mday,
		  currentTime->tm_mon + 1,
		  currentTime->tm_year % 100);
	 info[0] = "<C>   </U>Current Date<!U>   ";
	 info[1] = temp;
	 popupLabel (ScreenOf (question), (CDK_CSTRING2) info, 2);
      }
   }

   /* Clean up. */
   destroyCDKDialog (question);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}
