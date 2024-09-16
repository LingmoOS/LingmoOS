#!/bin/sh
# Pulls ignores out of svn, and applies them to a git repo.
# No actual conversion between syntaxes is done, the svn ignores
# used in d-i are probably close enough that they will work
# when put in git.
# The .gitignore files are stages, but not committed.

set -e

di_svn=$1
di_git=$2

if [ ! -d "$di_svn" ] || [ ! -d "$di_git" ]; then
	echo "usage: $0 di_svn di_git" >&2
	exit 1
fi

# empty dir, so not in git
mkdir -p $di_git/installer/build/localudebs

(cd $di_svn && svn propget svn:ignore -R) |
(cd $di_git && perl -e '
	while (<>) {
		chomp;
		if (/^(.*) - (.*)/) {
			collect();
			$dir=$1;
			$_=$2;
		}
		$c.=$_."\n";
	}
	collect();
	sub collect {
		return unless length $c && length $dir;
		return if -d "$dir/.svn"; # some dirs are still in svn
		system("mkdir -p $dir");
		open(OUT, ">$dir/.gitignore") || die "$dir/.gitignore: $!";
		print OUT $c;
		close OUT;
		$c="";
		#system("cd $dir; git add -f .gitignore");
	}
')
