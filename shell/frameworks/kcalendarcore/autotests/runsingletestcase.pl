#!/usr/bin/perl

# This file is part of the kcalcore library.
#
# SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
# SPDX-FileCopyrightText: 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
#
# SPDX-License-Identifier: LGPL-2.0-or-later

# This little script runs a test program on a given (calendar) file and 
# compares the output to a reference file. All discrepancies are shown 
# to the user. Usage:
#      runtestcase.pl appname testfile.ics
# The application/script appname is required to take two arguments:
#      appname inputfile outputfile
# where inputfile is the file to be used as input data, and the output of the
# program will go to outputfile (=testfile.ics.out if called through 
# runtestcase.pl). That outputfile is then compared to the reference file
# testfile.ics.ref.

if ( @ARGV != 4 ) {
  print STDERR "Missing arg! Arguments: testapp identifier filename \n";
  exit 1;
}

$app = "$ARGV[0]";
$id = "$ARGV[1]";
$file_orig = "$ARGV[2]";
$file = "$file_orig";
$outfile ="$ARGV[3]";  # necessary to avoid creating files in the $file directory

$MAXERRLINES=25;

$outfile = "$outfile.$id.out";

if ( $^O eq "MSWin32" || $^O eq "msys" ) {
  $testcmd = "\"$app\" \"$file\" \"$outfile\" 2> \$null";
} else {
  $testcmd = "\"$app\" \"$file\" \"$outfile\"";
}

#print "CMD $testcmd\n";

if ( system( $testcmd ) != 0 ) {
  print STDERR "Error running $app\n";
  exit 1;
}

checkfile( $file, $file_orig, $outfile );

exit 0;

sub checkfile()
{
  my $file = shift;
  my $file_orig = shift;
  my $outfile = shift;

  unless (-e "$file_orig.$id.ref") {
    print STDERR "Missing ref file: $file_orig.$id.ref\n";
    exit 1;
  }

  $cmd = 'diff -u -w -B -I "^DTSTAMP:[0-9ZT]*" -I "^LAST-MODIFIED:[0-9ZT]*" -I "^CREATED:[0-9ZT]*" -I "^DCREATED:[0-9ZT]*" -I "^X-KDE-KCALCORE-ENABLED:" -I "^X-KDE-ICAL-IMPLEMENTATION-VERSION:" -I "^PRODID:.*" -I "X-UID=[0-9]*" '."$file.$id.ref $outfile";
  if ( !open( DIFF, "$cmd|" ) ) {
    print STDERR "Unable to run diff command on the files $file_orig.$id.ref and $outfile\n";
    exit 1;
  }

  $errors = 0;
  $errorstr = "";
  while ( <DIFF> ) {
    $line = $_;
    next if ($line =~ m/^[+-]\s*(DTSTAMP|LAST-MODIFIED|CREATED|DCREATED|X-KDE-KCALCORE-ENABLED|X-KDE-ICAL-IMPLEMENTATION-VERSION|PRODID|X-UID)/);
    next if ($line =~ m/^[+-]\s*$/);
    next if ($line =~ m/No newline at end of file/);
    # cannot compare outfile to "/Compat/" because of the quotemeta stuff.
    next if ($outfile =~ m+/Compat\\/+ && $line =~ m/^[+-](SEQUENCE|PRIORITY|ORGANIZER:MAILTO):/);
    if ( $line =~ /^[+-][^+-]/ ) {
      # it's an added/deleted/modified line. Register it as an error
      $errors++;
    }
    $errorstr .= $line;
  }

  if ( $errors > 0 ) {
    print "~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=\n";
    print "Checking '$outfile':\n";
    print $errorstr;
    print "Encountered $errors errors\n";

    if ( !open( ERRLOG, ">>FAILED.log" ) ) {
      print "Unable to open FAILED.log";
    };
    print ERRLOG "\n\n\n~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=~=\n\n\n";
    print ERRLOG "Checking '$outfile':\n";
    print ERRLOG "Command: $testcmd\n";
    print ERRLOG $errorstr;

    if ( -e "$file_orig.$id.fixme" ) {
      if ( !open( FIXME, "$file_orig.$id.fixme" ) ) {
        print STDERR "Unable to open $file_orig.$id.fixme\n";
        exit 1;
      }
      my $firstline = <FIXME>;
      $firstline =~ /^(\d+) known errors/;
      my $expected = $1;
      if ( $expected == $errors ) {
        print ERRLOG "\n  EXPECTED FAIL: $errors errors found.\n";
        print ERRLOG "    Fixme:\n";
        while( <FIXME> ) {
          print ERRLOG "      ";
          print ERRLOG;
        }
      } else {
        print ERRLOG "\n  UNEXPECTED FAIL: $errors errors found, $expected expected.\n";
        exit 1;
      }
    } else {
      print ERRLOG "\n  FAILED: $errors errors found.\n";
#       system( "touch FAILED" );
      exit 1;
    }
  } else {
     unlink($outfile);
  }
}
