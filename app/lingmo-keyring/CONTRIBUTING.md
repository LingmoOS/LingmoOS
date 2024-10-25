# Contributing

These are the contribution guidelines for archlinux-keyring.
All code contributions fall under the terms of the GPL-3.0-or-later (see
[LICENSE](LICENSE)).

Please read our distribution-wide [Code of
Conduct](https://terms.archlinux.org/docs/code-of-conduct/) before
contributing, to understand what actions will and will not be tolerated.

Development of archlinux-keyring takes place on Arch Linux' Gitlab:
https://gitlab.archlinux.org/archlinux/archlinux-keyring.

Any merge request to the repository requires two approvals of authorized
approvers (the current main key holders).

## Discussion

Discussion around archlinux-keyring may take place on the [arch-projects
mailing list](https://lists.archlinux.org/listinfo/arch-projects) and in
[#archlinux-projects](ircs://irc.libera.chat/archlinux-projects) on [Libera
Chat](https://libera.chat/).

## Requirements

The following additional packages need to be installed to be able to lint
and develop this project:

* python-black
* python-coverage
* python-isort
* python-pytest
* python-tomli
* flake8
* mypy

## Keyringctl

The `keyringctl` script is written in typed python, which makes use of
[sequoia](https://sequoia-pgp.org/)'s `sq` command.

The script is type checked, linted and formatted using standard tooling.
When providing a merge request make sure to run `make lint`.

## Testing

Test cases are developed per module in the [test](test) directory and should
consist of atomic single expectation tests. A Huge test case asserting various
different expectations are discouraged and should be split into finer grained
test cases.

To execute all tests using pytest
```bash
make test
```

To run keyring integrity and consistency checks
```bash
make check
```

## Web Key Directory

Only tagged releases are built and exposed via WKD. This helps to ensure, that
inconsistent state of the keyring is not exposed to the enduser, which may make
use of it instantaneously.
