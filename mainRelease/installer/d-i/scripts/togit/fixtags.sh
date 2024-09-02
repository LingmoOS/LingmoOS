#!/bin/sh
# This fixes up broken tags that are not connected to anything. 
# The converter produced a lot of those for various reasons.
# Any such tag that pointed to a tree that *is* in a commit connected
# to master is replaced by a new, dummied up version. The original tagger,
# tag date, and message are preserved.

# Run with caution!

branches=$(git branch -r| awk '{print $1}')

for tag in $(git tag); do if git show --pretty=raw --raw $tag | grep -q '^parent'; then : ; else
	tree=$(git show --pretty=raw --raw $tag | grep '^tree ' | awk '{print $2}')
	commit=$(git log --pretty="format:%T %H" $branches | grep "^$tree " | awk '{print $2}'| head -n 1)
	if [ -z "$commit" ]; then
		echo "cannot fix disconnected tag $tag in (tried branches $branches)"
	else
		name="$(git log --pretty="format:%cn" $commit^..$commit)"
		email="$(git log --pretty="format:%ce" $commit^..$commit)"
		date="$(git log --pretty="format:%cd" $commit^..$commit)"
		message="$(git log --pretty="format:%B" $commit^..$commit)"
		git diff $tag..$commit > test
		if [ -s /tmp/test ]; then
			echo "$tag is $tree , but $commit somehow differs!"
		else
			echo "using $commit for $tag , committer: $name , date: $date"
			GIT_COMMITTER_DATE="$date" GIT_COMMITTER_NAME="$name" GIT_COMMITTER_EMAIL="$email" git tag -f -m "$message" -a $tag $commit
		fi
		rm -f /tmp/test
	fi
fi; done

git push --tags --force
