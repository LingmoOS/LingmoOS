# Namcap

Namcap is a lint tool [pacman](https://gitlab.archlinux.org/pacman/pacman) packages that checks binary packages and source PKGBUILDs for common packaging errors.

Namcap’s source is available [on Arch Linux’s GitLab instance](https://gitlab.archlinux.org/pacman/namcap).

# How Can I Help?

There are several ways that you can contribute and help namcap’s development.
You can contribute with code, patches, tests, bugs, and feature requests.

To report a bug or a feature request for namcap, file an issue in the [issue tracker](https://gitlab.archlinux.org/pacman/namcap/-/issues).
If you’re reporting a bug, please give concrete examples of packages where the problem occurs.

Minimal examples (very simple packages forged to exhibit unexpected behaviour from namcap) are also welcome to extend namcap’s test suite.

If you have a patch (fixing a bug or a new namcap module), then you can open a [merge request](https://gitlab.archlinux.org/pacman/namcap/-/merge_requests).
One could also send patches to the arch-projects mailing list and hope somebody else opens an MR.
Namcap development is managed with git, so patches prepared with `git format-patch` are preferred.

# How to Use

To run namcap on a PKGBUILD source or a built binary package:

``` console
$ namcap FILENAME
```

If you want to see extra informational messages, then invoke namcap with the `-i` flag:

``` console
$ namcap -i FILENAME
```

You can also see the *namcap(1)* manual by typing `man namcap` at the command line or see the usage help:

``` console
$ namcap -h 
```

# Understanding the Output

Namcap uses a system of tags to classify the output. Currently, tags are of three types, errors (denoted by E), warnings (denoted by W), and informational
(denoted by I).

An error is important and should be fixed immediately; mostly they relate to insufficient security, missing licenses, or permission problems.

Normally namcap prints a human-readable explanation (sometimes with suggestions on how to fix the problem).
If you want output which can be easily parsed by a program, then pass the `-m` (machine-readable) flag to namcap.

# How It Works

Namcap takes the package name (or a PKGBUILD) as an argument that runs through all the rules defined in Namcap.rules submodules and tests them against the argument looking for flaws.

# How to Create a Module

This section documents the innards of namcap and specifies how to create a new namcap rule.

The main namcap program `namcap.py` takes as parameters the filename of a package or a PKGBUILD and makes a *pkginfo* object, which it passes to a list of rules defined in `Namcap.rules.all_rules`:

- a rule is a subclass of `AbstractRule` (defined in `Namcap.ruleclass`)
- `PkgInfoRule` classes process any *pkginfo* object
- `PkgbuildRule` classes process only PKGBUILDs
- `TarballRule` classes process binary packages

Put the new rule in a module and make sure it is imported in `Namcap/rules/__init__.py`.

A very simple rule is the “url” rule (`Namcap/rules/pkginfo.py`):

``` python
from Namcap.ruleclass import PkgInfoRule

class UrlRule(PkgInfoRule):
    name = "url"
    description = "Verifies the package comes with an URL"
    def analyze(self, pkginfo, tar):
        if 'url' not in pkginfo:
            self.errors.append(("missing-url", ()))
```

Mostly, the code is self-explanatory.
The following are the list of attributes that each namcap rule must have:

- *base class*

  Either `PkgInfoRull`, `PkgbuildRule`, or `TarballRule`.

- *name*

  Contains a string containing a short name of the module.
  Usually, this is the same as the basename of the module file.

- *description*

  A string containing a concise description of the module.
  This description is used when listing all the rules using `namcap -r list`.

- *analyze* `(self, pkginfo, tar)`

  Should fill three lists self.errors, self.warnings and self.infos with error tags, warning tags and information tags respectively.
  Each member of these tag lists should be a tuple consisting of two components: the short, hyphenated form of the tag with the appropriate format specifiers (%s, etc.).
  The first word of this string must be the tag name.
  The human readable form of the tag should be put in the `namcap-tags` file.
  The format of the tags file is described below; and the parameters which should replace the format specifier tokens in the final output.

The `namcap-tags` file consists of lines specifying the human readable form of the hyphenated tags used in the namcap code.
A line beginning with a ‘\#’ is treated as a comment.
Otherwise the format of the file is:

    machine-parseable-tag %s :: This is machine parseable tag %s

Note that a double colon (::) is used to separate the hyphenated tag from the human readable description.

# How to Typecheck the Code Base

Namcap supports [mypy](https://www.mypy-lang.org/), a type checker
that infers type information.
This helps uncover hidden issues, and enables editors and IDEs to
make more informed decisions, e.g. inline completions.

To verify that the code base is free from apparent typing errors,
run:

``` console
mypy
```

# How to Test a Rule

Namcap comes with a test suite covering the classical mistakes or warnings we are expecting from the rules.
The test suite is in `Namcap/tests`.
The *makepkg* and *pkgbuild_test* submodules provide easy generic methods to test a rule.

To run the test suite, run:

``` console
env PARSE_PKGBUILD_PATH="$PWD" \
    PATH="$PWD/scripts:$PATH" \
    pytest
```

To run a single test, append the test filename, e.g. `Namecap/tests/packages/test_shebangdepends.py` to the command-line above.

Here is an example testing the rule `pkgnameindesc`:

``` python
from Namcap.tests.pkgbuild_test import PkgbuildTest
import Namcap.rules.pkgnameindesc

class NamcapPkgnameInDescTest(PkgbuildTest):
    pkgbuild1 = "...some bogus pkgbuild..."
    def preSetUp(self):
        self.rule = Namcap.rules.pkgnameindesc.package

    def test_example1(self):
        r = self.run_on_pkg(self.pkgbuild1)
        self.assertEqual(r.errors, [])
        self.assertEqual(set(r.warnings), set(
            [("pkgname-in-description", ())] ))
        self.assertEqual(r.infos, [])
```

- the `preSetUp()` method sets the rule to be used in the test

- the `test_example1()` method runs the rule on the given PKGBUILD and tests whether the results are as expected

- the `PkgbuildTest` also automatically tests the rule against a set of known valid PKGBUILDs.

# More Information

You can find more information about namcap on [namcap’s wiki page](https://wiki.archlinux.org/title/Namcap)
