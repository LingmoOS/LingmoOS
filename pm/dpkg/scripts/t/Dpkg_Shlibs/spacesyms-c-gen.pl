#!/usr/bin/perl
#
# spacesyms-c-gen.pl
#
# Output a C file that contains symbols matching the shell glob
# sym{defaultver,longver,shortver}{nospace,SPACE}{default,hidden,protected,internal}
# with symbol visibility matching the final element and at least one relocation
# against each symbol.
#
# When used together with spacesyms-o-map.pl and spacesyms.map, makes a shared
# object that contains symbols that covers all cases of:
#
# 1) has a short, long or Base version,
# 2) has or does not have a space in the symbol name,
# 3) default, hidden, protected or internal visibility.

use strict;
use warnings;

my @symbols;

foreach my $version (qw(defaultver longver shortver)) {
    foreach my $space (qw(nospace SPACE)) {
        foreach my $visibility (qw(default hidden protected internal)) {
            my $symbol = "sym$version$space$visibility";
            push @symbols, $symbol;
            print "void $symbol(void) __attribute__((visibility(\"$visibility\")));\n";
            print "void $symbol(void) {}\n";
        }
    }
}

print "extern void (*funcs[])(void);\n";
print "void (*funcs[])(void) = {\n";
foreach my $symbol (@symbols) {
    print "$symbol,\n";
}
print "};\n";
