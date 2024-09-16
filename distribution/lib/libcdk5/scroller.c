#include <cdk.h>
#include <cdk_int.h>
#include <scroller.h>

/*
 * $Author: aleahmad $
 * $Date: 2016/11/20 13:57:20 $
 * $Revision: 1.3 $
 */

void scroller_KEY_UP (CDKSCROLLER *scroller)
{
   if (scroller->listSize <= 0 || scroller->currentItem <= 0)
   {
      Beep ();
      return;
   }

   scroller->currentItem--;

   if (scroller->currentHigh)
   {
      scroller->currentHigh--;
   }

   if (scroller->currentTop && scroller->currentItem < scroller->currentTop)
   {
      scroller->currentTop--;
   }
}

void scroller_KEY_DOWN (CDKSCROLLER *scroller)
{
   if (scroller->listSize <= 0 || scroller->currentItem >= scroller->lastItem)
   {
      Beep ();
      return;
   }

   scroller->currentItem++;

   if (scroller->currentHigh < scroller->viewSize - 1)
   {
      scroller->currentHigh++;
   }

   if (scroller->currentTop < scroller->maxTopItem
       && scroller->currentItem > (scroller->currentTop + scroller->viewSize
				   - 1))
   {
      scroller->currentTop++;
   }
}

void scroller_KEY_LEFT (CDKSCROLLER *scroller)
{
   if (scroller->listSize <= 0 || scroller->leftChar <= 0)
   {
      Beep ();
      return;
   }

   scroller->leftChar--;
}

void scroller_KEY_RIGHT (CDKSCROLLER *scroller)
{
   if (scroller->listSize <= 0 || scroller->leftChar >= scroller->maxLeftChar)
   {
      Beep ();
      return;
   }

   scroller->leftChar++;
}

void scroller_KEY_PPAGE (CDKSCROLLER *scroller)
{
   int viewSize = scroller->viewSize - 1;

   if (scroller->listSize <= 0 || scroller->currentTop <= 0)
   {
      Beep ();
      return;
   }

   if (scroller->currentTop < viewSize)
   {
      scroller_KEY_HOME (scroller);
   }
   else
   {
      scroller->currentTop -= viewSize;
      scroller->currentItem -= viewSize;
   }
}

void scroller_KEY_NPAGE (CDKSCROLLER *scroller)
{
   int viewSize = scroller->viewSize - 1;

   if (scroller->listSize <= 0 || scroller->currentTop >= scroller->maxTopItem)
   {
      Beep ();
      return;
   }

   if ((scroller->currentTop + viewSize) <= scroller->maxTopItem)
   {
      scroller->currentTop += viewSize;
      scroller->currentItem += viewSize;
   }
   else
   {
      scroller_KEY_END (scroller);
   }
}

void scroller_KEY_HOME (CDKSCROLLER *scroller)
{
   scroller->currentTop = 0;
   scroller->currentItem = 0;
   scroller->currentHigh = 0;
}

void scroller_KEY_END (CDKSCROLLER *scroller)
{
   scroller->currentTop = scroller->maxTopItem;
   scroller->currentItem = scroller->lastItem;
   scroller->currentHigh = scroller->viewSize - 1;
}

void scroller_FixCursorPosition (CDKSCROLLER *scroller)
{
   int scrollbarAdj = (scroller->scrollbarPlacement == LEFT) ? 1 : 0;
   int ypos = SCREEN_YPOS (scroller, scroller->currentItem - scroller->currentTop);
   int xpos = SCREEN_XPOS (scroller, 0) + scrollbarAdj;

   wmove (InputWindowOf (scroller), ypos, xpos);
   wrefresh (InputWindowOf (scroller));
}

void scroller_SetPosition (CDKSCROLLER *scroller, int item)
{
   /* item out of band */
   if (item <= 0)
   {
      scroller_KEY_HOME (scroller);
      return;
   }

   /* item out of band */
   if (item >= scroller->lastItem)
   {
      scroller_KEY_END (scroller);
      return;
   }

   /* item in first view port */
   if (item < scroller->viewSize)
   {
      scroller->currentTop = 0;
   }
   /* item in last view port */
   else if (item >= scroller->lastItem - scroller->viewSize)
   {
      scroller->currentTop = scroller->maxTopItem;
   }
   /* item not in visible view port */
   else if (item < scroller->currentTop || item >= scroller->currentTop + scroller->viewSize)
   {
      scroller->currentTop = item;
   }

   scroller->currentItem = item;
   scroller->currentHigh = scroller->currentItem - scroller->currentTop;
}

int scroller_MaxViewSize (CDKSCROLLER *scroller)
{
   return (scroller->boxHeight - (2 * BorderOf (scroller) + TitleLinesOf (scroller)));
}

void scroller_SetViewSize (CDKSCROLLER *scroller, int size)
{
   int max_view_size = scroller_MaxViewSize (scroller);

   scroller->viewSize = max_view_size;
   scroller->listSize = size;
   scroller->lastItem = size - 1;
   scroller->maxTopItem = size - scroller->viewSize;

   if (size < scroller->viewSize)
   {
      scroller->viewSize = size;
      scroller->maxTopItem = 0;
   }

   if (scroller->listSize > 0 && max_view_size > 0)
   {
      scroller->step = (float)(max_view_size / (double)scroller->listSize);
      scroller->toggleSize = ((scroller->listSize > max_view_size)
			      ? 1
			      : ceilCDK (scroller->step));
   }
   else
   {
      scroller->step = 1;
      scroller->toggleSize = 1;
   }
}
