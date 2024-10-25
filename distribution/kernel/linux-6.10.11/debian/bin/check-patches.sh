#!/bin/sh -e

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT
for patchdir in debian/patches*; do
    sed '/^#/d; /^[[:space:]]*$/d; /^X /d; s/^+ //; s,^,'"$patchdir"'/,' "$patchdir"/series
done | sort -u > $TMPDIR/used
find debian/patches* ! -path '*/series' -type f -name "*.diff" -o -name "*.patch" -printf "%p\n" | sort > $TMPDIR/avail
echo "Used patches"
echo "=============="
cat $TMPDIR/used
echo
echo "Unused patches"
echo "=============="
grep -F -v -f $TMPDIR/used $TMPDIR/avail || test $? = 1
echo
echo "Patches without required headers"
echo "================================"
xargs grep -E -l '^(Subject|Description):' < $TMPDIR/used | xargs grep -E -l '^(From|Author|Origin):' > $TMPDIR/goodheaders || test $? = 1
grep -F -v -f $TMPDIR/goodheaders $TMPDIR/used || test $? = 1
echo
echo "Patches without Origin or Forwarded header"
echo "=========================================="
xargs grep -E -L '^(Origin:|Forwarded: (no\b|not-needed|http))' < $TMPDIR/used || test $? = 1
echo
echo "Patches to be forwarded"
echo "======================="
xargs grep -E -l '^Forwarded: no\b' < $TMPDIR/used || test $? = 1
