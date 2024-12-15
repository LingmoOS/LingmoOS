## Lingmo Tool Kit Core

Lingmo Tool Kit (DtkCore) is the base development tool of all C++/Qt Developer work on Lingmo.

You should read the <a href=docs/Specification.md>Lingmo Application Specification</a> firstly.

中文说明：[README.zh_CN.md](./README.zh_CN.md)

## Document

中文文档：[dtkcore文档](https://lingmoos.github.io/dtkcore/index.html)

## Dependencies

### Build dependencies

* Qt >= 5.10

## Compile option

| **Compile option**           | **meaning**      | **Default state**      |
|--------------------|-------------|---------------|
| BUILD_DOCS         | Compile document  | ON            |
| BUILD_TESTING      | Compile test      | Default is ON in debug mode |
| BUILD_EXAMPLES     | Compile example   | ON            |
| BUILD_WITH_SYSTEMD | Support Systemd function | OFF           |
| BUILD_THEME        | Add themes to the document | OFF           |
## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:

```bash
mkdir build
cd build
cmake ..
make
```

3. Install:

```bash
sudo make install
```

## Getting help

Any usage issues can ask for help via

* [Telegram group](https://t.me/lingmo)
* [Matrix](https://matrix.to/#/#lingmo-community:matrix.org)
* [IRC (libera.chat)](https://web.libera.chat/#lingmo-community)
* [Forum](https://bbs.lingmo.org)
* [WiKi](https://wiki.lingmo.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en).

## License

lingmo-tool-kit is licensed under [LGPL-3.0-or-later](LICENSE).
