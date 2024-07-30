#!/bin/bash

file=$1
sort -r $1.next.ref | grep -v "^3" | grep -v "^2[1-9].*" | grep -v "^20[2-9].*" | grep -v "^201[1-9].*"> $1.prev.ref
sed 's/T.*$//' $1.next.ref | uniq | grep -v "^3" | grep -v "^2[1-9].*" | grep -v "^20[2-9].*" | grep -v "^201[1-9].*" > $1.recurson.ref

diff $file.prev.ref     $file.prev.out     > /dev/null 2>&1  && rm $file.prev.out;
diff $file.next.ref     $file.next.out     > /dev/null 2>&1  && rm $file.next.out;
diff $file.recurson.ref $file.recurson.out > /dev/null 2>&1  && rm $file.recurson.out;


# cat $i |grep -v "^2[1-9].*" | grep -v "^20[2-9].*" | grep -v "^201[1-9].*" > $i.new;
