#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2012/03/22 00:43:33 $
 * $Revision: 1.3 $
 */

/*
 * This returns a selected value in a list.
 */
int getListIndex (CDKSCREEN *screen,
		  const char *title,
		  CDK_CSTRING2 list,
		  int listSize,
		  boolean numbers)
{
   /* *INDENT-EQLS* */
   CDKSCROLL *scrollp   = 0;
   int selected         = -1;
   int height           = 10;
   int width            = -1;
   int len              = 0;
   int x;

   /* Determine the height of the list. */
   if (listSize < 10)
   {
      height = listSize + (title == 0 ? 2 : 3);
   }

   /* Determine the width of the list. */
   for (x = 0; x < listSize; x++)
   {
      int temp = (int)strlen (list[x]) + 10;
      width = MAXIMUM (width, temp);
   }
   if (title != 0)
   {
      len = (int)strlen (title);
   }
   width = MAXIMUM (width, len);
   width += 5;

   /* Create the scrolling list. */
   scrollp = newCDKScroll (screen, CENTER, CENTER, RIGHT,
			   height, width, title,
			   (CDK_CSTRING2) list, listSize, numbers,
			   A_REVERSE, TRUE, FALSE);

   /* Check if we made the list. */
   if (scrollp == 0)
   {
      refreshCDKScreen (screen);
      return -1;
   }

   /* Let the user play. */
   selected = activateCDKScroll (scrollp, 0);

   /* Check how they exited. */
   if (scrollp->exitType != vNORMAL)
   {
      selected = -1;
   }

   /* Clean up. */
   destroyCDKScroll (scrollp);
   refreshCDKScreen (screen);
   return selected;
}
