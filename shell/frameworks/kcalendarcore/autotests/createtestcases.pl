#!/usr/bin/perl

# Written 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
# This file is put into the public domain, I claim no copyright on that code
#
# This little (really dumb) script can be used to split a large calendar file 
# up into several small files each containing one event/journal/todo/freebusy.
# All lines before the first VEVENT/VTODO/VJOURNAL/VFREEBUSY will be copied to 
# every file, so timezones should be fine.
# 
# Usage:
#   createtestcases.pl InputFile.ics OutputBaseFileName [ext]
#
# InputFile.ics .. The large calendar file containing multiple incidences
# OutputBaseFileName .. The base name of the output files. They will be
#                       numbered sequentially, i.e. OutputBaseFileName1.ext,
#                       OutputBaseFileName2.ext, etc.
# [ext].. optional file name extension for the output files. If none is given,
#         ics is used.
#
# Disclaimer: This script exists merely for development purposes to help us 
# KDE developer generate atomic test cases for libkcalcore's test suite.


use Cwd;

my $infile=shift @ARGV;
my $filebase=shift @ARGV;
my $ext=shift @ARGV;
if ( !$ext ) {
	$ext = "ics";
}
my $filenr=0;
my $filename="$filebase$filenr.$ext";

print "Infile: $infile\n";
print "filebase: $filebase\n";
print "filenr: $filenr\n";
print "filename: $filename\n";

my $pre = 1;
my $inevent = 0;
my $pretext = "";
my $opened = 0;


open(INFILE, $infile);
while (<INFILE>) {
  my $line = $_;
	
	if ( !$inevent && ($line =~ m/BEGIN:V(EVENT|TODO|FREEBUSY|JOURNAL)/) ) {
	  if ( !$opened ) {
			$opened = 1;
  		$filenr=$filenr+1;
	  	$filename="$filebase$filenr.$ext";
		  open fh, "> $filename";
  		if ( $pretext ) {
 	  		print fh $pretext;
 			} else {
	 			# TODO: close file, open new file
			 	print fh "BEGIN:VCALENDAR\nVERSION:2.0\nPRODID:-//K Desktop Environment (KDE)//LibKCal test cases 1.0//EN\n\n";
			}
			
		}
		$pre = 0;
		$inevent = 1;
		print fh $line;
	} elsif ( $inevent && ($line =~ m/END:V(EVENT|TODO|FREEBUSY|JOURNAL)/) ) {
		$inevent = 0;
		print fh $line;
		print fh "END:VCALENDAR\n";
		close fh;
		$opened = 0;
	} elsif ( $inevent ) {
		print fh $line;
	} elsif ( $pre ) {
		$pretext="$pretext$line";	
print "PRE: $pretext";
	} 
	
}

