#!/bin/sh
for repo in $(find `pwd`/git -type d -name \*.git); do
	cd $repo
	for branch in $(git branch | sed 's/^\** *//'); do
		git log $branch --pretty='format:%an %ae' | grep '@localhost'
	done
done
