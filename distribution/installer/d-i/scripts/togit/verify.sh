#!/bin/sh

set -eu

SVNCACHE=file://$PWD/svn
RESULTDIR=verify.out
XFAIL=

. ./verify.conf

init() {
	[ ! -d svn ] && init_svn
	[ ! -d git ] && init_git
}

init_git() {
	[ -e mrconfig ] || make_mrconfig mrconfig $GIT
	mr -c mrconfig checkout
}

init_svn() {
	rsync -rv $SVN/ svn
}

fail() {
	local failure=$1
	local package=$2
	local tag=$3
	local X=

	if echo "$XFAIL" | grep -q "^$failure $package $tag\$"; then
		failure="X${failure}"
	fi

	echo "$failure $package $tag"
	return 1
}

getpackages() {
	(getpackages_git; getpackages_svn) | sort -u
}

getpackages_git() {
	getrepomap git | awk '{ print $2 }'
}

getpackages_svn() {
	getrepomap svn | awk '{ print $2 }'
}

gettags() {
	(gettags_svn $1; gettags_git $1) | sort -u
}

gettags_git() {
	git --git-dir=$(getgitpath $1)/.git tag -l
}

gettags_svn() {
	# Take tags from SVN tags directory
	svn ls "$(getsvnpath $1)" | sed -e 's,/$,,' | sort -g

	# Include trunk
	echo trunk
}

getrepomap() {
	set +u
	(
		if [ "$2" ]; then
			egrep "^$1 $2 "
		elif [ "$1" ]; then
			egrep "^$1 "
		else
			cat
		fi
	) < repomap
	set -u
}

getpath() {
	getrepomap "$@" | awk '{ print $3 }'
}

getsvnpath() {
	local package=$1
	local tag=
	local path
	
	path=$(getpath svn "$@")

	set +u
	tag=$2
	if [ -z "$tag" ]; then
		echo "$SVNCACHE/tags/$path"
	elif [ "$tag" == trunk ]; then
		echo "$SVNCACHE/trunk/$path"
	else
		echo "$SVNCACHE/tags/$path/$tag"
	fi
	set -u
}

getgitpath() {
	local path=$(getpath git "$@")
	[ "$path" ] && echo "git/$path"
}

svntag2gittag() {
	# trunk is special
	if [ "$1" = trunk ]; then
		echo master
	else
		# Drop epoch, replace ~ with .
		echo $1 | sed -e 's/^[0-9]*://' -e 's/~/./g'
	fi
}

#
# Fix keyword expansion; seems there is no cleaner way
# short of deleting the svn:keywords property.
#
svn_undo_keyword_expansion() {
	local svnpath=$1
	local wc=$2

	local IFS="
"

	for line in $(svn propget -R svn:keywords $svnpath); do
		unset IFS
		set -- $line
		local file=$1
		shift; shift

		file="$wc/${file#$svnpath}"

		if [ ! -f "$file" ]; then
			echo WARNING $file does not exist
			continue
		fi

		for keyword in $@; do
			[ "$keyword" = "id" ] && keyword=Id
			sed -i "s/\\\$$keyword:[^$]*\\\$/\\\$$keyword\\$/" "$file"
		done

	done
	unset IFS
}

checkout_svn() {
	local svnpath=$1
	local wc=$2

	# Checkout tag
	svn export -q $svnpath $wc || return 1

	svn_undo_keyword_expansion $svnpath $wc
}

checkout_git() {
	local gitpath=$1
	local tag=$2
	local wc=$3

	mkdir -p $wc
	git --git-dir=$gitpath/.git --work-tree=$wc checkout -q -f $tag
}

compare_checkouts() {
	diff --recursive --new-file --unified --exclude=.svn --exclude=.git $1 $2
}

compare_tag() {
	local package=$1
	local tag=$2
	local outdir tmpdir local svnpath gitpath
	local fail=0

	tmpdir="$WCTEMPDIR/$package/$tag"
	svnpath=$(getsvnpath $package $tag)
	gitpath=$(getgitpath $package)

	local gitid=$(git --git-dir=$gitpath/.git describe --always $(svntag2gittag $tag))
	local svnrev=$(LANG=C svn info $svnpath 2>/dev/null | sed -n -e '/Last Changed Rev: /s/.*: //p')

	echo "comparing $package $tag (@${svnrev}/${gitid})"

	if [ -z "$svnpath" ]; then
		fail LOOKUP $package $tag || fail=1
	elif ! checkout_svn $svnpath $tmpdir/svn; then
		fail SVNFAIL $package $tag || fail=1
	fi

	if [ -z "$gitpath" ]; then
		fail LOOKUP $package $tag || fail=1
	elif ! checkout_git "$gitpath" "$(svntag2gittag $tag)" "$tmpdir/git"; then
		fail GITFAIL $package $tag || fail=1
	fi

	if [ "$fail" -eq 0 ]; then
		local outdir="$RESULTDIR/${package}"
		mkdir -p $outdir

		if ! compare_checkouts $tmpdir/svn $tmpdir/git > $outdir/${tag}; then
			fail MISMATCH $package $tag || fail=1
		fi
	fi

	# Keep?
	rm -rf $tmpdir
}

compare_package() {
	local package=$1
	local ret=0

	for tag in $(gettags_svn $package); do
		compare_tag $package $tag
	done
}

compare_all() {
	local package tag
	local ret=0

	for package in $(getpackages_svn); do
		compare_package $package
	done
}

compare_lists() {
	# Build package lists
	getpackages_svn | sort -n > $RESULTDIR/packages.svn
	getpackages_git | sort -n > $RESULTDIR/packages.git

	# Build tag lists.
	for p in $(getpackages_svn); do
		# Changes to svn tags:
		# - strip epoch to match git tags
		# - drop trunk
		gettags_svn $p | sed -e 's/^[0-9]://' -e 's/~/./g' | grep -v '^trunk$' | sed -e "s/^/$p /"
	done | sort -n > $RESULTDIR/tags.svn

	for p in $(getpackages_git); do
		gettags_git $p | sed -e "s/^/$p /"
	done | sort -n > $RESULTDIR/tags.git

	# Compare
	diff -u $RESULTDIR/tags.svn $RESULTDIR/tags.git > $RESULTDIR/tags.diff || :
	diff -u $RESULTDIR/packages.svn $RESULTDIR/packages.git > $RESULTDIR/packages.diff || :

	# Cleanup
	rm $RESULTDIR/tags.svn $RESULTDIR/tags.git
	rm $RESULTDIR/packages.svn $RESULTDIR/packages.git
}

checkresults() {
	local total=$(grep '^comparing' $RESULTDIR/report | wc -l)
	local gitfail=$(grep '^GITFAIL' $RESULTDIR/report | wc -l)
	local svnfail=$(grep '^SVNFAIL' $RESULTDIR/report | wc -l)
	local mismatch=$(grep '^MISMATCH' $RESULTDIR/report | wc -l)
	local xfail=$(egrep '^X[A-Z]+' $RESULTDIR/report | wc -l)

	echo STATS total:$total gitfail:$gitfail svnfail:$svnfail mismatch:$mismatch xfail:$xfail

	if [ "${gitfail}/${svnfail}/${mismatch}" != "0/0/0" ]; then
		echo "FAILED"
		return 1
	fi

	echo "PASSED"
	return 0
}

stripresults() {
	# Drop all empty diffs
	find $RESULTDIR -type f -size 0 | xargs --no-run-if-empty rm

	# Drop all empty dirs
	find $RESULTDIR -type d -empty | xargs --no-run-if-empty rmdir
}

make_mrconfig() {
	local config=$1
	local source=$2

	:>$config

	# List unique repos
	for path in $(getrepomap git | awk '{ print $3 }' | sort -n | uniq); do
		mr -c $config config git/${path%.git} checkout="git clone $source/${path%.git}"
	done
}

groupdiffs() {
	# Group based on actual changes in the diff
	mkdir -p $RESULTDIR/groups

	for x in $(find $RESULTDIR -type f -size +0); do
		sum=$(egrep '^[+-]' $x | egrep -v '^[+-]{3} '| md5sum | cut -d' ' -f1)
		echo $x >> $RESULTDIR/groups/$sum
	done

	# Drop groups with only a single member
	for x in $(find $RESULTDIR/groups -type f); do
		if [ "$(wc -l < $x)" -eq 1 ]; then
			rm $x
		fi
	done
}

__compare() {
	local f=$1
	shift
	$f "$@" 2>&1 | tee $RESULTDIR/report
	stripresults
	checkresults | tee -a $RESULTDIR/report
}

all() {
	__compare compare_all "$@"
	compare_lists

	mkdir -p $RESULTDIR/conf
	[ -f verify.conf ] && cp verify.conf $RESULTDIR/conf
	[ -f repomap ] && cp repomap $RESULTDIR/conf
	[ -f mrconfig ] && cp mrconfig $RESULTDIR/conf
}

tag() {
	__compare compare_tag "$@"
}

package() {
	__compare compare_package "$@"
}

if [ -d $RESULTDIR ]; then
	echo $RESULTDIR exists, exiting
	exit 1
fi

mkdir -p $RESULTDIR

LANG=C eval "$@"

[ -d "$WCTEMPDIR" ] && rm -rf "$WCTEMPDIR"
