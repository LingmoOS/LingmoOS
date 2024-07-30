# Prison

A barcode abstraction layer providing uniform access to generation of barcodes with data.

## Introduction

Prison has a Prison::AbstractBarcode, which is the base class for the actual
barcode generators. Prison currently implements barcode generators for the following formats:

- QRCode
- Data Matrix
- Aztec
- Code39
- Code93
- [Code128](https://en.wikipedia.org/wiki/Code_128)
- [PDF417](https://en.wikipedia.org/wiki/PDF417) (ISO/IEC 15438)
- [EAN13](https://en.wikipedia.org/wiki/International_Article_Number)

Prison currently ships the org.kde.prison.Barcode QML element that can be used to render barcodes in QML code.

## Supported Barcode types

There are basically two types of barcodes:
* barcodes that carry the data
* barcodes that carry a lookup number, and require a specific server to
  look up the actual data.

Prison isn't as such designed for the latter, it will probably work, but
patches implementing barcode support for such barcodes will not be accepted.
An example is [EZCode](https://en.wikipedia.org/wiki/EZcode).

Prison is currently using [libdmtx](https://github.com/dmtx/libdmtx) for generation of
[DataMatrix](https://en.wikipedia.org/wiki/Datamatrix) barcodes,
[libqrencode](https://fukuchi.org/works/qrencode/) for generation
of [QRCode](https://en.wikipedia.org/wiki/QR_Code) barcodes and
[ZXing](https://github.com/nu-book/zxing-cpp) for generating
[EAN13 and PDF417](https://en.wikipedia.org/wiki/PDF417) barcodes.

# Prison Scanner

A barcode scanner consuming a live video feed from QtMultimedia.

## Introduction

Prison's barcode scanner can be used from C++ code using the Prison::VideoScanner
class, or from QML using the org.kde.prison.scanner.VideoScanner QML element.

There are standalone QML examples in tests/scanner-qt(5|6).qml demonstrating
the QML API.

## Supported barcode formats

Barcode detection is implemented using the [ZXing](https://github.com/nu-book/zxing-cpp)
library, all formats supported by ZXing can be detected.
