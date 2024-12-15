// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QImage>

#define ALPHA8STRING QLatin1String("alpha8")

static bool convertImageTo(const QString &originPath, const QString &targetPath)
{
    QImage originImage(originPath);
    if (originImage.isNull())
        return false;
    const auto fi = QFileInfo(originPath);
    const QString &suffix = fi.suffix();

    // ###(Chen bin): The tool only determines whether the image is suitable
    // for conversion，and does not judge the palette.
    if ((originImage.width() % 2) || (originImage.height() % 2))
        return false;

    auto tmp = originImage.convertToFormat(QImage::Format_Alpha8);
    QImage alpha(tmp.constBits(), tmp.width(), tmp.height(), tmp.bytesPerLine(), QImage::Format_Grayscale8);
    return alpha.save(targetPath, suffix.toLocal8Bit());
}

static bool convertImageFrom(const QString &originPath, const QString &targetPath) {
    QString path(originPath);
    auto fi = QFileInfo(originPath);
    QString suffix = fi.suffix();
    if (suffix.compare(ALPHA8STRING, Qt::CaseInsensitive))
        return false;  // No convert to alpha8.
    path.remove(QLatin1Char('.') + fi.suffix());
    QImage originImage(originPath);
    if (originPath.isNull() || originImage.format() != QImage::Format_Grayscale8)
        return false;
    suffix = QFileInfo(path).suffix();
    QImage targetImage(originImage.bits(), originImage.width(), originImage.height(),
                       originImage.bytesPerLine(), QImage::Format_Alpha8);
    return targetImage.save(targetPath, suffix.toLocal8Bit());
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setApplicationName("dci-image-converter");
    a.setApplicationVersion("0.0.1");

    QCommandLineParser commandParser;

    auto options = QList<QCommandLineOption> {
        QCommandLineOption("toAlpha8", "Convert image format to alpha8.", "targetPath"),
        QCommandLineOption("fromAlpha8", "Convert image format from alpha8.", "targetPath"),
    };

    commandParser.addOptions(options);
    commandParser.addPositionalArgument("sourcesPath", "The file path of the original image to be converted.",
                                        "[dir1 dir2...]/[file1 file2...]");
    commandParser.addHelpOption();
    commandParser.addVersionOption();
    commandParser.process(a);

    auto arguments = commandParser.positionalArguments();
    if (arguments.isEmpty())
        commandParser.showHelp(-1);
    if (commandParser.isSet(options.at(0))) {
        if (!convertImageTo(arguments.first(), commandParser.value(options.at(0)))) {
            printf("Convert image failed.\n");
            return 1;
        }
    } else if (commandParser.isSet(options.at(1))) {
        if (!convertImageFrom(arguments.first(), commandParser.value(options.at(1)))) {
            printf("Convert image failed.\n");
            return 1;
        }
    } else {
        commandParser.showHelp(-1);
    }

    return 0;
}
