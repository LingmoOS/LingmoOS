/*
    SPDX-FileCopyrightText: 2014 Harald Fernengel <harry@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

/* This tool converts an svg to a Mac OS X icns file.
 * Note: Requires the 'iconutil' Mac OS X binary
 */

#include <QCommandLineParser>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>

#include <QGuiApplication>
#include <QPainter>
#include <QStandardPaths>
#include <QSvgRenderer>

#include "../../kiconthemes_version.h"

#include <stdio.h>

/* clang-format off */
#define EXIT_ON_ERROR(isOk, ...) \
    do { \
        if (!(isOk)) { \
            fprintf(stderr, __VA_ARGS__); \
            return 1; \
        } \
    } while (false);
/* clang-format on */

static bool writeImage(QSvgRenderer &svg, int size, const QString &outFile1, const QString &outFile2 = QString())
{
    QImage out(size, size, QImage::Format_ARGB32);
    out.fill(Qt::transparent);

    QPainter painter(&out);
    svg.setAspectRatioMode(Qt::KeepAspectRatio);
    svg.render(&painter);
    painter.end();

    if (!out.save(outFile1)) {
        fprintf(stderr, "Unable to write %s\n", qPrintable(outFile1));
        return false;
    }
    if (!outFile2.isEmpty() && !out.save(outFile2)) {
        fprintf(stderr, "Unable to write %s\n", qPrintable(outFile2));
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    app.setApplicationName(QStringLiteral("ksvg2icns"));
    app.setApplicationVersion(QStringLiteral(KICONTHEMES_VERSION_STRING));
    QCommandLineParser parser;
    parser.setApplicationDescription(app.translate("main", "Creates an icns file from an svg image"));
    parser.addPositionalArgument("iconname", app.translate("main", "The svg icon to convert"));
    parser.addHelpOption();

    parser.process(app);
    if (parser.positionalArguments().isEmpty()) {
        parser.showHelp();
        return 1;
    }

    bool isOk;

    // create a temporary dir to create an iconset
    QTemporaryDir tmpDir(QDir::tempPath() + QStringLiteral("/ksvg2icns"));

    isOk = tmpDir.isValid();
    EXIT_ON_ERROR(isOk, "Unable to create temporary directory\n");

    const QString outPath = tmpDir.filePath(QStringLiteral("out.iconset"));

    isOk = QDir(tmpDir.path()).mkpath(outPath);
    EXIT_ON_ERROR(isOk, "Unable to create out.iconset directory\n");

    const QStringList &args = app.arguments();
    EXIT_ON_ERROR(args.size() == 2, "Usage: %s svg-image\n", qPrintable(args.value(0)));

    const QString &svgFileName = args.at(1);

    // open the actual svg file
    QSvgRenderer svg;
    isOk = svg.load(svgFileName);
    EXIT_ON_ERROR(isOk, "Unable to load %s\n", qPrintable(svgFileName));

    // The sizes are from:
    // https://developer.apple.com/library/mac/documentation/GraphicsAnimation/Conceptual/HighResolutionOSX/Optimizing/Optimizing.html

    struct OutFiles {
        int size;
        QString out1;
        QString out2;
    };

    // create the pngs in various resolutions
    const OutFiles outFiles[] = {{1024, outPath + QStringLiteral("/icon_512x512@2x.png"), QString()},
                                 {512, outPath + QStringLiteral("/icon_512x512.png"), outPath + QStringLiteral("/icon_256x256@2x.png")},
                                 {256, outPath + QStringLiteral("/icon_256x256.png"), outPath + QStringLiteral("/icon_128x128@2x.png")},
                                 {128, outPath + QStringLiteral("/icon_128x128.png"), QString()},
                                 {64, outPath + QStringLiteral("/icon_32x32@2x.png"), QString()},
                                 {32, outPath + QStringLiteral("/icon_32x32.png"), outPath + QStringLiteral("/icon_16x16@2x.png")},
                                 {16, outPath + QStringLiteral("/icon_16x16.png"), QString()}};

    for (const OutFiles &outFile : outFiles) {
        isOk = writeImage(svg, outFile.size, outFile.out1, outFile.out2);
        if (!isOk) {
            return 1;
        }
    }

    // convert the iconset to icns using the "iconutil" command
    const QString iconutilExec = QStandardPaths::findExecutable(QStringLiteral("iconutil"));
    if (iconutilExec.isEmpty()) {
        EXIT_ON_ERROR(false, "Could not find iconutil executable in PATH.\n");
    }

    const QString outIcns = QFileInfo(svgFileName).baseName() + QStringLiteral(".icns");

    const QStringList utilArgs = QStringList() << "-c"
                                               << "icns"
                                               << "-o" << outIcns << outPath;

    QProcess iconUtil;

    iconUtil.start(iconutilExec, utilArgs, QIODevice::ReadOnly);
    isOk = iconUtil.waitForFinished(-1);
    EXIT_ON_ERROR(isOk, "Unable to launch iconutil: %s\n", qPrintable(iconUtil.errorString()));

    EXIT_ON_ERROR(iconUtil.exitStatus() == QProcess::NormalExit, "iconutil crashed!\n");
    EXIT_ON_ERROR(iconUtil.exitCode() == 0, "iconutil returned %d\n", iconUtil.exitCode());

    return 0;
}
