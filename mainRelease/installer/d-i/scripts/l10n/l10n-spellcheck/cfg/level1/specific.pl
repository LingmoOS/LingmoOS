#! /usr/bin/perl -w

use strict;
use Unicode::String;

sub checkSpecials (@)
{
    my $msgid = shift;
    my $msgstr = shift;
    my $count_id = 0;
    my $count_st = 0;
    my $open_ch;
    my $close_ch;
    my $unmatched=0;

#FIXME: uk uses „quoteme“ whereas zh_CN uses “quoteme” 
    my %pairs=('(', ')', 
	       '[', ']',
	       '«', '»');

#FIXME: for some langs makes sense to check also '\'', '`'
    my @singles=("\\\"");


    if (defined $msgstr)
    {
	foreach(@singles)
	{
	    if ($msgstr =~ m/$_/g)
	    {
		my $numquote=1;
		while ($msgstr =~ /$_/g) { $numquote++ };
		if ($numquote % 2)
		{
		    print "# >>> Unbalanced quote(s): $_ <<<\n";		
		    $unmatched=1;
		}		
	    }
	}

	# Check for unmatched pairs: () [] etc.
	my $bad_pair=0; 
	while (($open_ch, $close_ch) = each(%pairs) )
	{
	    if ($msgstr =~ /\Q$open_ch\E/)
	    {
		my $count_open=0;
		my $count_close=0;		   
		
		while ($msgstr =~ /\Q$open_ch\E/g) { $count_open++ }
		while ($msgstr =~ /\Q$close_ch\E/g) { $count_close++ }		    
		if ($count_open != $count_close)
		{
		    print "# >>> Unmatched pair(s): $open_ch $close_ch <<<\n";
		    $bad_pair=1;
		}		    
	    }
	    elsif ($msgstr =~ /\Q$close_ch\E/)
	    {
		print "# >>> Unmatched pair(s): $open_ch $close_ch <<<\n";
		$bad_pair=1;
	    }
	}

	$unmatched=1 if $bad_pair;

	if ($msgid =~ /^Choose language$/ && $msgstr !~ /\/[ ]*Choose language$/)
	{
	    print "# >>> Missing \"/Choose language\" in the msgstr <<<\n";		
	    $unmatched=1;	    
	}

	if ($msgid =~ /^US\[/ && $msgstr !~ /^[A-Z][A-Z]$/)
	{
	    print "# >>> Wrong country code <<<\n";		
	    $unmatched=1;
	}

	if ($msgid =~ /^-- / && $msgstr !~ /^-- /)
	{
	    print "# >>> String should start with \"-- \" <<<\n";		
	    $unmatched=1;
	}

	if ($msgid =~ / --$/ && $msgstr !~ / --$/)
	{
	    print "# >>> String should end with \" --\" <<<\n";		
	    $unmatched=1;
	}

	if ($_ =~ /#. Do not translate "\/cdrom\/"/ && $msgid =~ /cdrom/ && $msgstr !~ /cdrom/)
	{
	    print "# >>> \"cdrom\" should not be translated <<<\n";		
	    $unmatched=1;
	}

	if ($_ =~ /#.\s+Type:\s+select/ && $_ =~ /#.\s+Choices/ )
	    {
		while ($msgid =~ /,/g) { $count_id++ };
		if ($count_id > 0)
		{
		    while ($msgstr =~ /,/g) { $count_st++ };
		    if ($count_id != $count_st)
		    {
			print "# >>> Wrong number of choices in the list <<<\n";		
			$unmatched=1;			
		    }
		}
	    }

	    my @gi_only = ("bn", "dz", "ka", "gu",
			   "hi", "kn", "km", "lo",
			   "ml", "mr", "ne", "pa",
			   "sa", "ta", "te", "th");
	    my $lang=$ARGV[1];

	    # check length of the msgstr only if language is supported by newt frontend
	    if(!grep $_ eq $lang, @gi_only)
            {

		if ($_ =~ m/#. Translators, this is a menu choice. MUST BE UNDER 65 COLUMNS/ )
		    {
			utf8::decode($msgstr);
			  my $lung = length $msgstr;
			  if ($lung > 65)
			  {
			      print "# >>> Line is too long: keep it under 65 characters <<<\n";		
			      $unmatched=1;			
			  }
		     }
	    }
    }

    return 0 if $unmatched;
    return 1;
}

$/ = "\n\n";
open (PO, "< $ARGV[0]") or die "Unable to open $ARGV[0]: $!\n";
while (<PO>)
{
        s/"\n"//g;
        next if m/^#, .*fuzzy/m;
        next unless m/^msgid ".+"$/m;
        my @msgs = ();
        while (m/^msg\S+ "(.+)"$/mg)
        {
                push (@msgs, $1);
        }
        checkSpecials(@msgs) || print;
}
close (PO);
