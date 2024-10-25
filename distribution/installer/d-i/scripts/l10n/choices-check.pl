#!/usr/bin/perl -w
#
# This script checks the PO file in stdin and outputs all Choices-type
# entries for Select or Multiselect templates which have over 65
# characters for one of their components
#
# This is aimed at detecting D-I translations which may exceed this 
# common limit imposed by the dialog interface of debconf
#
# This script was contributed by Eugeniy Meshcheryakov <eugen@univ.kiev.ua>
#
# Usage: choices-check.pl <file
#
# The script outputs nothing if no offending line was found
# and two numbers per offending line otherwise : the line number
# and the number of characters in it

use strict;
use utf8;
use Text::CharWidth qw(mbswidth);

my ($line, $n, $first);
$n = 0;
while ($n++, $line = <STDIN>) {
	if (($line =~ '^#\. Type: select') ||
		($line =~ '^#\. Type: multiselect')) {
		$line = <STDIN>; $n++;
		
		next if (!($line =~ '^#\. Choices'));
		do {
			$line = <STDIN>; $n++;
		} while (!($line =~ '^msgstr'));
			
		my ($msgstart, $msg);
		$msgstart = $n;
		
		$line =~ /^msgstr "(.*)"$/;
		$msg = $1;
		while (1) {
			$line = <STDIN>; $n++;
			last if (!($line =~ '^"'));
			$line =~ /^"(.*)"$/;
			$msg .= $1;
		}
		my @choices = split /, /, $msg;
		my @widths = map {mbswidth($_)}  @choices;
		my $max = 0;
			
		foreach (@widths) {
			$max = $_ if ($_ > $max);
		}
		
		print "$msgstart $max\n" if ($max > 65);
	}
}

