# lingmo-keyring

The lingmo-keyring project is based on archlinux-keyring. Is holding PGP package materials and tools
('Keyringctl') To create a distribution key ring for Lingmo OS.
The key ring is used by the LPM to establish a trust network for the wrapper
Distributed. Can be used on arch linux

The PGP packets describing the main signing keys can be found below the
[keyring/main](keyring/main) directory, while those of the packagers are located below the
[keyring/packager](keyring/packager) directory.

## Requirements

The following packages need to be installed to be able to create a PGP keyring
from the provided data structure and to install it:

Build:

* make
* findutils
* pkgconf
* systemd

Runtime:

* python
* sequoia-sq >= 0.31.0

Optional:

* hopenpgp-tools (verify)
* git (ci)

## Usage

### Build

Build all PGP artifacts (keyring, ownertrust, revoked files) to the build directory
```bash
./keyringctl build
```

### Import

Import a new packager key by deriving the username from the filename.
```bash
./keyringctl import <username>.asc
```

Alternatively import a file or directory and override the username
```bash
./keyringctl import --name <username> <file_or_directory...>
```

Updates to existing keys will automatically derive the username from the known fingerprint.
```bash
./keyringctl import <file_or_directory...>
```

Main key imports support the same options plus a mandatory `--main`
```bash
./keyringctl import --main <username>.asc
```

### Export

Export the whole keyring including main and packager to stdout
```bash
./keyringctl export
```

Limit to specific certs using an output file
```bash
./keyringctl export <username_or_fingerprint_or_directory...> --output <filename>
```

### List

List all certificates in the keyring
```bash
./keyringctl list
```

Only show a specific main key
```bash
./keyringctl list --main <username_or_fingerprint...>
```

### Inspect

Inspect all certificates in the keyring
```bash
./keyringctl inspect
```

Only inspect a specific main key
```bash
./keyringctl inspect --main <username_or_fingerprint_or_directory...>
```

### Verify

Verify certificates against modern expectations and assumptions
```bash
./keyringctl verify <username_or_fingerprint_or_directory...>
```

## Installation

To install archlinux-keyring system-wide use the included `Makefile`:

```bash
make install
```

## Contribute

Read our [contributing guide](CONTRIBUTING.md) to learn more about guidelines and
how to provide fixes or improvements for the code base.

## License

Archlinux-keyring is licensed under the terms of the **GPL-3.0-or-later** (see
[LICENSE](LICENSE)).
