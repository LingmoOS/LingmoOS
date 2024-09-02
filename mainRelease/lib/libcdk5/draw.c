#include <cdk_int.h>

/*
 * $Author: aleahmad $
 * $Date: 2019/02/15 23:58:59 $
 * $Revision: 1.63 $
 */

/*
 * This sets up a basic set of color pairs. These can be redefined
 * if wanted...
 */
void initCDKColor (void)
{
#ifdef HAVE_START_COLOR
   if (has_colors ())
   {
      int color[] =
      {
	 COLOR_WHITE, COLOR_RED, COLOR_GREEN,
	 COLOR_YELLOW, COLOR_BLUE, COLOR_MAGENTA,
	 COLOR_CYAN, COLOR_BLACK
      };
      int pair = 1;
      int fg, bg;
      int limit;

      start_color ();

      limit = (COLORS < MAX_COLORS) ? COLORS : MAX_COLORS;

      /* Create the color pairs. */
      for (fg = 0; fg < limit; fg++)
      {
	 for (bg = 0; bg < limit; bg++)
	 {
	    init_pair ((short)pair++, (short)color[fg], (short)color[bg]);
	 }
      }
   }
#endif
}

/*
 * This prints out a box around a window with attributes
 */
void boxWindow (WINDOW *window, chtype attr)
{
   /* *INDENT-EQLS* */
   int tlx      = 0;
   int tly      = 0;
   int brx      = getmaxx (window) - 1;
   int bry      = getmaxy (window) - 1;

   /* Draw horizontal lines. */
   (void)mvwhline (window, tly, 0, ACS_HLINE | attr, getmaxx (window));
   (void)mvwhline (window, bry, 0, ACS_HLINE | attr, getmaxx (window));

   /* Draw vertical lines. */
   (void)mvwvline (window, 0, tlx, ACS_VLINE | attr, getmaxy (window));
   (void)mvwvline (window, 0, brx, ACS_VLINE | attr, getmaxy (window));

   /* Draw in the corners. */
   (void)mvwaddch (window, tly, tlx, ACS_ULCORNER | attr);
   (void)mvwaddch (window, tly, brx, ACS_URCORNER | attr);
   (void)mvwaddch (window, bry, tlx, ACS_LLCORNER | attr);
   (void)mvwaddch (window, bry, brx, ACS_LRCORNER | attr);
   (void)wrefresh (window);
}

/*
 * This draws a box with attributes and lets the user define
 * each element of the box.
 */
void attrbox (WINDOW *win,
	      chtype tlc,
	      chtype trc,
	      chtype blc,
	      chtype brc,
	      chtype horz,
	      chtype vert,
	      chtype attr)
{
   /* *INDENT-EQLS* */
   int x1       = 0;
   int y1       = 0;
   int y2       = getmaxy (win) - 1;
   int x2       = getmaxx (win) - 1;
   int count    = 0;

   /* Draw horizontal lines. */
   if (horz != 0)
   {
      (void)mvwhline (win, y1, 0, horz | attr, getmaxx (win));
      (void)mvwhline (win, y2, 0, horz | attr, getmaxx (win));
      count++;
   }

   /* Draw vertical lines. */
   if (vert != 0)
   {
      (void)mvwvline (win, 0, x1, vert | attr, getmaxy (win));
      (void)mvwvline (win, 0, x2, vert | attr, getmaxy (win));
      count++;
   }

   /* Draw in the corners. */
   if (tlc != 0)
   {
      (void)mvwaddch (win, y1, x1, tlc | attr);
      count++;
   }
   if (trc != 0)
   {
      (void)mvwaddch (win, y1, x2, trc | attr);
      count++;
   }
   if (blc != 0)
   {
      (void)mvwaddch (win, y2, x1, blc | attr);
      count++;
   }
   if (brc != 0)
   {
      (void)mvwaddch (win, y2, x2, brc | attr);
      count++;
   }
   if (count != 0)
   {
      wrefresh (win);
   }
}

/*
 * Draw a box around the given window using the object's defined line-drawing
 * characters.
 */
void drawObjBox (WINDOW *win, CDKOBJS *object)
{
   attrbox (win,
	    object->ULChar,
	    object->URChar,
	    object->LLChar,
	    object->LRChar,
	    object->HZChar,
	    object->VTChar,
	    object->BXAttr);
}

/*
 * This draws a line on the given window. (odd angle lines not working yet)
 */
void drawLine (WINDOW *window, int startx, int starty, int endx, int endy, chtype line)
{
   /* *INDENT-EQLS* */
   int xdiff    = endx - startx;
   int ydiff    = endy - starty;
   int x        = 0;
   int y        = 0;

   /* Determine if we are drawing a horizontal or vertical line. */
   if (ydiff == 0)
   {
      if (xdiff > 0)
	 (void)mvwhline (window, starty, startx, line, xdiff);
   }
   else if (xdiff == 0)
   {
      if (ydiff > 0)
	 (void)mvwvline (window, starty, startx, line, ydiff);
   }
   else
   {
      /* We need to determine the angle of the line. */
      /* *INDENT-EQLS* */
      int height        = xdiff;
      int width         = ydiff;
      int xratio        = (height > width ? 1 : (width / height));
      int yratio        = (width > height ? (width / height) : 1);
      int xadj          = 0;
      int yadj          = 0;

      /* Set the vars. */
      x = startx;
      y = starty;
      while (x != endx && y != endy)
      {
	 /* Add the char to the window. */
	 (void)mvwaddch (window, y, x, line);

	 /* Make the x and y adjustments. */
	 if (xadj != xratio)
	 {
	    x = (xdiff < 0 ? x - 1 : x + 1);
	    xadj++;
	 }
	 else
	 {
	    xadj = 0;
	 }
	 if (yadj != yratio)
	 {
	    y = (ydiff < 0 ? y - 1 : y + 1);
	    yadj++;
	 }
	 else
	 {
	    yadj = 0;
	 }
      }
   }
}

/*
 * This draws a shadow around a window.
 */
void drawShadow (WINDOW *shadowWin)
{
   if (shadowWin != 0)
   {
      int x_hi = getmaxx (shadowWin) - 1;
      int y_hi = getmaxy (shadowWin) - 1;

      /* Draw the line on the bottom. */
      (void)mvwhline (shadowWin, y_hi, 1, ACS_HLINE | A_DIM, x_hi);

      /* Draw the line on the right. */
      (void)mvwvline (shadowWin, 0, x_hi, ACS_VLINE | A_DIM, y_hi);

      (void)mvwaddch (shadowWin, 0, x_hi, ACS_URCORNER | A_DIM);
      (void)mvwaddch (shadowWin, y_hi, 0, ACS_LLCORNER | A_DIM);
      (void)mvwaddch (shadowWin, y_hi, x_hi, ACS_LRCORNER | A_DIM);
      wrefresh (shadowWin);
   }
}

/*
 * Write a string of blanks, using writeChar().
 */
void writeBlanks (WINDOW *window, int xpos, int ypos, int align, int start, int end)
{
   if (start < end)
   {
      unsigned want = (unsigned)(end - start) + 1000;
      char *blanks = (char *)malloc (want);

      if (blanks != 0)
      {
	 cleanChar (blanks, (int)(want - 1), ' ');
	 writeChar (window, xpos, ypos, blanks, align, start, end);
	 freeChar (blanks);
      }
   }
}

static void CDKmvwaddch (WINDOW *window,
			 int ypos,
			 int xpos,
			 chtype ch,
			 chtype attr)
{
   chtype color = (attr | ch) & (A_COLOR);
   (void)mvwaddch (window, ypos, xpos, ((attr | ch) & (~A_COLOR)) | color);
}
/*
 * This writes out a char * string with no attributes.
 */
void writeChar (WINDOW *window,
		int xpos,
		int ypos,
		char *string,
		int align,
		int start,
		int end)
{
   writeCharAttrib (window, xpos, ypos, string, A_NORMAL, align, start, end);
}

/*
 * This writes out a char * string with attributes.
 */
void writeCharAttrib (WINDOW *window,
		      int xpos,
		      int ypos,
		      char *string,
		      chtype attr,
		      int align,
		      int start,
		      int end)
{
   int display = end - start;
   int x;

   if (align == HORIZONTAL)
   {
      /* Draw the message on a horizontal axis. */
      display = MINIMUM (display, getmaxx (window) - 1);
      for (x = 0; x < display; x++)
      {
	 CDKmvwaddch (window,
		      ypos,
		      xpos + x,
		      CharOf (string[x + start]),
		      attr);
      }
   }
   else
   {
      /* Draw the message on a vertical axis. */
      display = MINIMUM (display, getmaxy (window) - 1);
      for (x = 0; x < display; x++)
      {
	 CDKmvwaddch (window,
		      ypos + x,
		      xpos,
		      CharOf (string[x + start]),
		      attr);
      }
   }
}

/*
 * This writes out a chtype * string.
 */
void writeChtype (WINDOW *window,
		  int xpos,
		  int ypos,
		  chtype *string,
		  int align,
		  int start,
		  int end)
{
   writeChtypeAttrib (window, xpos, ypos, string, A_NORMAL, align, start, end);
}

/*
 * This writes out a chtype * string * with the given attributes added.
 */
void writeChtypeAttrib (WINDOW *window,
			int xpos,
			int ypos,
			chtype *string,
			chtype attr,
			int align,
			int start,
			int end)
{
   /* *INDENT-EQLS* */
   int diff             = end - start;
   int display          = 0;
   int x                = 0;

   if (align == HORIZONTAL)
   {
      /* Draw the message on a horizontal axis. */
      display = MINIMUM (diff, getmaxx (window) - xpos);
      for (x = 0; x < display; x++)
      {
	 CDKmvwaddch (window, ypos, xpos + x, string[x + start], attr);
      }
   }
   else
   {
      /* Draw the message on a vertical axis. */
      display = MINIMUM (diff, getmaxy (window) - ypos);
      for (x = 0; x < display; x++)
      {
	 CDKmvwaddch (window, ypos + x, xpos, string[x + start], attr);
      }
   }
}
