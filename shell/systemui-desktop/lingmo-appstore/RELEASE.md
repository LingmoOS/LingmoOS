LINGMO Appstore Release Notes
===

Release schedule
---

LINGMO Appstore releases are done on the timetable set by the [LINGMO release schedule](https://release.lingmo.org/calendar/).

Maintainers take it in turns to make releases so that the load is spread out evenly.

Making a release
---

Adapted from the [LINGMO release process](https://handbook.lingmo.org/maintainers/making-a-release.html).

These instructions use the following variables:
 - `new_version`: the version number of the release you are making, for example 3.38.1
 - `previous_version`: the version number of the most-recently released version in the same release series, for example 3.38.0
 - `branch`: the branch which the release is based on, for example lingmo-40 or main
 - `key_id`: the ID of your GPG key, see the output of `gpg --list-keys` and the note at the end of this file

Go to https://github.com/LingmoOS/lingmo-appstore/-/milestones/ and
choose the corresponding milestone. Verify all issues and merge requests
tagged for this release are complete now. Move those which not to the next
milestone or merge pending fixes when possible.

Make sure your repository is up to date and doesn’t contain local changes:
```
git pull
git status
```

Check the version in `meson.build` is correct for this release.

Download
[gitlab-changelog](https://github.com/LingmoOS/pwithnall/gitlab-changelog) and use
it to write release entries:
```
gitlab-changelog.py LINGMO/lingmo-appstore ${previous_version}..
```

Edit this down to just the user visible changes, and list them in
`data/metainfo/org.lingmo.Appstore.metainfo.xml.in`. User visible changes are ones
which the average user might be interested to know about, such as a fix for an
impactful bug, a UI change, or a feature change.

You can get review of your metainfo changes from other co-maintainers if you wish.

Generate `NEWS` file:
```
appstreamcli metainfo-to-news ./data/metainfo/org.lingmo.Appstore.metainfo.xml.in ./NEWS
```

Commit the release:
```
git add -p
git commit -S -m "Release version ${new_version}"
```

Build the release tarball:
```
# Only execute git clean if you don't have anything not tracked by git that you
# want to keep
git clean -dfx
meson setup --prefix $PWD/install -Dbuildtype=release build/
ninja -C build/ dist
```

Tag, sign and push the release (see below for information about `git evtag`):
```
git evtag sign -u ${key_id} ${new_version}
git push --atomic origin ${branch} ${new_version}
```
To use a specific key add an option `-u ${keyid|email}` after the `sign` argument.

Use `Tag ${new_version} release` as the tag message.

Post release version bump in `meson.build`:
```
# edit meson.build, then
git commit -a -m "trivial: Post release version bump"
git push
```

Upload the release tarball:
```
scp build/meson-dist/lingmo-appstore-${new_version}.tar.xz master.lingmo.org:
ssh master.lingmo.org ftpadmin install lingmo-appstore-${new_version}.tar.xz
```

Add the release notes to GitLab and close the milestone:
 - Go to https://github.com/LingmoOS/lingmo-appstore/-/releases/new?tag_name=${new_version}
   - set `Release title` to `${new_version}`
   - set the Milestone of the release, if such exists
   - copy the Release notes for the new release from the `NEWS` file
     (replace `~~~~~~~~~~~~` with `===` (only three `=`))
   - in the Links section add:
     | URL | Link title | Type |
     | ------ | ------ | ------ |
     | `https://download.lingmo.org/sources/lingmo-appstore/${new_major_version}/lingmo-appstore-${new_version}.tar.xz` | Release tarball | Other |
     | `https://download.lingmo.org/sources/lingmo-appstore/${new_major_version}/lingmo-appstore-${new_version}.sha256sum` | Release tarball sha256sum | Other |
   - save the changes with `Create release` button
   - verify the added links for the release artifacts work
 - Go to https://github.com/LingmoOS/lingmo-appstore/-/milestones/
   choose the milestone and close it

`git-evtag`
---

Releases should be done with `git evtag` rather than `git tag`, as it provides
stronger security guarantees. See
[its documentation](https://github.com/cgwalters/git-evtag) for more details.
In particular, it calculates its checksum over all blobs reachable from the tag,
including submodules; and uses a stronger checksum than SHA-1.

You will need a GPG key for this, ideally which has been signed by others so
that it can be verified as being yours. However, even if your GPG key is
unsigned, using `git evtag` is still beneficial over using `git tag`.
