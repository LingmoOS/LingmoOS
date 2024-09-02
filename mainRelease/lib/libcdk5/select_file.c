#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2012/03/22 00:38:13 $
 * $Revision: 1.2 $
 */

/*
 * This allows a person to select a file.
 */
char *selectFile (CDKSCREEN *screen, const char *title)
{
   /* *INDENT-EQLS* */
   CDKFSELECT *fselect  = 0;
   char *filename       = 0;
   char *holder         = 0;

   /* Create the file selector. */
   fselect = newCDKFselect (screen, CENTER, CENTER, -4, -20,
			    title, "File: ",
			    A_NORMAL, '_', A_REVERSE,
			    "</5>", "</48>", "</N>", "</N>",
			    TRUE, FALSE);

   /* Let the user play. */
   holder = activateCDKFselect (fselect, 0);

   /* Check the way the user exited the selector. */
   if (fselect->exitType != vNORMAL)
   {
      destroyCDKFselect (fselect);
      refreshCDKScreen (screen);
      return (0);
   }

   /* Otherwise... */
   filename = copyChar (holder);
   destroyCDKFselect (fselect);
   refreshCDKScreen (screen);
   return (filename);
}
