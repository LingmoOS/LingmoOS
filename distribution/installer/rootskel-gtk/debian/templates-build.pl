#!/usr/bin/perl
use strict;
use warnings;

die "must specify arch" if not defined $ARGV[0];
my $arch = $ARGV[0];

my %template;
$template{Fields} = [];
$template{'Description-Long'} = "";

sub print_template {
	foreach ( @{$template{Fields}} ) {
		print $_ . ": ";
		if ( ref $template{$_} eq "HASH" ) {
			if ( defined $template{$_}->{$arch} ) {
				print $template{$_}->{$arch};
			} else {
				print $template{$_}->{default};
			}
		} else {
			print $template{$_};
		}
		print "\n";
	}
	print $template{'Description-Long'} . "\n";

	%template = ();
	$template{Fields} = [];
	$template{'Description-Long'} = "";
}

while ( <STDIN> ) {
	chomp;
	if (m/^$/) {
	    print_template;
	} elsif ( m/^(\w+)(\[(\w+)\])?:\s+(.*)\s*$/ ) {
		if ( defined $3 ) {
			if ( defined $template{$1} and ref $template{$1} ne "HASH" ) {
				local $_;
				$_ = $template{$1};
				$template{$1} = ();
				$template{$1}->{default} = $_;
			} elsif ( not defined $template{$1} ) {
				push ( @{$template{Fields}}, $1 );
			}
			$template{$1}->{$3} = $4;
		} else {
			$template{$1} = $4;
			push ( @{$template{Fields}}, $1 );
		}
	} else {
		$template{'Description-Long'} .= $_ . "\n";
	}
}

print_template;

