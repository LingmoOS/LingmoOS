#include <cdk_int.h>

/*
 * $Author: tom $
 * $Date: 2012/03/20 21:58:33 $
 * $Revision: 1.11 $
 */

FILE *CDKDEBUG;


/*
 * This starts debugging for CDK.
 */
FILE *startCDKDebug (const char *filename)
{
   const char *defFile = "cdkdebug.log";

   /* Check if the filename is null. */
   if (filename == 0)
   {
      filename = defFile;
   }

   /* Try to open the file. */
   return (fopen (filename, "w"));
}

/*
 * This writes a message to the debug file.
 */
void writeCDKDebugMessage (FILE * fd,
			   const char *filename,
			   const char *function,
			   int line, const char *message)
{
   /* Print the message as long as the file pointer is not null. */
   if (fd != 0)
   {
      fprintf (fd, "%s::%s (Line %d) %s\n", filename, function, line, message);
   }
}

/*
 * This turns off the debugging for CDK.
 */
void stopCDKDebug (FILE * fd)
{
   if (fd != 0)
   {
      fclose (fd);
   }
}
