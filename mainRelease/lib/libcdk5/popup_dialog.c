#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2012/03/21 08:45:41 $
 * $Revision: 1.3 $
 */

/*
 * This pops up a dialog box.
 */
int popupDialog (CDKSCREEN *screen, CDK_CSTRING2 mesg, int mesgCount, CDK_CSTRING2 buttons, int buttonCount)
{
   /* Declare local variables. */
   CDKDIALOG *popup	= 0;
   int choice;

   /* Create the dialog box. */
   popup = newCDKDialog (screen, CENTER, CENTER,
			 mesg, mesgCount,
			 buttons, buttonCount,
			 A_REVERSE, TRUE,
			 TRUE, FALSE);

   /* Activate the dialog box */
   drawCDKDialog (popup, TRUE);

   /* Get the choice. */
   choice = activateCDKDialog (popup, 0);

   /* Destroy the dialog box. */
   destroyCDKDialog (popup);

   /* Clean the screen. */
   eraseCDKScreen (screen);
   refreshCDKScreen (screen);

   /* Return the choice. */
   return choice;
}
