# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com)
and this project adheres to [Semantic Versioning](https://semver.org).

## [Unreleased]

## [1.10.6] - 2024-05-10

### Changed

- Vim: Allow "x86_64_v3" for $arch (!47) (Frederik “Freso” S. Olesen)

### Fixed

- Vim: Fix the Zlib license SPDX identifier (!48) (Robin Candau)
- pacscripts: Revert 'printf' lint (!49) (Justin Gassner)

## [1.10.5] - 2024-03-16

### Changed

- Vim: Adapt to the recent switch to SPDX identifiers for the license array (!46) (Robin Candau)
- pacdiff: Be compatible with plocate (!45) (Tilman Blumenbach)

## [1.10.4] - 2024-01-29

### Fixed

- checkupdates: Remove color code from the "updates" array when using the --download option (!44) (Robin Candau)
- checkupdates: Add zsh completion for --change (27b854fe) (Daniel M. Capella)

## [1.10.3] - 2024-01-27

### Fixed

- pacscripts: Fix glob (!43) (Gesh)

## [1.10.2] - 2024-01-25

### Added

- checkupdates: Preserve lpm colors when printing the list of pending updates. (!42) (Robin Candau)

## [1.10.1] - 2024-01-03

### Fixed

- paccache: Fix regression when using multiple cache directories. (df2554b7) (Harald)

## [1.10.0] - 2024-01-02

### Added

- rankmirrors: Add --working-only option to only output working mirrors. (!37) (Victor Westerhuis)
- checkupdates: Add the -c/--change option to print only when available updates differ from the last --change run. (!40) (Robin Candau)

### Fixed

- Create pipelines for and satisfy Cppcheck, markdownlint-cli2, ShellCheck, and Vint. (!32, !33, !34, !35, 41554d65, d674bf23) (Matthew Armand, Robin Candau, Daniel M. Capella)
- rankmirrors: Fix word-splitting. (!36) (Victor Westerhuis)

## [1.9.1] - 2023-07-13

### Fixed

- pacdiff: Fix the .bak workflow being broken for /etc/sudoers. (!31) (Matt Armand)

## [1.9.0] - 2023-04-03

### Added

- Vim: recognise comments inside array groups. (!29) (éclairevoyant)
- Add the missing man pages as well as the missing utils from the README. (!30) (Robin Candau)

## [1.8.2] - 2023-01-04

### Fixed

- Fix pacdiff with multiple CacheDirs. (!27) (Baltazár Radics)

## [1.8.0] - 2022-11-01

### Added

- Add SRCINFO syntax for Vim. (!23) (ObserverOfTime)

### Fixed

- rankmirrors: Fix zsh completion & consistency touch-up. (!25) (Frederick Zhang)
- Vim: build.lpm syntax fixes. (!24) (ObserverOfTime)

## [1.7.1] - 2022-09-07

### Fixed

- updpkgsums: fix when build.lpm contains \*sums+=. (!22) (Piggy NL)

## [1.7.0] - 2022-09-07

### Added

- rankmirrors: Allow parallel time check. (6614285b) (Dmitry Kuzmenko)

### Fixed

- Consistency fixes. (!20) (Daniel M. Capella)

## [1.6.0] - 2022-06-30

### Added

- zsh completions. (!14) (Arvid Norlander)
- pacdiff: Add backup option to save old files (.bak) when (O)verwriting. (!17) (Matthew Armand)
- pacdiff: Add -3/--threeway option to view diffs in 3-way fashion. (!18) (Ehsan Ghorbannezhad)

### Changed

- pacdiff: clarify -s/--sudo docs to indicate use of sudoedit as well as sudo. (!16) (Matt Armand)

## [1.5.3] - 2022-06-09

### Changed

- pacdiff: stop if the base file extraction fails. (a4a32b5f) (Evangelos Foutras)

### Fixed

- pacdiff: fix base package filtering expression. (bb26f141) (Evangelos Foutras)

## [1.5.2] - 2022-05-21

### Added

- Vim: Add ISC and OFL as special licenses. (!11) (Daniel M. Capella)
- Add continuous testing. (bbdf959f, b05ed44f, a0b1d8e1) (Daniel M. Capella)

### Changed

- Switch to EditorConfig from Vim modelines. (!9) (Daniel M. Capella)

### Fixed

- Remove PrivateUsers=yes from paccache.service. (!13) (David Runge)

## [1.5.0] - 2022-05-03

### Added

- pacdiff: automatically delete pacfile after viewing if identical. (3528b32c) (John A. Leuenhagen)
- pacdiff: Learn the (M)erge mode. (94b2a194) (Denton Liu)
- pacdiff: Add option to use sudo/sudoedit to manage files. (19ab4fac) (Daniel Parks)
- paccache.service.in: Harden unit. (59fd4efb) (Frederik “Freso” S. Olesen)
- lpm-filesdb: systemd service and timer for `lpm -Fy`. (dff74498, 871ffc94) (Thiago Perrotta)
- checkupdates: Provide --nosync option. (!2) (Samir Benmendil)

### Changed

- build.lpm.vim improvements. (!5) (Jelle van der Waa)

### Fixed

- updpkgsums: don't try to add nonexistent checksums. (80275d21) (Eli Schwartz)

## [1.4.0] - 2020-07-28

### Added

- pactree: Add --debug. (2d13a236) (Edward E)
- pactree: Add --gpgdir, set the gpg directory. (a4e69cac) (Edward E)
- pactree: introduce the --optional flag. (FS#61336) (b6d08b40) (Will Song)

### Changed

- doc: make timestamps in man pages reproducible. (e2a5e43b) (Jonas Witschel)
- paccache.service.in: Reword description to more clearly specify what it does. (269a2cdc) (Johannes Löthberg)

### Fixed

- pactree: Improve command line validation. (FS#64589) (e77e13b0) (Sebastian Jakubiak)
- pactree: Fix redundant arrows in Graphviz output. (c7be8631) (Sebastian Jakubiak)
- paccache: Support cleaning many thousands of candidates. (547a66d5) (Leonid Bloch)

## [1.3.0] - 2019-12-25

### Added

- Add document describing the release procedures. (21449e3) (Johannes Löthberg)
- vim: Add Unlicense as valid license. (f835547) (Daniel M. Capella)

### Changed

- checkupdates: Use $UID instead of $USER in the tempdir path. (a8b342e) (nl6720)
- checkupdates, paccache, and pacdiff were ported to libmakepkg. (431e564) (Eli Schwartz)

### Fixed

- checkupdates: Exit with 2 if there are no updates available. (3da550e) (Eli Schwartz)
- rankmirrors: Fix parsing of -m argument. (d026415) (Johannes Löthberg)
- updpkgsums: Use makepkg's checksum algorithm type specification, fixing support for b2sums. (c8ef727) (Eli Schwartz)
- vim: Recognize validpgpkeys variable. (e6950d3) (Jelle van der Waa)

## [1.2.0] - 2019-10-06

### Added

- checkupdates: Add option for downloading updates. (ab69666)
- docs: Add manpages for checkupdates, pacdiff, pacsort, updpkgsums. (d25a8b2, 35eef6b, ef63784, b258c31) (Jelle van der Waa, Johannes Löthberg)
- paccache: Add --age-atime and --age-mtime arguments. (45c1916) (wisp3rwind)
- vim: Add indent file. (020b533) (Daniel M. Capella)
- vim: Add b2sums to build.lpm syntax file. (fc21909) (Eli Schwartz)
- Vim: Add Boost and MPL2 as valid licenses. (b5cb1ce) (Daniel M. Capella)

### Changed

- paclist: Allow listing packages from multiple repos at once. (58bfa06) (Lars Rustand)
- paclist: Also list packages where the installed version differ from the repo version. (4849211) (Michael Straube)
- vim: Remove version check for Vim older than 6.0 from build.lpm syntax file. (b4ae0e5) (Daniel M. Capella)
- checkupdates, paccache, pacdiff: Don't use colors on a dumb terminal. (c57d275) (Ivy Foster)

### Fixed

- pacccache: Fix parsing of --move argument. (9ebae18) (Self-Perfection)
- pacdiff: Don't assume the DBPath has a trailing slash. (0c260d3) (Eli Schwartz)
- pacsort: Support all compression formats supported by `makepkg`. (aa22e5c) (Johannes Löthberg)
- vim: Add `unknown` license special case to build.lpm syntax file. (6193d12) (Daniel M. Capella)

## [1.1.0] - 2018-08-05

### Added

- paccache: Add manpage. (1603b07c) (Michael Straube)
- rankmirrors: Add --max-time flag to specify the timeout used. (2197352a) (Sami Kerola)

### Changed

- pacscripts: Don't log to lpm.log when files are downloaded to the cache. (a5a3045f) (Eli Schwartz)
- pacscripts: Find package file path using lpm instead of doing it manually ourselves. (823ceb0f) (Eli Schwartz)
- pactree: Use full dependency string when finding dependencies, since if we don't use the version requirement part of the string, we will sometimes return the wrong results. (60313f75) (Johannes Löthberg)
- vim/ftplugin/build.lpm: Set vim 'commentstring' option. (b1d5d3a8) (Antony Lee)
- vim/syntax/build.lpm: Add sha224sums support. (0439723c) (morganamilo)

### Fixed

- pacsort: Fix short version option. (5db0b1ee) (Michael Straube)

## [1.0.0] - 2018-05-28

### Added

- `--version` option to pactree. (17beb345) (Johannes Löthberg)
- Import pacsort utility. (f3084926) (Daniel M. Capella)
- systemd service and timer for paccache. (769a3db6) (Isaac Good)

### Changed

- Make pacscripts use `-v` as the short flag for version output instead of `-V`, in accordance with its documentation. (a970b3aa) (Johannes Löthberg)

### Fixed

- checkupdates now reports failures to update the databases. (b61b9b25) (Andrew Barchuk)
- Add `-v` to pacsort help output. (40a55a41) (Michael Straube)

## [0.0.1] - 2016-10-17

### Added

- Import the following utilities (Johannes Löthberg):
  - checkupdates (db366b32)
  - paccache (bf8826e4)
  - pacdiff (9948ec2b)
  - paclist (7d91b1d7)
  - paclog-pkglist (e3066b47)
  - pacscripts (723c1599)
  - pacsearch (95ba86e3)
  - pactree (1d166e4a)
  - rankmirrors (1ccc6854)
  - updpkgsums (bda662e1)
- Add vim highlighting for build.lpms. (43ae503e) (Johannes Löthberg)

[0.0.1]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/commits/v0.0.1
[1.0.0]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v0.0.1...v1.0.0
[1.1.0]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.0.0...v1.1.0
[1.2.0]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.1.0...v1.2.0
[1.3.0]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.2.0...v1.3.0
[1.4.0]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.3.0...v1.4.0
[1.5.0]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.4.0...v1.5.0
[1.5.2]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.5.0...v1.5.2
[1.5.3]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.5.2...v1.5.3
[1.6.0]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.5.3...v1.6.0
[1.7.0]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.6.0...v1.7.0
[1.7.1]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.7.0...v1.7.1
[1.8.0]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.7.1...v1.8.0
[1.8.2]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.8.0...v1.8.2
[1.9.0]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.8.2...v1.9.0
[1.9.1]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.9.0...v1.9.1
[1.10.0]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.9.1...v1.10.0
[1.10.1]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.10.0...v1.10.1
[1.10.2]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.10.1...v1.10.2
[1.10.3]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.10.2...v1.10.3
[1.10.4]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.10.3...v1.10.4
[1.10.5]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.10.4...v1.10.5
[1.10.5]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.10.5...v1.10.6
[unreleased]: https://gitlab.archlinux.org/lpm/lpm-contrib/-/compare/v1.10.6...master

<!-- markdownlint-disable-file MD013 MD024 -->
