/*
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#ifndef PRISON_IMAGESCANNER_H
#define PRISON_IMAGESCANNER_H

#include "format.h"
#include "prisonscanner_export.h"

class QImage;

namespace Prison
{

class ScanResult;

/** Scans a still image for barcodes.
 *
 *  @since 6.3
 */
namespace ImageScanner
{

/** Scan @p image for a barcode.
 *  This method is synchronous and expensive.
 *  For use in the main thread running this on a secondary thread is strongly recommended
 *  when processing larger images.
 *
 *  @code
 *  QtConcurrent::run([&img]() { return ImageScanner::scan(img); }).then([](const ScanResult &result) {
 *    ...
 *  });
 *  @endcode
 *
 *  @param image The image to scan for barcodes, in any format.
 *  @param formats The barcode formats to look for. By default all supported formats
 *  are searched, limiting this improves performance and can improve result quality.
 *
 *  @since 6.3
 */
[[nodiscard]] PRISONSCANNER_EXPORT ScanResult scan(const QImage &image, Format::BarcodeFormats formats = {});

}
}

#endif // PRISON_IMAGESCANNER_H
