#!/usr/bin/perl -w
# Script to filter out the output of websec-txt-all script
# Contributed by Jacobo Tarrio <jtarrio@debian.org>

use strict;

my @oldstat = ();
my @newstat = ();
my %oldfiles = ();
my %newfiles = ();
my $oldglobal = "";
my $newglobal = "";
my $oldmaster = "(unknown)";
my $newmaster = "(unknown)";
my $oldunkwords = -1;
my $newunkwords = -1;
my $oldwrongvars = -1;
my $newwrongvars = -1;
my $oldwrongmsgstrs = -1;
my $newwrongmsgstrs = -1;

# Gather data
my @lines = <>;
foreach my $line (@lines) {
  chomp $line;
  if ($line =~ /^---\s+.*?level(\d)_(.*?)\.txt\.old\s+(.*?)\.0+\s+([-+]\d+)$/) {
    @oldstat = ($1, $2, "$3 $4");
  }
  elsif ($line =~ /^\+\+\+\s+.*?level(\d)_(.*?)\.txt\s+(.*?)\.0+\s+([-+]\d+)$/) {
    @newstat = ($1, $2, "$3 $4");
  }
  elsif ($line =~ /^([-+])Global statistics: (\d+t\d+f\d+u)/) {
    $oldglobal = $2 if ($1 eq '-');
    $newglobal = $2 if ($1 eq '+');
  }
  elsif ($line =~ /^([-+])(?:\s+\*?)(.*?\.po):?\s+((\d+t)?(\d+f)?(\d+u)?)/) {
    $oldfiles{$2} = $3 if ($1 eq '-');
    $newfiles{$2} = $3 if ($1 eq '+');
  }
  elsif ($line =~ /^([-+])Master:\s+(\d*t\d*f\d*u)/) {
    $oldmaster = $2 if ($1 eq '-');
    $newmaster = $2 if ($1 eq '+');
  }
  elsif ($line =~ /^([-+])\s+(-?\d+) unknown words/) {
    $oldunkwords = $2 if ($1 eq '-');
    $newunkwords = $2 if ($1 eq '+');
  }
  elsif ($line =~ /^([-+])\s+(-?\d+) wrong variables/) {
    $oldwrongvars = $2 if ($1 eq '-');
    $newwrongvars = $2 if ($1 eq '+');
  }
  elsif ($line =~ /^([-+])\s+(-?\d+) level-specific wrong msgstrs/) {
    $oldwrongmsgstrs = $2 if ($1 eq '-');
    $newwrongmsgstrs = $2 if ($1 eq '+');
  }
}

# Prepare data
my $new = "";
my $changed = "";
my $removed = "";
foreach my $f (sort keys %newfiles) {
  if (!defined $oldfiles{$f}) {
    $new .= "  * $f : $newfiles{$f}\n";
  }
  else {
    $changed .= "  * $f : $oldfiles{$f} -> $newfiles{$f}\n";
    delete $oldfiles{$f};
  }
}
foreach my $f (sort keys %oldfiles) {
  $removed .= "  * $f : $oldfiles{$f}\n";
}

# Create translation report
my $transreport = "";
$transreport .= "Removed files:\n$removed\n" if ($removed ne "");
$transreport .= "Added files:\n$new\n" if ($new ne "");
$transreport .= "Changed files:\n$changed\n" if ($changed ne "");

$transreport .= "Master translation: $oldmaster -> $newmaster\n\n" if ($oldmaster ne $newmaster);

$transreport .= "Old totals: $oldglobal\n" if ($oldglobal ne "");
$transreport .= "New totals: $newglobal\n" if ($newglobal ne "");

# Create spellchecker report
my $spellreport = "";
if ($newunkwords >= 0) {
  $spellreport .= "Unknown words: $newunkwords";
  $spellreport .= " (from $oldunkwords)" if ($oldunkwords >= 0);
  $spellreport .= "\n";
}
if ($newwrongvars >= 0) {
  $spellreport .= "Wrong variables: $newwrongvars";
  $spellreport .= " (from $oldwrongvars)" if ($oldwrongvars >= 0);
  $spellreport .= "\n";
}
if ($newwrongmsgstrs >= 0) {
  $spellreport .= "Level-specific wrong msgstrs: $newwrongmsgstrs";
  $spellreport .= " (from $oldwrongmsgstrs)" if ($oldwrongmsgstrs >= 0);
  $spellreport .= "\n";
}


# Display report if not empty
if ($transreport ne "" || $spellreport ne "") {
  print "Changes for $newstat[1] level $newstat[0]\n";
  print "   between $oldstat[2] and $newstat[2]\n";
  print "\n";
  print "$transreport\n" if ($transreport ne "");
  print "$spellreport\n" if ($spellreport ne "");
  print "\n";
  print "Check http://d-i.debian.org/l10n-stats/ to access D-I levels material\n";
  print "Check http://d-i.debian.org/l10n-spellcheck/ for details about spellchecker issues\n";
}
