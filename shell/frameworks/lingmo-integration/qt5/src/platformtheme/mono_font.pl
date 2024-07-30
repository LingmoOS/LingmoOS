#! /usr/bin/perl

use strict;

while (<>)
{
    chomp;
    s/fixed=Oxygen\ Mono/fixed=Hack/;
    print "$_\n";
}
