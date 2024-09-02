/* $Id: menu_ex.c,v 1.13 2016/12/04 15:22:16 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "menu_ex";
#endif

static int displayCallback (EObjectType cdktype, void *object,
			    void *clientData,
			    chtype input);
static const char *menulist[MAX_MENU_ITEMS][MAX_SUB_ITEMS];
static const char *menuInfo[3][4] =
{
   {
      "",
      "This saves the current info.",
      "This exits the program.",
      ""
   },
   {
      "",
      "This cuts text",
      "This copies text",
      "This pastes text"
   },
   {
      "",
      "Help for editing",
      "Help for file management",
      "Info about the program"}
};

/*
 * This program demonstratres the Cdk menu widget.
 */
int main (void)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;
   CDKLABEL *infoBox    = 0;
   CDKMENU *menu        = 0;
   int submenusize[3], menuloc[4];
   const char *mesg[5];
   char temp[256];
   int selection;

   cdkscreen = initCDKScreen (NULL);

   /* Start CDK color. */
   initCDKColor ();

   /* Set up the menu. */
   menulist[0][0] = "</B>File<!B>";
   menulist[0][1] = "</B>Save<!B>";
   menulist[0][2] = "</B>Exit<!B>";

   menulist[1][0] = "</B>Edit<!B>";
   menulist[1][1] = "</B>Cut<!B> ";
   menulist[1][2] = "</B>Copy<!B>";
   menulist[1][3] = "</B>Paste<!B>";

   menulist[2][0] = "</B>Help<!B>";
   menulist[2][1] = "</B>On Edit <!B>";
   menulist[2][2] = "</B>On File <!B>";
   menulist[2][3] = "</B>About...<!B>";

   submenusize[0] = 3;
   submenusize[1] = 4;
   submenusize[2] = 4;

   menuloc[0] = LEFT;
   menuloc[1] = LEFT;
   menuloc[2] = RIGHT;

   /* Create the label window. */
   mesg[0] = "                                          ";
   mesg[1] = "                                          ";
   mesg[2] = "                                          ";
   mesg[3] = "                                          ";
   infoBox = newCDKLabel (cdkscreen, CENTER, CENTER,
			  (CDK_CSTRING2) mesg, 4,
			  TRUE, TRUE);

   /* Create the menu. */
   menu = newCDKMenu (cdkscreen, menulist, 3, submenusize, menuloc,
		      TOP, A_UNDERLINE, A_REVERSE);

   /* Create the post process function. */
   setCDKMenuPostProcess (menu, displayCallback, infoBox);

   /* Draw the CDK screen. */
   refreshCDKScreen (cdkscreen);

   /* Activate the menu. */
   selection = activateCDKMenu (menu, 0);

   /* Determine how the user exited from the widget. */
   if (menu->exitType == vEARLY_EXIT)
   {
      mesg[0] = "<C>You hit escape. No menu item was selected.";
      mesg[1] = "",
	 mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }
   else if (menu->exitType == vNORMAL)
   {
      sprintf (temp, "<C>You selected menu #%d, submenu #%d",
	       selection / 100,
	       selection % 100);
      mesg[0] = temp;
      mesg[1] = "",
	 mesg[2] = "<C>Press any key to continue.";
      popupLabel (cdkscreen, (CDK_CSTRING2) mesg, 3);
   }

   /* Clean up. */
   destroyCDKMenu (menu);
   destroyCDKLabel (infoBox);
   destroyCDKScreen (cdkscreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

/*
 * This gets called after every movement.
 */
static int displayCallback (EObjectType cdktype GCC_UNUSED, void *object,
			    void *clientData,
			    chtype input GCC_UNUSED)
{
   /* *INDENT-EQLS* */
   CDKMENU *menu        = (CDKMENU *)object;
   CDKLABEL *infoBox    = (CDKLABEL *)clientData;
   char *mesg[10];
   char temp[256];

   /* Recreate the label message. */
   sprintf (temp, "Title: %.*s",
	    (int)(sizeof (temp) - 20),
	    menulist[menu->currentTitle][0]);
   mesg[0] = strdup (temp);
   sprintf (temp, "Sub-Title: %.*s",
	    (int)(sizeof (temp) - 20),
	    menulist[menu->currentTitle][menu->currentSubtitle + 1]);
   mesg[1] = strdup (temp);
   mesg[2] = strdup ("");
   sprintf (temp, "<C>%.*s",
	    (int)(sizeof (temp) - 20),
	    menuInfo[menu->currentTitle][menu->currentSubtitle + 1]);
   mesg[3] = strdup (temp);

   /* Set the message of the label. */
   setCDKLabel (infoBox, (CDK_CSTRING2) mesg, 4, TRUE);
   drawCDKLabel (infoBox, TRUE);

   freeCharList (mesg, 4);
   return 0;
}
