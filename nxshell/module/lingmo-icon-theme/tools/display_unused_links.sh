#!/bin/sh
for f in $(find $1 -type l);do [ ! -e $f ] && echo `ls -l $f`;done

# Delete the unused links
#for f in $(find $1 -type l);do [ ! -e $f ] && rm -f $f;done

