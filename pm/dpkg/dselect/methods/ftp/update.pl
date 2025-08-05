#!/usr/bin/perl
#
# Copyright © 1996 Andy Guy <andy@cyteen.org>
# Copyright © 1998 Martin Schulze <joey@infodrom.north.de>
# Copyright © 1999, 2009 Raphaël Hertzog <hertzog@debian.org>
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

eval q{
    use Dpkg; # Dummy import to require the presence of Dpkg::*.
};
if ($@) {
    warn "Missing Dpkg modules required by the FTP access method.\n\n";
    exit 1;
}

use Dselect::Method;
use Dselect::Method::Ftp;

# deal with arguments
my $vardir = $ARGV[0];
my $method = $ARGV[1];
my $option = $ARGV[2];

if ($option eq 'manual') {
    print "Enter package file names or a blank line to finish\n";
    while(1) {
	print 'Enter package file name:';
	my $fn = <STDIN>;
	chomp $fn;
	if ($fn eq '') {
	    exit 0;
	}
	if ( -f $fn ) {
	    system('dpkg', '--merge-avail', $fn);
	} else {
	    print "Could not find $fn, try again\n";
	}
    };
};

#print "vardir: $vardir, method: $method, option: $option\n";

my $arch = qx(dpkg --print-architecture);
$arch = 'i386' if $?;
chomp $arch;
my $exit = 0;

# get info from control file
read_config("$vardir/methods/ftp/vars");

chdir "$vardir/methods/ftp";

print "Getting Packages files...(stop with ^C)\n\n";

my @pkgfiles;
my $ftp;
my $packages_modified = 0;

sub download {
foreach (@{$CONFIG{site}}) {
   my $site = $_;

        $ftp = do_connect(ftpsite => $_->[0],
                          ftpdir => $_->[1],
                          passive => $_->[3],
                          username => $_->[4],
                          password => $_->[5],
                          useproxy => $CONFIG{use_auth_proxy},
                          proxyhost => $CONFIG{proxyhost},
                          proxylogname => $CONFIG{proxylogname},
                          proxypassword => $CONFIG{proxypassword});

    my @dists = @{$_->[2]};
    PACKAGE:
    foreach my $dist (@dists) {
	my $dir = "$dist/binary-$arch";
	my $must_get = 0;
	my $newest_pack_date;

	# check existing Packages on remote site
	print "\nChecking for Packages file... ";
	$newest_pack_date = do_mdtm ($ftp, "$dir/Packages.gz");
	if (defined $newest_pack_date) {
	    print "$dir/Packages.gz\n";
	} else {
	    $dir = "$dist";
	    $newest_pack_date = do_mdtm ($ftp, "$dir/Packages.gz");
	    if (defined $newest_pack_date) {
		print "$dir/Packages.gz\n";
	    } else {
		print "Couldn't find Packages.gz in $dist/binary-$arch or $dist; ignoring.\n";
		print "Your setup is probably wrong, check the distributions directories,\n";
		print "and try with passive mode enabled/disabled (if you use a proxy/firewall)\n";
		next PACKAGE;
	    }
	}

	# we now have $dir set to point to an existing Packages.gz file

	# check if we already have a Packages file (and get its date)
	$dist =~ tr/\//_/;
	my $file = "Packages.$site->[0].$dist";

	# if not
	if (! -f $file) {
	    # must get one
#	    print "No Packages here; must get it.\n";
	    $must_get = 1;
	} else {
	    # else check last modification date
            my @pack_stat = stat($file);
	    if($newest_pack_date > $pack_stat[9]) {
#		print "Packages has changed; must get it.\n";
		$must_get = 1;
	    } elsif ($newest_pack_date < $pack_stat[9]) {
		print " Our file is newer than theirs; skipping.\n";
	    } else {
		print " Already up-to-date; skipping.\n";
	    }
	}

	if ($must_get) {
	    -f 'Packages.gz' and unlink 'Packages.gz';
	    -f 'Packages' and unlink 'Packages';
	    my $size = 0;

	  TRY_GET_PACKAGES:
	    while (1) {
		if ($size) {
		    print ' Continuing ';
		} else {
		    print ' Getting ';
		}
		print "Packages file from $dir...\n";
		eval {
		    if ($ftp->get("$dir/Packages.gz", 'Packages.gz', $size)) {
			if (system('gunzip', 'Packages.gz')) {
			    print "  Couldn't gunzip Packages.gz, stopped";
			    die 'error';
			}
		    } else {
			print "  Couldn't get Packages.gz from $dir !!! Stopped.";
			die 'error';
		    }
		};
		if ($@) {
		    $size = -s 'Packages.gz';
		    if (ref($ftp)) {
		      $ftp->abort();
		      $ftp->quit();
		    };
		    if (yesno ('y', "Transfer failed at $size: retry at once")) {
                         $ftp = do_connect(ftpsite => $site->[0],
                                           ftpdir => $site->[1],
                                           passive => $site->[3],
                                           username => $site->[4],
                                           password => $site->[5],
                                           useproxy => $CONFIG{use_auth_proxy},
                                           proxyhost => $CONFIG{proxyhost},
                                           proxylogname => $CONFIG{proxylogname},
                                           proxypassword => $CONFIG{proxypassword});

			if ($newest_pack_date != do_mdtm ($ftp, "$dir/Packages.gz")) {
			    print ("Packages file has changed !\n");
			    $size = 0;
			}
			next TRY_GET_PACKAGES;
		    } else {
			die 'error';
		    }
		}
		last TRY_GET_PACKAGES;
	    }

	    if (!rename 'Packages', "Packages.$site->[0].$dist") {
		print "  Couldn't rename Packages to Packages.$site->[0].$dist";
		die 'error';
	    } else {
		# set local Packages file to same date as the one it mirrors
		# to allow comparison to work.
		utime $newest_pack_date, $newest_pack_date, "Packages.$site->[0].$dist";
		$packages_modified = 1;
	    }
	}
	push @pkgfiles, "Packages.$site->[0].$dist";
    }
    $ftp->quit();
 }
}

eval {
    local $SIG{INT} = sub {
	die "interrupted!\n";
    };
    download();
};
if($@) {
    $ftp->quit() if (ref($ftp));
    if($@ =~ /timeout/i) {
	print "FTP TIMEOUT\n";
    } else {
	print "FTP ERROR - $@\n";
    }
    $exit = 1;
};

# Don't clear if nothing changed.
if ($packages_modified) {
    print <<'EOM';

It is a good idea to clear the available list of old packages.
However if you have only downloaded a Package files from non-main
distributions you might not want to do this.

EOM
    if (yesno ('y', 'Do you want to clear available list')) {
	print "Clearing...\n";
	if (system('dpkg', '--clear-avail')) {
	    print 'dpkg --clear-avail failed.';
	    die 'error';
	}
    }
}

if (!$packages_modified) {
    print "No Packages files was updated.\n";
} else {
    foreach my $file (@pkgfiles) {
	if (system('dpkg', '--merge-avail', $file)) {
	    print "Dpkg merge available failed on $file";
	    $exit = 1;
	}
    }
}
exit $exit;
