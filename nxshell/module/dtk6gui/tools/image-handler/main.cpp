// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DImageHandler>

#include <QHash>
#include <QGuiApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

DGUI_USE_NAMESPACE

bool rotateImage(DImageHandler &handler, QImage &image, const QString &param)
{
    int degresses = param.toInt();
    if (!handler.rotateImage(image, degresses)) {
        qWarning().noquote() << "Rotate image failed" << handler.lastError();
        return false;
    }

    return true;
}

bool applyImageFilter(QImage &image, const QString &param)
{
    enum FilterType { Old, Warm, Cool, Gray, AntiColor, Metal };
    static const QHash<QString, FilterType> colorFilters{
        {"old", Old}, {"warm", Warm}, {"cool", Cool}, {"gray", Gray}, {"anticolor", AntiColor}, {"metal", Metal}};

    QString filter = param.toLower();
    if (!colorFilters.contains(filter)) {
        qWarning().noquote() << QString("There is no color filter named %1.").arg(filter);
        return false;
    }

    FilterType type = colorFilters.value(filter);
    switch (type) {
        case Old:
            image = DImageHandler::oldColorFilter(image);
            break;
        case Warm:
            image = DImageHandler::warmColorFilter(image);
            break;
        case Cool:
            image = DImageHandler::coolColorFilter(image);
            break;
        case Gray:
            image = DImageHandler::grayScaleColorFilter(image);
            break;
        case AntiColor:
            image = DImageHandler::antiColorFilter(image);
            break;
        case Metal:
            image = DImageHandler::metalColorFilter(image);
            break;
        default:
            break;
    }

    return true;
}

void printMultipleImage(const QStringList &imageFiles, bool extra)
{
    if (imageFiles.isEmpty()) {
        return;
    }

    DImageHandler handler;
    QStringList notSupportList;
    QStringList errorList;
    for (const QString &imageFile : imageFiles) {
        handler.setFileName(imageFile);
        if (!handler.isReadable()) {
            notSupportList << imageFile;
        } else {
            // Load image internal.
            QSize size = handler.imageSize();
            QString errorString = handler.lastError();
            if (errorString.isEmpty()) {
                qInfo().noquote() << imageFile << handler.imageFormat() << size;
                if (extra) {
                    qInfo().noquote() << handler.findAllMetaData();
                }
            } else {
                errorList << QString("%1 %2").arg(imageFile).arg(errorString);
            }
        }
    }

    if (!notSupportList.isEmpty()) {
        qInfo().noquote() << "\nNot support image format:";
        for (const QString &notSupport : notSupportList) {
            qInfo().noquote() << notSupport;
        }
    }

    if (!errorList.isEmpty()) {
        qInfo().noquote() << "\nLoad image error:";
        for (const QString &error : errorList) {
            qInfo().noquote() << error;
        }
    }
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName("Image handler");

    QCommandLineOption rotateOption({"r", "rotate"}, "Rotate image with geven degresses.", "degresses");
    QCommandLineOption saveOption({"o", "output"}, "Save image with file path.", "file");
    QCommandLineOption filterOption("f",
                                    "Apply filters to pictures.\n"
                                    "Contains old, warm, cool, gray, anticolor and metal.",
                                    "filter");
    QCommandLineOption extraOption("e", "Show extra image info.");
    QCommandLineOption supportOption({"l", "list"}, "List all support image formats");

    QCommandLineParser parser;
    parser.setApplicationDescription("Image handler");
    parser.addHelpOption();
    parser.addOption(rotateOption);
    parser.addOption(saveOption);
    parser.addOption(filterOption);
    parser.addOption(extraOption);
    parser.addOption(supportOption);
    parser.addPositionalArgument("file", "Open file. More than one file will only display file info.", "[file...]");
    parser.process(app);

    if (parser.isSet(supportOption)) {
        qInfo().noquote() << "Support image formats:";
        qInfo().noquote() << DImageHandler::supportFormats().join(',');
        return 0;
    }

    const QStringList fileArgs = parser.positionalArguments();
    if (fileArgs.isEmpty()) {
        parser.showHelp();
        return 0;
    }

    bool needRotate = parser.isSet(rotateOption);
    bool needFilter = parser.isSet(filterOption);
    if (needRotate || needFilter) {
        QString fileName = fileArgs.first();
        QString saveFileName = parser.isSet(saveOption) ? parser.value(saveOption) : fileName;

        DImageHandler handler;
        handler.setFileName(fileName);
        QImage image = handler.readImage();
        if (image.isNull()) {
            qWarning().noquote() << "Can't read image." << handler.lastError();
            return 0;
        }

        if (needRotate) {
            if (!rotateImage(handler, image, parser.value(rotateOption))) {
                return 0;
            }
        }

        if (needFilter) {
            if (!applyImageFilter(image, parser.value(filterOption))) {
                return 0;
            }
        }

        if (!handler.saveImage(image, saveFileName)) {
            qWarning().noquote() << QString("Save file %1 failed.").arg(saveFileName) << handler.lastError();
            return 0;
        }
    } else {
        printMultipleImage(fileArgs, parser.isSet(extraOption));
    }

    return 0;
}