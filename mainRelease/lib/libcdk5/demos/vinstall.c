/* $Id: vinstall.c,v 1.22 2019/02/20 02:13:54 tom Exp $ */

#include <cdk_test.h>

#ifdef HAVE_XCURSES
char *XCursesProgramName = "vinstall";
#endif

/*
 * Written by:	Mike Glover
 * Purpose:
 *		This is a fairly basic install interface.
 */

/* Declare global types and prototypes. */
static const char *FPUsage = "-f filename [-s source directory] [-d destination directory] [-t title] [-o Output file] [-q]";

typedef enum
{
   vCanNotOpenSource,
   vCanNotOpenDest,
   vOK
}
ECopyFile;

static ECopyFile copyFile (CDKSCREEN *cdkScreen, char *src, char *dest);
static int verifyDirectory (CDKSCREEN *screen, char *directory);

int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkScreen         = 0;
   CDKSWINDOW *installOutput    = 0;
   CDKENTRY *sourceEntry        = 0;
   CDKENTRY *destEntry          = 0;
   CDKLABEL *titleWin           = 0;
   CDKHISTOGRAM *progressBar    = 0;
   char *sourcePath             = 0;
   char *destPath               = 0;
   char *sourceDir              = 0;
   char *destDir                = 0;
   char *filename               = 0;
   char *title                  = 0;
   char *output                 = 0;
   int quiet                    = FALSE;
   int errors                   = 0;
   int sWindowHeight            = 0;
   char *titleMessage[10];
   char **fileList              = 0;
   const char *mesg[20];
   char oldPath[512], newPath[512], temp[2000];
   int count, ret, x;

   /* Parse up the command line. */
   while (1)
   {
      ret = getopt (argc, argv, "d:s:f:t:o:q");
      if (ret == -1)
      {
	 break;
      }
      switch (ret)
      {
      case 's':
	 sourcePath = strdup (optarg);
	 break;

      case 'd':
	 destPath = strdup (optarg);
	 break;

      case 'f':
	 filename = strdup (optarg);
	 break;

      case 't':
	 title = strdup (optarg);
	 break;

      case 'o':
	 output = strdup (optarg);
	 break;

      case 'q':
	 quiet = TRUE;
	 break;
      }
   }

   /* Make sure have everything we need. */
   if (filename == 0)
   {
      fprintf (stderr, "Usage: %s %s\n", argv[0], FPUsage);
      ExitProgram (EXIT_FAILURE);
   }

   /* Open the file list file and read it in. */
   count = CDKreadFile (filename, &fileList);
   if (count == 0)
   {
      fprintf (stderr, "%s: Input filename <%s> is empty.\n", argv[0], filename);
      ExitProgram (EXIT_FAILURE);
   }

   /*
    * Cycle through what was given to us and save it.
    */
   for (x = 0; x < count; x++)
   {
      /* Strip white space from the line. */
      stripWhiteSpace (vBOTH, fileList[x]);
   }

   cdkScreen = initCDKScreen (NULL);

   /* Start color. */
   initCDKColor ();

   /* Create the title label. */
   titleMessage[0] = copyChar ("<C></32/B><#HL(30)>");
   if (title == 0)
   {
      sprintf (temp, "<C></32/B>CDK Installer");
   }
   else
   {
      sprintf (temp, "<C></32/B>%.256s", title);
   }
   titleMessage[1] = copyChar (temp);
   titleMessage[2] = copyChar ("<C></32/B><#HL(30)>");
   titleWin = newCDKLabel (cdkScreen, CENTER, TOP,
			   (CDK_CSTRING2)titleMessage, 3,
			   FALSE, FALSE);
   freeCharList (titleMessage, 3);

   /* Allow them to change the install directory. */
   if (sourcePath == 0)
   {
      sourceEntry = newCDKEntry (cdkScreen, CENTER, 8,
				 0, "Source Directory        :",
				 A_NORMAL, '.', vMIXED,
				 40, 0, 256, TRUE, FALSE);
   }
   if (destPath == 0)
   {
      destEntry = newCDKEntry (cdkScreen, CENTER, 11,
			       0, "Destination Directory:", A_NORMAL,
			       '.', vMIXED, 40, 0, 256, TRUE, FALSE);
   }

   /* Get the source install path. */
   if (sourceEntry != 0)
   {
      drawCDKScreen (cdkScreen);
      sourceDir = copyChar (activateCDKEntry (sourceEntry, 0));
   }
   else
   {
      sourceDir = copyChar (sourcePath);
   }

   /* Get the destination install path. */
   if (destEntry != 0)
   {
      drawCDKScreen (cdkScreen);
      destDir = copyChar (activateCDKEntry (destEntry, 0));
   }
   else
   {
      destDir = copyChar (destPath);
   }

   /* Destroy the path entry fields. */
   if (sourceEntry != 0)
   {
      destroyCDKEntry (sourceEntry);
   }
   if (destEntry != 0)
   {
      destroyCDKEntry (destEntry);
   }

   /*
    * Verify that the source directory is valid.
    */
   if (verifyDirectory (cdkScreen, sourceDir) != 0)
   {
      /* Clean up and leave. */
      freeChar (destDir);
      freeChar (sourceDir);
      destroyCDKLabel (titleWin);
      destroyCDKScreen (cdkScreen);
      endCDK ();
      ExitProgram (EXIT_FAILURE);
   }

   /*
    * Verify that the source directory is valid.
    */
   if (verifyDirectory (cdkScreen, destDir) != 0)
   {
      /* Clean up and leave. */
      freeChar (destDir);
      freeChar (sourceDir);
      destroyCDKLabel (titleWin);
      destroyCDKScreen (cdkScreen);
      endCDK ();
      ExitProgram (EXIT_FAILURE);
   }

   /* Create the histogram. */
   progressBar = newCDKHistogram (cdkScreen, CENTER, 5,
				  3, 0, HORIZONTAL,
				  "<C></56/B>Install Progress",
				  TRUE, FALSE);

   /* Set the top left/right characters of the histogram. */
   setCDKHistogramLLChar (progressBar, ACS_LTEE);
   setCDKHistogramLRChar (progressBar, ACS_RTEE);

   /* Set the initial value of the histogram. */
   setCDKHistogram (progressBar, vPERCENT, TOP, A_BOLD,
		    1, count, 1,
		    COLOR_PAIR (24) | A_REVERSE | ' ',
		    TRUE);

   /* Determine the height of the scrolling window. */
   if (LINES >= 16)
   {
      sWindowHeight = LINES - 13;
   }
   else
   {
      sWindowHeight = 3;
   }

   /* Create the scrolling window. */
   installOutput = newCDKSwindow (cdkScreen, CENTER, BOTTOM,
				  sWindowHeight, 0,
				  "<C></56/B>Install Results",
				  2000, TRUE, FALSE);

   /* Set the top left/right characters of the scrolling window. */
   setCDKSwindowULChar (installOutput, ACS_LTEE);
   setCDKSwindowURChar (installOutput, ACS_RTEE);

   /* Draw the screen. */
   drawCDKScreen (cdkScreen);

   /* Start copying the files. */
   for (x = 0; x < count; x++)
   {
      char **files;
      int chunks;

      /*
       * If the 'file' list file has 2 columns, the first is
       * the source filename, the second being the destination
       * filename.
       */
      files = CDKsplitString (fileList[x], ' ');
      chunks = (int)CDKcountStrings ((CDK_CSTRING2)files);
      if (chunks == 2)
      {
	 /* Create the correct paths. */
	 sprintf (oldPath, "%s/%s", sourceDir, files[0]);
	 sprintf (newPath, "%s/%s", destDir, files[1]);
      }
      else
      {
	 /* Create the correct paths. */
	 sprintf (oldPath, "%s/%s", sourceDir, fileList[x]);
	 sprintf (newPath, "%s/%s", destDir, fileList[x]);
      }
      CDKfreeStrings (files);

      /* Copy the file from the source to the destination. */
      ret = copyFile (cdkScreen, oldPath, newPath);
      if (ret == vCanNotOpenSource)
      {
	 sprintf (temp,
		  "</16>Error: Can not open source file \"%.256s\"<!16>", oldPath);
	 errors++;
      }
      else if (ret == vCanNotOpenDest)
      {
	 sprintf (temp,
		  "</16>Error: Can not open destination file \"%.256s\"<!16>", newPath);
	 errors++;
      }
      else
      {
	 sprintf (temp, "</24>%.256s -> %.256s", oldPath, newPath);
      }

      /* Add the message to the scrolling window. */
      addCDKSwindow (installOutput, temp, BOTTOM);
      drawCDKSwindow (installOutput, ObjOf (installOutput)->box);

      /* Update the histogram. */
      setCDKHistogram (progressBar, vPERCENT, TOP, A_BOLD,
		       1, count, x + 1,
		       COLOR_PAIR (24) | A_REVERSE | ' ',
		       TRUE);

      /* Update the screen. */
      drawCDKHistogram (progressBar, TRUE);
   }

   /*
    * If there were errors, inform the user and allow them to look at the
    * errors in the scrolling window.
    */
   if (errors != 0)
   {
      /* Create the information for the dialog box. */
      const char *buttons[] =
      {
	 "Look At Errors Now",
	 "Save Output To A File",
	 "Ignore Errors"
      };
      mesg[0] = "<C>There were errors in the installation.";
      mesg[1] = "<C>If you want, you may scroll through the";
      mesg[2] = "<C>messages of the scrolling window to see";
      mesg[3] = "<C>what the errors were. If you want to save";
      mesg[4] = "<C>the output of the window you may press </R>s<!R>";
      mesg[5] = "<C>while in the window, or you may save the output";
      mesg[6] = "<C>of the install now and look at the install";
      mesg[7] = "<C>history at a later date.";

      /* Popup the dialog box. */
      ret = popupDialog (cdkScreen,
			 (CDK_CSTRING2)mesg, 8,
			 (CDK_CSTRING2)buttons, 3);
      if (ret == 0)
      {
	 activateCDKSwindow (installOutput, 0);
      }
      else if (ret == 1)
      {
	 (void)injectCDKSwindow (installOutput, 's');
      }
   }
   else
   {
      /*
       * If they specified the name of an output file, then save the
       * results of the installation to that file.
       */
      if (output != 0)
      {
	 dumpCDKSwindow (installOutput, output);
      }
      else
      {
	 /* Ask them if they want to save the output of the scrolling window. */
	 if (quiet == FALSE)
	 {
	    const char *buttons[] =
	    {
	       "No",
	       "Yes"
	    };
	    mesg[0] = "<C>Do you want to save the output of the";
	    mesg[1] = "<C>scrolling window to a file?";

	    if (popupDialog (cdkScreen,
			     (CDK_CSTRING2)mesg, 2,
			     (CDK_CSTRING2)buttons, 2) == 1)
	    {
	       (void)injectCDKSwindow (installOutput, 's');
	    }
	 }
      }
   }

   /* Clean up. */
   destroyCDKLabel (titleWin);
   destroyCDKHistogram (progressBar);
   destroyCDKSwindow (installOutput);
   destroyCDKScreen (cdkScreen);
   endCDK ();
   ExitProgram (EXIT_SUCCESS);
}

/*
 * This copies a file from one place to another. (tried rename
 * library call, but it is equivalent to mv)
 */
static ECopyFile copyFile (CDKSCREEN *cdkScreen GCC_UNUSED, char *src, char *dest)
{
#define MYSIZE 256
   char command[MYSIZE * 5];
   FILE *fd;

   /* Make sure we can open the source file. */
   if ((fd = fopen (src, "r")) == 0)
   {
      return vCanNotOpenSource;
   }
   fclose (fd);

   /*
    * Remove the destination file first, just in case it already exists.
    * This allows us to check if we can write to the desintation file.
    */
   sprintf (command, "rm -f %s", dest);
   system (command);

   /* Try to open the destination. */
   if ((fd = fopen (dest, "w")) == 0)
   {
      return vCanNotOpenDest;
   }
   fclose (fd);

   /*
    * Copy the file. There has to be a better way to do this. I
    * tried rename and link but they both have the same limitation
    * as the 'mv' command that you can not move across partitions.
    * Quite limiting in an install binary.
    */
   sprintf (command, "rm -f %.*s; cp %.*s %.*s; chmod 444 %.*s",
	    MYSIZE, dest, MYSIZE, src, MYSIZE, dest, MYSIZE, dest);
   system (command);
   return vOK;
}

/*
 * This makes sure that the directory given exists. If it
 * doesn't then it will make it.
 * THINK
 */
static int verifyDirectory (CDKSCREEN *cdkScreen, char *directory)
{
   int status = 0;
#if !defined (__MINGW32__)
   mode_t dirMode = S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH;
#endif
   struct stat fileStat;
   char temp[512];

   /* Stat the directory. */
   if (lstat (directory, &fileStat) != 0)
   {
      /* The directory does not exist. */
      if (errno == ENOENT)
      {
	 const char *buttons[] =
	 {
	    "Yes",
	    "No"
	 };
	 const char *mesg[10];
	 char *error[10];

	 /* Create the question. */
	 mesg[0] = "<C>The directory ";
	 sprintf (temp, "<C>%.256s", directory);
	 mesg[1] = temp;
	 mesg[2] = "<C>Does not exist. Do you want to";
	 mesg[3] = "<C>create it?";

	 /* Ask them if they want to create the directory. */
	 if (popupDialog (cdkScreen,
			  (CDK_CSTRING2)mesg, 4,
			  (CDK_CSTRING2)buttons, 2) == 0)
	 {
	    /* Create the directory. */
#if defined (__MINGW32__)
	    if (mkdir (directory) != 0)
#else
	    if (mkdir (directory, dirMode) != 0)
#endif
	    {
	       /* Create the error message. */
	       error[0] = copyChar ("<C>Could not create the directory");
	       sprintf (temp, "<C>%.256s", directory);
	       error[1] = copyChar (temp);

#ifdef HAVE_STRERROR
	       sprintf (temp, "<C>%.256s", strerror (errno));
#else
	       sprintf (temp, "<C>Check the permissions and try again.");
#endif
	       error[2] = copyChar (temp);

	       /* Pop up the error message. */
	       popupLabel (cdkScreen, (CDK_CSTRING2)error, 3);

	       freeCharList (error, 3);
	       status = -1;
	    }
	 }
	 else
	 {
	    /* Create the message. */
	    error[0] = copyChar ("<C>Installation aborted.");

	    /* Pop up the error message. */
	    popupLabel (cdkScreen, (CDK_CSTRING2)error, 1);

	    freeCharList (error, 1);
	    status = -1;
	 }
      }
   }
   return status;
}
