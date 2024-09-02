#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2021/12/16 01:10:45 $
 * $Revision: 1.226 $
 */

#define L_MARKER '<'
#define R_MARKER '>'

char *GPasteBuffer = 0;

/*
 * This beeps then flushes the stdout stream.
 */
void Beep (void)
{
   beep ();
   fflush (stdout);
}

/*
 * This sets a string to the given character.
 */
void cleanChar (char *s, int len, char character)
{
   if (s != 0)
   {
      int x;
      for (x = 0; x < len; x++)
      {
	 s[x] = character;
      }
      s[--x] = '\0';
   }
}

void cleanChtype (chtype *s, int len, chtype character)
{
   if (s != 0)
   {
      int x;
      for (x = 0; x < len; x++)
      {
	 s[x] = character;
      }
      s[--x] = '\0';
   }
}

/*
 * This takes an x and y position and realigns the values iff they sent in
 * values like CENTER, LEFT, RIGHT, ...
 */
void alignxy (WINDOW *window, int *xpos, int *ypos, int boxWidth, int boxHeight)
{
   int first, gap, limit, last;

   first = getbegx (window);
   limit = getmaxx (window);
   if ((gap = (limit - boxWidth)) < 0)
      gap = 0;
   last = first + gap;

   switch (*xpos)
   {
   case LEFT:
      (*xpos) = first;
      break;
   case RIGHT:
      (*xpos) = first + gap;
      break;
   case CENTER:
      (*xpos) = first + (gap / 2);
      break;
   default:
      if ((*xpos) > last)
	 (*xpos) = last;
      else if ((*xpos) < first)
	 (*xpos) = first;
      break;
   }
   (void)last;

   first = getbegy (window);
   limit = getmaxy (window);
   if ((gap = (limit - boxHeight)) < 0)
      gap = 0;
   last = first + gap;

   switch (*ypos)
   {
   case TOP:
      (*ypos) = first;
      break;
   case BOTTOM:
      (*ypos) = first + gap;
      break;
   case CENTER:
      (*ypos) = first + (gap / 2);
      break;
   default:
      if ((*ypos) > last)
      {
	 (*ypos) = last;
      }
      else if ((*ypos) < first)
      {
	 (*ypos) = first;
      }
      break;
   }
}

/*
 * This takes a string, a field width and a justification type
 * and returns the adjustment to make, to fill
 * the justification requirement.
 */
int justifyString (int boxWidth, int mesgLength, int justify)
{
   /*
    * Make sure the message isn't longer than the width.
    * If it is, return 0.
    */
   if (mesgLength >= boxWidth)
      return (0);

   /* Try to justify the message.  */
   if (justify == LEFT)
      return (0);

   if (justify == RIGHT)
      return boxWidth - mesgLength;

   if (justify == CENTER)
      return ((int)((boxWidth - mesgLength) / 2));

   return (justify);
}

/*
 * This frees a string if it is not null. This is a safety
 * measure. Some compilers let you free a null string. I
 * don't like that idea.
 */
void freeChar (char *string)
{
   freeChecked (string);
}

void freeChtype (chtype *string)
{
   freeChecked (string);
}

/*
 * Corresponding list freeing (does not free the list pointer).
 */
void freeCharList (char **list, unsigned size)
{
   if (list != 0)
   {
      while (size-- != 0)
      {
	 freeChar (list[size]);
	 list[size] = 0;
      }
   }
}

void freeChtypeList (chtype **list, unsigned size)
{
   if (list != 0)
   {
      while (size-- != 0)
      {
	 freeChtype (list[size]);
	 list[size] = 0;
      }
   }
}

/*
 * This performs a safe copy of a string. This means it adds the null
 * terminator on the end of the string, like strdup().
 */
char *copyChar (const char *original)
{
   char *newstring = 0;

   if (original != 0)
   {
      if ((newstring = typeMallocN (char, strlen (original) + 1)) != 0)
	   strcpy (newstring, original);
   }
   return (newstring);
}

chtype *copyChtype (const chtype *original)
{
   chtype *newstring = 0;

   if (original != 0)
   {
      int len = chlen (original);

      if ((newstring = typeMallocN (chtype, len + 4)) != 0)
      {
	 int x;

	 for (x = 0; x < len; x++)
	 {
	    newstring[x] = original[x];
	 }
	 newstring[len] = '\0';
	 newstring[len + 1] = '\0';
      }
   }
   return (newstring);
}

/*
 * Copy the given lists.
 */
char **copyCharList (const char **list)
{
   size_t size = (size_t)lenCharList (list) + 1;
   char **result = typeMallocN (char *, size);

   if (result != 0)
   {
      unsigned n;
      for (n = 0; n < size; ++n)
	 result[n] = copyChar (list[n]);
   }
   return result;
}

chtype **copyChtypeList (const chtype **list)
{
   size_t size = (size_t)lenChtypeList (list) + 1;
   chtype **result = typeMallocN (chtype *, size);

   if (result != 0)
   {
      unsigned n;
      for (n = 0; n < size; ++n)
	 result[n] = copyChtype (list[n]);
   }
   return result;
}

/*
 * Return the length of the given lists.
 */
int lenCharList (const char **list)
{
   int result = 0;
   if (list != 0)
   {
      while (*list++ != 0)
	 ++result;
   }
   return result;
}

int lenChtypeList (const chtype **list)
{
   int result = 0;
   if (list != 0)
   {
      while (*list++ != 0)
	 ++result;
   }
   return result;
}

/*
 * This reads a file and sticks it into the char *** provided.
 */
int CDKreadFile (const char *filename, char ***array)
{
   FILE *fd;
   char temp[BUFSIZ];
   unsigned lines = 0;
   unsigned used = 0;

   /* Can we open the file?  */
   if ((fd = fopen (filename, "r")) == 0)
   {
      return (-1);
   }

   while ((fgets (temp, sizeof (temp), fd) != 0))
   {
      size_t len = strlen (temp);
      if (len != 0 && temp[len - 1] == '\n')
	 temp[--len] = '\0';
      used = CDKallocStrings (array, temp, lines++, used);
   }
   fclose (fd);

   return (int)(lines);
}

#define DigitOf(c) ((c)-'0')

static int encodeAttribute (const char *string, int from, chtype *mask)
{
   *mask = 0;
   switch (string[from + 1])
   {
   case 'B':
      *mask = A_BOLD;
      break;
   case 'D':
      *mask = A_DIM;
      break;
   case 'K':
      *mask = A_BLINK;
      break;
   case 'R':
      *mask = A_REVERSE;
      break;
   case 'S':
      *mask = A_STANDOUT;
      break;
   case 'U':
      *mask = A_UNDERLINE;
      break;
   }

   if (*mask != 0)
   {
      from++;
   }
   else
   {
      int digits;
      int pair = 0;

      for (digits = 1; digits <= 3; ++digits)
      {
	 if (!isdigit (CharOf (string[1 + from])))
	    break;
	 pair *= 10;
	 pair += DigitOf (string[++from]);
      }
#ifdef HAVE_START_COLOR
#define MAX_PAIR (int) (A_COLOR / (((~A_COLOR) << 1) & A_COLOR))
      if (pair > MAX_PAIR)
	 pair = MAX_PAIR;
      *mask = (chtype)COLOR_PAIR (pair);
#else
      *mask = A_BOLD;
#endif
   }
   return from;
}

/*
 * The reverse of encodeAttribute()
 * Well almost.  If attributes such as bold and underline are combined in
 * the same string, we do not necessarily reconstruct them in the same order.
 * Also, alignment markers and tabs are lost.
 */
static unsigned decodeAttribute (char *string,
				 unsigned from,
				 chtype oldattr,
				 chtype newattr)
{
   /* *INDENT-OFF* */
   static const struct {
      int	code;
      chtype	mask;
   } table[] = {
      { 'B', A_BOLD },
      { 'D', A_DIM },
      { 'K', A_BLINK },
      { 'R', A_REVERSE },
      { 'S', A_STANDOUT },
      { 'U', A_UNDERLINE },
   };
   /* *INDENT-ON* */

   char temp[80];
   char *result = (string != 0) ? (string + from) : temp;
   char *base = result;
   chtype tmpattr = oldattr & A_ATTRIBUTES;

   newattr &= A_ATTRIBUTES;
   if (tmpattr != newattr)
   {
      while (tmpattr != newattr)
      {
	 unsigned n;
	 bool found = FALSE;

	 for (n = 0; n < sizeof (table) / sizeof (table[0]); ++n)
	 {
	    if ((table[n].mask & tmpattr) != (table[n].mask & newattr))
	    {
	       found = TRUE;
	       *result++ = L_MARKER;
	       if (table[n].mask & tmpattr)
	       {
		  *result++ = '!';
		  tmpattr &= ~(table[n].mask);
	       }
	       else
	       {
		  *result++ = '/';
		  tmpattr |= (table[n].mask);
	       }
	       *result++ = (char)table[n].code;
	       break;
	    }
	 }
#ifdef HAVE_START_COLOR
	 if ((tmpattr & A_COLOR) != (newattr & A_COLOR))
	 {
	    int oldpair = PAIR_NUMBER (tmpattr);
	    int newpair = PAIR_NUMBER (newattr);
	    if (!found)
	    {
	       found = TRUE;
	       *result++ = L_MARKER;
	    }
	    if (newpair == 0)
	    {
	       *result++ = '!';
	       sprintf (result, "%d", oldpair);
	    }
	    else
	    {
	       *result++ = '/';
	       sprintf (result, "%d", newpair);
	    }
	    result += strlen (result);
	    tmpattr &= ~A_COLOR;
	    newattr &= ~A_COLOR;
	 }
#endif
	 if (found)
	    *result++ = R_MARKER;
	 else
	    break;
      }
   }

   return (from + (unsigned)(result - base));
}

/*
 * This function takes a character string, full of format markers
 * and translates them into a chtype * array. This is better suited
 * to curses, because curses uses chtype almost exclusively
 */
chtype *char2Chtype (const char *string, int *to, int *align)
{
   chtype *result = 0;
   chtype mask;

   (*to) = 0;
   *align = LEFT;

   if (string != 0 && *string != 0)
   {
      int len = (int)strlen (string);
      int pass;
      int used = 0;
      /*
       * We make two passes because we may have indents and tabs to expand, and
       * do not know in advance how large the result will be.
       */
      for (pass = 0; pass < 2; pass++)
      {
	 chtype attrib;
	 int insideMarker;
	 int from;
	 int adjust;
	 int start;
	 int x = 3;

	 if (pass != 0)
	 {
	    if ((result = typeMallocN (chtype, used + 2)) == 0)
	    {
	       used = 0;
	       break;
	    }
	 }
	 adjust = 0;
	 attrib = A_NORMAL;
	 start = 0;
	 used = 0;

	 /* Look for an alignment marker.  */
	 if (*string == L_MARKER)
	 {
	    if (string[1] == 'C' && string[2] == R_MARKER)
	    {
	       (*align) = CENTER;
	       start = 3;
	    }
	    else if (string[1] == 'R' && string[2] == R_MARKER)
	    {
	       (*align) = RIGHT;
	       start = 3;
	    }
	    else if (string[1] == 'L' && string[2] == R_MARKER)
	    {
	       start = 3;
	    }
	    else if (string[1] == 'B' && string[2] == '=')
	    {
	       /* Set the item index value in the string.       */
	       if (result != 0)
	       {
		  result[0] = ' ';
		  result[1] = ' ';
		  result[2] = ' ';
	       }

	       /* Pull out the bullet marker.  */
	       while (string[x] != R_MARKER && string[x] != 0)
	       {
		  if (result != 0)
		     result[x] = (chtype)string[x] | A_BOLD;
		  x++;
	       }
	       adjust = 1;

	       /* Set the alignment variables.  */
	       start = x;
	       used = x;
	    }
	    else if (string[1] == 'I' && string[2] == '=')
	    {
	       from = 2;
	       x = 0;

	       while (string[++from] != R_MARKER && string[from] != 0)
	       {
		  if (isdigit (CharOf (string[from])))
		  {
		     adjust = (adjust * 10) + DigitOf (string[from]);
		     x++;
		  }
	       }

	       start = x + 4;
	    }
	 }

	 while (adjust-- > 0)
	 {
	    if (result != 0)
	       result[used] = ' ';
	    used++;
	 }

	 /* Set the format marker boolean to false.  */
	 insideMarker = FALSE;

	 /* Start parsing the character string.  */
	 for (from = start; from < len; from++)
	 {
	    /* Are we inside a format marker?  */
	    if (!insideMarker)
	    {
	       if (string[from] == L_MARKER
		   && (string[from + 1] == '/'
		       || string[from + 1] == '!'
		       || string[from + 1] == '#'))
	       {
		  insideMarker = TRUE;
	       }
	       else if (string[from] == '\\' && string[from + 1] == L_MARKER)
	       {
		  from++;
		  if (result != 0)
		     result[used] = CharOf (string[from]) | attrib;
		  used++;
		  from++;
	       }
	       else if (string[from] == '\t')
	       {
		  do
		  {
		     if (result != 0)
			result[used] = ' ';
		     used++;
		  }
		  while (used & 7);
	       }
	       else
	       {
		  if (result != 0)
		     result[used] = CharOf (string[from]) | attrib;
		  used++;
	       }
	    }
	    else
	    {
	       switch (string[from])
	       {
	       case R_MARKER:
		  insideMarker = 0;
		  break;
	       case '#':
		  {
		     chtype lastChar = 0;
		     switch (string[from + 2])
		     {
		     case 'L':
			switch (string[from + 1])
			{
			case 'L':
			   lastChar = ACS_LLCORNER;
			   break;
			case 'U':
			   lastChar = ACS_ULCORNER;
			   break;
			case 'H':
			   lastChar = ACS_HLINE;
			   break;
			case 'V':
			   lastChar = ACS_VLINE;
			   break;
			case 'P':
			   lastChar = ACS_PLUS;
			   break;
			}
			break;
		     case 'R':
			switch (string[from + 1])
			{
			case 'L':
			   lastChar = ACS_LRCORNER;
			   break;
			case 'U':
			   lastChar = ACS_URCORNER;
			   break;
			}
			break;
		     case 'T':
			switch (string[from + 1])
			{
			case 'T':
			   lastChar = ACS_TTEE;
			   break;
			case 'R':
			   lastChar = ACS_RTEE;
			   break;
			case 'L':
			   lastChar = ACS_LTEE;
			   break;
			case 'B':
			   lastChar = ACS_BTEE;
			   break;
			}
			break;
		     case 'A':
#if !(defined(__hpux) && !defined(NCURSES_VERSION))
			switch (string[from + 1])
			{
			case 'L':
			   lastChar = ACS_LARROW;
			   break;
			case 'R':
			   lastChar = ACS_RARROW;
			   break;
			case 'U':
			   lastChar = ACS_UARROW;
			   break;
			case 'D':
			   lastChar = ACS_DARROW;
			   break;
			}
#endif
			break;
		     default:
#if 0
			if (isdigit (string[from + 1])
			    && isdigit (string[from + 2]))
			   /* special case to allow control characters to be edited */
			   lastChar = (10 * ((string[from + 1] - '0'))
				       + (string[from + 2] - '0'));
			else
#endif
#if !(defined(__hpux) && !defined(NCURSES_VERSION))
			   if (string[from + 1] == 'D' &&
			       string[from + 2] == 'I')
			   lastChar = ACS_DIAMOND;
			else if (string[from + 1] == 'C' &&
				 string[from + 2] == 'B')
			   lastChar = ACS_CKBOARD;
			else if (string[from + 1] == 'D' &&
				 string[from + 2] == 'G')
			   lastChar = ACS_DEGREE;
			else if (string[from + 1] == 'P' &&
				 string[from + 2] == 'M')
			   lastChar = ACS_PLMINUS;
			else if (string[from + 1] == 'B' &&
				 string[from + 2] == 'U')
			   lastChar = ACS_BULLET;
			else if (string[from + 1] == 'S' &&
				 string[from + 2] == '1')
			   lastChar = ACS_S1;
			else if (string[from + 1] == 'S' &&
				 string[from + 2] == '9')
			   lastChar = ACS_S9;
#endif
			break;
		     }

		     if (lastChar != 0)
		     {
			adjust = 1;
			from += 2;

			if (string[from + 1] == '(')
			   /* Check for a possible numeric modifier.  */
			{
			   from++;
			   adjust = 0;

			   while (string[++from] != ')' && string[from] != 0)
			   {
			      if (isdigit (CharOf (string[from])))
			      {
				 adjust = (adjust * 10) + DigitOf (string[from]);
			      }
			   }
			}
		     }
		     for (x = 0; x < adjust; x++)
		     {
			if (result != 0)
			   result[used] = lastChar | attrib;
			used++;
		     }
		     break;
		  }
	       case '/':
		  from = encodeAttribute (string, from, &mask);
		  attrib = attrib | mask;
		  break;
	       case '!':
		  from = encodeAttribute (string, from, &mask);
		  attrib = attrib & ~mask;
		  break;
	       }
	    }
	 }

	 if (result != 0)
	 {
	    result[used] = 0;
	    result[used + 1] = 0;
	 }

	 /*
	  * If there are no characters, put the attribute into the
	  * the first character of the array.
	  */
	 if (used == 0
	     && result != 0)
	 {
	    result[0] = attrib;
	 }
      }
      *to = used;
   }
   else
   {
      /*
       * Try always to return something; otherwise lists of chtype strings
       * would get a spurious null pointer whenever there is a blank line,
       * and CDKfreeChtypes() would fail to free the whole list.
       */
      result = typeCallocN (chtype, 1);
   }
   return result;
}

/*
 * This determines the length of a chtype string
 */
int chlen (const chtype *string)
{
   int result = 0;

   if (string != 0)
   {
      while (string[result] != 0)
	 result++;
   }

   return (result);
}

/*
 * Compare a regular string to a chtype string
 */
int cmpStrChstr (const char *str, const chtype *chstr)
{
   int r = 0;

   if (!str && !chstr)
      return 0;
   if (!str)
      return 1;
   if (!chstr)
      return -1;

   while (!r && *str && *chstr)
   {
      r = *str - CharOf (*chstr);
      ++str;
      ++chstr;
   }

   if (r)
      return r;
   else if (!*str)
      return -1;
   else if (!*chstr)
      return 1;
   return 0;
}

void chstrncpy (char *dest, const chtype *src, int maxcount)
{
   int i = 0;

   while (i < maxcount && *src)
      *dest++ = (char)(*src++);

   *dest = '\0';
}

/*
 * This returns a pointer to char * of a chtype *
 * Formatting codes are omitted.
 */
char *chtype2Char (const chtype *string)
{
   char *newstring = 0;

   if (string != 0)
   {
      int len = chlen (string);

      if ((newstring = typeMallocN (char, len + 1)) != 0)
      {
	 int x;

	 for (x = 0; x < len; x++)
	 {
	    newstring[x] = (char)CharOf (string[x]);
	 }
	 newstring[len] = '\0';
      }
   }
   return (newstring);
}

/*
 * This returns a pointer to char * of a chtype *
 * Formatting codes are embedded.
 */
char *chtype2String (const chtype *string)
{
   char *newstring = 0;

   if (string != 0)
   {
      int pass;
      int len = chlen (string);

      for (pass = 0; pass < 2; ++pass)
      {
	 int x;
	 unsigned need = 0;

	 for (x = 0; x < len; ++x)
	 {
	    chtype this_ch = string[x];
	    chtype last_ch = (x > 0) ? string[x - 1] : 0;
	    unsigned next = ((this_ch & 0xff) < 32) ? 5 : 1;

	    need = decodeAttribute (newstring, need, last_ch, this_ch);
	    if (newstring != 0)
	    {
	       if (next == 1)
	       {
		  newstring[need] = (char)this_ch;
	       }
	       else
	       {
		  unsigned ch = (0xff & this_ch);
		  newstring[need] = L_MARKER;
		  newstring[need + 1] = '#';
		  newstring[need + 2] = (char)('0' + (ch / 10));
		  newstring[need + 3] = (char)('0' + (ch % 10));
		  newstring[need + 4] = R_MARKER;

	       }
	    }
	    need += next;
	 }
	 if (pass)
	 {
	    newstring[need] = 0;	/* keep a null on the end */
	 }
	 ++need;
	 if (!pass)
	 {
	    if ((newstring = typeCallocN (char, need)) == 0)
	         break;
	 }
      }
   }
   return (newstring);
}

static int comparSort (const void *a, const void *b)
{
   return strcmp (*(const char *const *)a, (*(const char *const *)b));
}

void sortList (CDK_CSTRING *list, int length)
{
   if (length > 1)
      qsort (list, (unsigned)length, sizeof (list[0]), comparSort);
}

/*
 * This strips white space from the given string.
 */
void stripWhiteSpace (EStripType stripType, char *string)
{
   /* Declare local variables.  */
   size_t stringLength = 0;

   /* Make sure the string is not null.  */
   if (string != 0
       && (stringLength = strlen (string)) != 0)
   {
      /* Strip leading whitespace */
      if (stripType == vFRONT || stripType == vBOTH)
      {
	 unsigned alphaChar = 0;
	 unsigned x;

	 /* Find the first non-whitespace character.  */
	 while (string[alphaChar] == ' ' || string[alphaChar] == '\t')
	 {
	    alphaChar++;
	 }

	 for (x = alphaChar; x <= stringLength; ++x)
	    string[x - alphaChar] = string[x];
      }

      /* Strip trailing whitespace */
      if (stripType == vBACK || stripType == vBOTH)
      {
	 stringLength = strlen (string);
	 while (stringLength-- != 0 &&
		(string[stringLength] == ' ' ||
		 string[stringLength] == '\t'))
	 {
	    string[stringLength] = '\0';
	 }
      }
   }
}

static unsigned countChar (const char *string, int separator)
{
   unsigned result = 0;
   int ch;

   while ((ch = *string++) != 0)
   {
      if (ch == separator)
	 result++;
   }
   return result;
}

/*
 * Split a string into a list of strings.
 */
char **CDKsplitString (const char *string, int separator)
{
   char **result = 0;
   char *temp;

   if (string != 0 && *string != 0)
   {
      unsigned need = countChar (string, separator) + 2;
      if ((result = typeMallocN (char *, need)) != 0)
      {
	 unsigned item = 0;
	 const char *first = string;
	 for (;;)
	 {
	    while (*string != 0 && *string != separator)
	       string++;

	    need = (unsigned)(string - first);
	    if ((temp = typeMallocN (char, need + 1)) == 0)
	         break;

	    memcpy (temp, first, need);
	    temp[need] = 0;
	    result[item++] = temp;

	    if (*string++ == 0)
	       break;
	    first = string;
	 }
	 result[item] = 0;
      }
   }
   return result;
}

/*
 * Add a new string to a list.  Keep a null pointer on the end so we can use
 * CDKfreeStrings() to deallocate the whole list.
 */
unsigned CDKallocStrings (char ***list, char *item, unsigned length, unsigned used)
{
   unsigned need = 1;

   while (need < length + 2)
      need *= 2;
   if (need > used)
   {
      used = need;
      if (*list == 0)
      {
	 *list = typeMallocN (char *, used);
      }
      else
      {
	 *list = typeReallocN (char *, *list, used);
      }
   }
   (*list)[length++] = copyChar (item);
   (*list)[length] = 0;
   return used;
}

/*
 * Count the number of items in a list of strings.
 */
unsigned CDKcountStrings (CDK_CSTRING2 list)
{
   unsigned result = 0;
   if (list != 0)
   {
      while (*list++ != 0)
	 result++;
   }
   return result;
}

/*
 * Free a list of strings, terminated by a null pointer.
 */
void CDKfreeStrings (char **list)
{
   if (list != 0)
   {
      void *base = (void *)list;
      while (*list != 0)
	 free (*list++);
      free (base);
   }
}

/*
 * Free a list of chtype-strings, terminated by a null pointer.
 */
void CDKfreeChtypes (chtype **list)
{
   if (list != 0)
   {
      void *base = (void *)list;
      while (*list != 0)
      {
	 freeChtype (*list++);
      }
      free (base);
   }
}

int mode2Filetype (mode_t mode)
{
   /* *INDENT-OFF* */
   static const struct {
      mode_t	mode;
      char	code;
   } table[] = {
#ifdef S_IFBLK
      { S_IFBLK,  'b' },  /* Block device */
#endif
      { S_IFCHR,  'c' },  /* Character device */
      { S_IFDIR,  'd' },  /* Directory */
      { S_IFREG,  '-' },  /* Regular file */
#ifdef S_IFLNK
      { S_IFLNK,  'l' },  /* Socket */
#endif
#ifdef S_IFSOCK
      { S_IFSOCK, '@' },  /* Socket */
#endif
      { S_IFIFO,  '&' },  /* Pipe */
   };
   /* *INDENT-ON* */

   int filetype = '?';
   unsigned n;

   for (n = 0; n < sizeof (table) / sizeof (table[0]); n++)
   {
      if ((mode & S_IFMT) == table[n].mode)
      {
	 filetype = table[n].code;
	 break;
      }
   }

   return filetype;

}

/*
 * This function takes a mode_t type and creates a string represntation
 * of the permission mode.
 */
int mode2Char (char *string, mode_t mode)
{
   /* *INDENT-OFF* */
   static struct {
      mode_t	mask;
      unsigned	col;
      char	flag;
   } table[] = {
      { S_IRUSR,	1,	'r' },
      { S_IWUSR,	2,	'w' },
      { S_IXUSR,	3,	'x' },
#if defined (S_IRGRP) && defined (S_IWGRP) && defined (S_IXGRP)
      { S_IRGRP,	4,	'r' },
      { S_IWGRP,	5,	'w' },
      { S_IXGRP,	6,	'x' },
#endif
#if defined (S_IROTH) && defined (S_IWOTH) && defined (S_IXOTH)
      { S_IROTH,	7,	'r' },
      { S_IWOTH,	8,	'w' },
      { S_IXOTH,	9,	'x' },
#endif
#ifdef S_ISUID
      { S_ISUID,	3,	's' },
#endif
#ifdef S_ISGID
      { S_ISGID,	6,	's' },
#endif
#ifdef S_ISVTX
      { S_ISVTX,	9,	't' },
#endif
   };
   /* *INDENT-ON* */

   int permissions = 0;
   int filetype = mode2Filetype (mode);
   unsigned n;

   /* Clean the string.  */
   cleanChar (string, 11, '-');
   string[11] = '\0';

   if (filetype == '?')
      return -1;

   for (n = 0; n < sizeof (table) / sizeof (table[0]); n++)
   {
      if ((mode & table[n].mask) != 0)
      {
	 string[table[n].col] = table[n].flag;
	 permissions |= (int)table[n].mask;
      }
   }

   /* Check for unusual permissions.  */
#ifdef S_ISUID
   if (((mode & S_IXUSR) == 0) &&
       ((mode & S_IXGRP) == 0) &&
       ((mode & S_IXOTH) == 0) &&
       (mode & S_ISUID) != 0)
   {
      string[3] = 'S';
   }
#endif

   return permissions;
}

/*
 * This returns the length of the integer.
 */
int intlen (int value)
{
   if (value < 0)
      return 1 + intlen (-value);
   else if (value >= 10)
      return 1 + intlen (value / 10);
   return 1;
}

/*
 * This opens the current directory and reads the contents.
 */
int CDKgetDirectoryContents (const char *directory, char ***list)
{
   /* Declare local variables.  */
   struct dirent *dirStruct;
   int counter = 0;
   DIR *dp;
   unsigned used = 0;

   /* Open the directory.  */
   dp = opendir (directory);

   /* Could we open the directory?  */
   if (dp == 0)
   {
      return -1;
   }

   /* Read the directory.  */
   while ((dirStruct = readdir (dp)) != 0)
   {
      if (strcmp (dirStruct->d_name, "."))
	 used = CDKallocStrings (list, dirStruct->d_name,
				 (unsigned)counter++, used);
   }

   /* Close the directory.  */
   closedir (dp);

   /* Sort the info.  */
   sortList ((CDK_CSTRING *)*list, counter);

   /* Return the number of files in the directory.  */
   return counter;
}

/*
 * This looks for a subset of a word in the given list.
 */
int searchList (CDK_CSTRING2 list, int listSize, const char *pattern)
{
   int Index = -1;

   /* Make sure the pattern isn't null. */
   if (pattern != 0)
   {
      size_t len = strlen (pattern);
      int x;

      /* Cycle through the list looking for the word. */
      for (x = 0; x < listSize; x++)
      {
	 /* Do a string compare. */
	 int ret = strncmp (list[x], pattern, len);

	 /*
	  * If 'ret' is less than 0, then the current word is alphabetically
	  * less than the provided word.  At this point we will set the index
	  * to the current position.  If 'ret' is greater than 0, then the
	  * current word is alphabetically greater than the given word.  We
	  * should return with index, which might contain the last best match. 
	  * If they are equal, then we've found it.
	  */
	 if (ret < 0)
	 {
	    Index = ret;
	 }
	 else
	 {
	    if (ret == 0)
	       Index = x;
	    break;
	 }
      }
   }
   return Index;
}

/*
 * This function checks to see if a link has been requested.
 */
int checkForLink (const char *line, char *filename)
{
   int len = 0;
   int fPos = 0;

   /* Make sure the line isn't null. */
   if (line == 0)
   {
      return -1;
   }
   len = (int)strlen (line);

   /* Strip out the filename. */
   if (line[0] == L_MARKER && line[1] == 'F' && line[2] == '=')
   {
      int x = 3;

      /* Strip out the filename.  */
      while (x < len)
      {
	 if (line[x] == R_MARKER)
	 {
	    break;
	 }
	 if (fPos < CDK_PATHMAX)
	    filename[fPos++] = line[x];
	 ++x;
      }
   }
   filename[fPos] = '\0';
   return (fPos != 0);
}

/*
 * Returns the filename portion of the given pathname, i.e., after the last
 * slash.
 */
char *baseName (char *pathname)
{
   char *base = 0;

   if (pathname != 0
       && *pathname != '\0'
       && (base = copyChar (pathname)) != 0)
   {
      size_t pathLen;

      if ((pathLen = strlen (pathname)) != 0)
      {
	 size_t x;

	 for (x = pathLen - 1; x != 0; --x)
	 {
	    /* Find the last '/' in the pathname. */
	    if (pathname[x] == '/')
	    {
	       strcpy (base, pathname + x + 1);
	       break;
	    }
	 }
      }
   }
   return base;
}

/*
 * Returns the directory for the given pathname, i.e., the part before the
 * last slash.
 */
char *dirName (char *pathname)
{
   char *dir = 0;
   size_t pathLen;

   /* Check if the string is null.  */
   if (pathname != 0
       && (dir = copyChar (pathname)) != 0
       && (pathLen = strlen (pathname)) != 0)
   {
      size_t x = pathLen;
      while ((dir[x] != '/') && (x > 0))
      {
	 dir[x--] = '\0';
      }
   }

   return dir;
}

/*
 * If the dimension is a negative value, the dimension will be the full
 * height/width of the parent window - the value of the dimension.  Otherwise,
 * the dimension will be the given value.
 */
int setWidgetDimension (int parentDim, int proposedDim, int adjustment)
{
   int dimension = 0;

   /* If the user passed in FULL, return the parent's size. */
   if ((proposedDim == FULL) || (proposedDim == 0))
   {
      dimension = parentDim;
   }
   else
   {
      /* If they gave a positive value, return it. */
      if (proposedDim >= 0)
      {
	 if (proposedDim >= parentDim)
	    dimension = parentDim;
	 else
	    dimension = (proposedDim + adjustment);
      }
      else
      {
	 /*
	  * If they gave a negative value, then return the
	  * dimension of the parent plus the value given.
	  */
	 dimension = parentDim + proposedDim;

	 /* Just to make sure. */
	 if (dimension < 0)
	    dimension = parentDim;
      }
   }
   return dimension;
}

/*
 * This safely erases a given window.
 */
void eraseCursesWindow (WINDOW *window)
{
   if (window != 0)
   {
      werase (window);
      wrefresh (window);
   }
}

/*
 * This safely deletes a given window.
 */
void deleteCursesWindow (WINDOW *window)
{
   if (window != 0)
   {
      eraseCursesWindow (window);
      delwin (window);
   }
}

/*
 * This moves a given window (if we're able to set the window's beginning).
 * We do not use mvwin(), because it does (usually) not move subwindows.
 */
void moveCursesWindow (WINDOW *window, int xdiff, int ydiff)
{
   if (window != 0)
   {
      int xpos, ypos;

      getbegyx (window, ypos, xpos);
      (void)setbegyx (window, (short)ypos, (short)xpos);
      xpos += xdiff;
      ypos += ydiff;
      werase (window);
      (void)setbegyx (window, (short)ypos, (short)xpos);
      (void)ypos;
      (void)xpos;
   }
}

/*
 * Return an integer like 'floor()', which returns a double.
 */
int floorCDK (double value)
{
   int result = (int)value;
   if (result > value)		/* e.g., value < 0.0 and value is not an integer */
      result--;
   return result;
}

/*
 * Return an integer like 'ceil()', which returns a double.
 */
int ceilCDK (double value)
{
   return -floorCDK (-value);
}

/*
 * Compatibility for different versions of curses.
 */
#if !(defined(HAVE_GETBEGX) && defined(HAVE_GETBEGY))
int getbegx (WINDOW *win)
{
   int y, x;
   getbegyx (win, y, x);
   return x;
}
int getbegy (WINDOW *win)
{
   int y, x;
   getbegyx (win, y, x);
   return y;
}
#endif

#if !(defined(HAVE_GETMAXX) && defined(HAVE_GETMAXY))
int getmaxx (WINDOW *win)
{
   int y, x;
   getmaxyx (win, y, x);
   return x;
}
int getmaxy (WINDOW *win)
{
   int y, x;
   getmaxyx (win, y, x);
   return y;
}
#endif
