#!/usr/bin/perl
#
# spacesyms-o-map.pl INPUT OUTPUT
#
# Copy the object file INPUT to OUTPUT, redefining any symbol in INPUT that
# contains "SPACE" in its name to contain "SPA CE" instead.

use strict;
use warnings;

my ($input, $output) = @ARGV;
my @cmds = ('objcopy');

open my $nm, '-|', 'nm', $input or die "cannot run nm: $!";
while (<$nm>) {
    next if not m/SPACE/;
    chomp;
    my $x = (split / /, $_, 3)[2];
    my $y = $x =~ s/SPACE/SPA CE/r;
    push @cmds, "--redefine-sym=$x=$y";
}
close $nm;

push @cmds, $input, $output;
exec { $cmds[0] } @cmds;
