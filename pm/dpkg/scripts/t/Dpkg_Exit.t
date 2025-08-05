#!/usr/bin/perl
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

use strict;
use warnings;

use Test::More tests => 7;

BEGIN {
    use_ok('Dpkg::Exit');
}

my $track = 0;

sub test_handler {
    $track++;
}

Dpkg::Exit::run_exit_handlers();

is($track, 0, 'no handlers run');

Dpkg::Exit::push_exit_handler(\&test_handler);
Dpkg::Exit::pop_exit_handler();

Dpkg::Exit::run_exit_handlers();

is($track, 0, 'push/pop; no handlers run');

Dpkg::Exit::push_exit_handler(\&test_handler);

Dpkg::Exit::run_exit_handlers();

is($track, 1, 'push; handler run');

# Check the exit handlers, must be the last thing done.
sub exit_handler {
    pass('exit handler invoked');
    exit 0;
}

sub ini_handler {
    pass('ini handler invoked');
}

sub end_handler {
    pass('end handler invoked');
}

Dpkg::Exit::push_exit_handler(\&end_handler);
Dpkg::Exit::push_exit_handler(\&exit_handler);
Dpkg::Exit::push_exit_handler(\&ini_handler);

kill 'INT', $$;
