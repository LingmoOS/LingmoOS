// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QGuiApplication>
#include <QImageReader>
#include <QCommandLineParser>
#include <QDirIterator>
#include <QBuffer>
#include <QDebug>

#include <DDciFile>
#include <DIconTheme>
#include <DGuiApplicationHelper>
#include <DPlatformTheme>

DCORE_USE_NAMESPACE
DGUI_USE_NAMESPACE

#define MAX_SCALE 10
#define INVALIDE_QUALITY -2
static int quality4Scaled[MAX_SCALE] = {};
static inline void initQuality() {
    for (int i = 0; i < MAX_SCALE; ++i)
        quality4Scaled[i] = INVALIDE_QUALITY;
}

static inline void dciChecker(bool result, std::function<const QString()> cb) {
    if (!result) {
        qWarning() << "Failed on writing dci file" << cb();
        exit(-6);
    }
}

static inline QByteArray webpImageData(const QImage &image, int quality) {
    QByteArray data;
    QBuffer buffer(&data);
    bool ok = buffer.open(QIODevice::WriteOnly);
    Q_ASSERT(ok);
    dciChecker(image.save(&buffer, "webp", quality), []{return "failed to save webp image";});
    return data;
}

static bool writeScaledImage(DDciFile &dci, const QString &imageFile, const QString &targetDir, int scale/* = 2*/)
{
    QString sizeDir = targetDir.mid(1, targetDir.indexOf("/", 1) - 1);
    bool ok = false;
    int baseSize = sizeDir.toInt(&ok);
    if (!ok)
        baseSize = 256;

    int size = scale * baseSize;
    QImageReader image(imageFile);
    if (!image.canRead()) {
        qWarning() << "Ignore the null image file:" << imageFile;
        return false;
    }

    if (image.supportsOption(QImageIOHandler::ScaledSize)) {
        image.setScaledSize(QSize(size, size));
    }

    dciChecker(dci.mkdir(targetDir + QString("/%1").arg(scale)), [&]{return dci.lastErrorString();});
    const QImage &img = image.read().scaledToWidth(size, Qt::SmoothTransformation);
    int quality =  quality4Scaled[scale - 1];
    const QByteArray &data = webpImageData(img, quality);
    dciChecker(dci.writeFile(targetDir + QString("/%1/1.webp").arg(scale), data), [&]{return dci.lastErrorString();});

    return true;
}

static bool writeImage(DDciFile &dci, const QString &imageFile, const QString &targetDir)
{
    for (int i = 0; i < MAX_SCALE; ++i) {
        if (quality4Scaled[i] == INVALIDE_QUALITY)
            continue;

        if (!writeScaledImage(dci, imageFile, targetDir, i + 1))
            return false;
    }

    return true;
}

static bool recursionLink(DDciFile &dci, const QString &fromDir, const QString &targetDir)
{
    for (const auto &i : dci.list(fromDir, true)) {
        const QString file(fromDir + "/" + i);
        const QString targetFile(targetDir + "/" + i);
        if (dci.type(file) == DDciFile::Directory) {
            if (!dci.mkdir(targetFile))
                return false;
            if (!recursionLink(dci, file, targetFile))
                return false;
        } else {
            if (!dci.link(file, targetFile))
                return false;
        }
    }

    return true;
}

static QByteArray readNextSection(QIODevice *io) {
    QByteArray section;
    char ch;

    bool oneLine = true;
    while (io->getChar(&ch)) {
        if (!oneLine && ch == '"') { // ["] end
            if (io->peek(1) == ",")
                io->skip(1); // skip [,]
            break;
        }  else if (ch == ',') {
            break;
        } else if (ch == '"') { // ["] begin
            oneLine = false;
            continue;
        }

        section.append(ch);

        if(oneLine && io->peek(1) == "\n")
            break;
    }

    return section.trimmed();
}

QMultiHash<QString, QString> parseIconFileSymlinkMap(const QString &csvFile) {
    QFile file(csvFile);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed on open symlink map file:" << csvFile;
        exit(-7);
    }

    QMultiHash<QString, QString> map;
    while (!file.atEnd()) {
        QByteArray key = readNextSection(&file);
        QByteArray value = readNextSection(&file);
        for (const auto &i : value.split('\n')) {
            map.insert(QString::fromLocal8Bit(key), QString::fromLocal8Bit(i));
        }

        char ch = 0;
        while (file.getChar(&ch) && ch != '\n');
    }

    qInfo() << "Got symlinks:" << map.size();

    return map;
}

void makeLink(const QFileInfo &file, const QDir &outputDir, const QString &dciFilePath,
              const QMultiHash<QString, QString> &symlinksMap)
{
    if (symlinksMap.contains(file.completeBaseName())) {
        const QString symlinkKey = QFileInfo(dciFilePath).fileName();
        for (const auto &symTarget : symlinksMap.values(file.completeBaseName())) {
            const QString newSymlink = outputDir.absoluteFilePath(symTarget + ".dci");
            qInfo() << "Create symlink from" << symlinkKey << "to" << newSymlink;
            if (!QFile::link(symlinkKey, newSymlink)) {
                qWarning() << "Failed on create symlink from" << symlinkKey << "to" << newSymlink;
            }
        }
    }
}

void doFixDarkTheme(const QFileInfo &file, const QDir &outputDir, const QMultiHash<QString, QString> &symlinksMap)
{
    const QString &newFile = outputDir.absoluteFilePath(file.fileName());

    DDciFile dciFile(file.absoluteFilePath());
    if (!dciFile.isValid()) {
        qWarning() << "Skip invalid dci file:" << file.absoluteFilePath();
        return;
    }

    for (const auto &i : dciFile.list("/")) {
        if (dciFile.type(i) != DDciFile::Directory)
            continue;

        for (const auto &j : dciFile.list(i)) {
            if (dciFile.type(j) != DDciFile::Directory || !j.endsWith(".light"))
                continue;

            const QString darkDir(j.left(j.size() - 5) + "dark");
            Q_ASSERT(darkDir.endsWith(".dark"));
            if (!dciFile.exists(darkDir)) {
                dciChecker(dciFile.mkdir(darkDir), [&]{return dciFile.lastErrorString();});
                dciChecker(recursionLink(dciFile, j, darkDir), [&]{return dciFile.lastErrorString();});
            }
        }
    }

    dciChecker(dciFile.writeToFile(newFile), [&]{return dciFile.lastErrorString();});

    makeLink(file, outputDir, newFile, symlinksMap);
}

int main(int argc, char *argv[])
{
    QCommandLineOption fileFilter({"m", "match"}, "Give wildcard rules on search icon files, "
                                                  "Each eligible icon will be packaged to a dci file, "
                                                  "If the icon have the dark mode, it needs to store "
                                                  "the dark icon file at \"dark/\" directory relative "
                                                  "to current icon file, and the file name should be "
                                                  "consistent.", "wildcard palette");
    QCommandLineOption outputDirectory({"o", "output"}, "Save the *.dci files to the given directory.",
                                       "directory");
    QCommandLineOption symlinkMap({"s", "symlink"}, "Give a csv file to create symlinks for the output icon file."
                                                    "\nThe content of symlink.csv like:\n"
                                                    "\t\t sublime-text, com.sublimetext.2\n"
                                                    "\t\t deb, \"\n"
                                                    "\t\t application-vnd.debian.binary-package\n"
                                                    "\t\t application-x-deb\n"
                                                    "\t\t gnome-mime-application-x-deb\n"
                                                    "\t\t \"\n"
                                  ,
                                       "csv file");
    QCommandLineOption fixDarkTheme("fix-dark-theme", "Create symlinks from light theme for dark theme files.");
    QCommandLineOption iconFinder("find", "Find dci icon file path");
    QCommandLineOption themeOpt({"t","theme"}, "Give a theme name to find dci icon file path", "theme name");
    QCommandLineOption scaleQuality({"O","scale-quality"}, "Quility of dci scaled icon image\n"
                                                "The value may like <scale size>=<quality value>  e.g. 2=98:3=95\n"
                                                "The quality factor must be in the range 0 to 100 or -1.\n"
                                                "Specify 0 to obtain small compressed files, 100 for large uncompressed files "
                                                "and -1 to use the image handler default settings.\n"
                                                "The higher the quality, the larger the dci icon file size", "scale quality");

    QGuiApplication a(argc, argv);
    a.setApplicationName("dci-icon-theme");
    a.setApplicationVersion("0.0.6");

    QCommandLineParser cp;
    cp.setApplicationDescription("dci-icon-theme tool is a command tool that generate dci icons from common icons.\n"
                                 "For example, the tool is used in the following ways: \n"
                                 "\t dci-icon-theme /usr/share/icons/hicolor/256x256/apps -o ~/Desktop/hicolor -O 3=95\n"
                                 "\t dci-icon-theme -m *.png /usr/share/icons/hicolor/256x256/apps -o ~/Desktop/hicolor -O 3=95\n"
                                 "\t dci-icon-theme --fix-dark-theme <input dci files directory> -o <output directory path> \n"
                                 "\t dci-icon-theme --find <icon name>\n"
                                 "\t dci-icon-theme --find <icon name> -t bloom\n"
                                 "\t dci-icon-theme <input file directory> -o <output directory path> -s <csv file> -O <qualities>\n"""
                                 );

    cp.addOptions({fileFilter, outputDirectory, symlinkMap, fixDarkTheme, iconFinder, themeOpt, scaleQuality});
    cp.addPositionalArgument("source", "Search the given directory and it's subdirectories, "
                                       "get the files conform to rules of --match.",
                             "~/dci-png-icons");
    cp.addHelpOption();
    cp.addVersionOption();
    cp.process(a);

    if (a.arguments().size() == 1)
        cp.showHelp(-1);

    bool isIconFinder = cp.isSet(iconFinder);
    if (cp.positionalArguments().isEmpty()) {
        qWarning() << "Not give a" << (isIconFinder ? "icon name." : "source directory.");
        cp.showHelp(-2);
    }

    QString iconThemeName;
    if (cp.isSet(themeOpt)) {
        iconThemeName = cp.value(themeOpt);
    } else {
        iconThemeName = DGuiApplicationHelper::instance()->applicationTheme()->iconThemeName();
    }

    if (isIconFinder) {
        QString iconName = cp.positionalArguments().value(0);

        QString iconPath = DIconTheme::findDciIconFile(iconName, iconThemeName);
        qInfo() << iconName << "[" << iconThemeName << "]:" << iconPath;
        return 0;
    }

    if (!cp.isSet(outputDirectory)) {
        qWarning() << "Not give -o argument";
        cp.showHelp(-4);
    }

    if (!cp.isSet(scaleQuality) && !cp.isSet(fixDarkTheme)) {
        qWarning() << "Not give -O argument"; scaleQuality.flags();
        cp.showHelp(-5);
    }

    initQuality();
    QString surfix;
    if (cp.isSet(scaleQuality)) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        auto behavior = Qt::SkipEmptyParts;
#else
        auto behavior = QString::SkipEmptyParts;
#endif
        QStringList qualityList = cp.value(scaleQuality).split(":", behavior);

#ifdef QT_DEBUG
            surfix = cp.value(scaleQuality).prepend("-");
#endif

        for (const QString &kv : qualityList) {
            auto sq = kv.split("=");
            if (sq.size() != 2) {
                qWarning() << "Invalid quality value:" << kv;
                continue;
            }

            int scaleSize = sq.value(0).toInt();
            if (scaleSize < 1 || scaleSize > MAX_SCALE) {
                qWarning() << "Invalid scale size:" << kv;
                continue;
            }
            int validQuality = qMax(qMin(sq.value(1).toInt(), 100), -1); // -1,  0~100
            quality4Scaled[scaleSize - 1] = validQuality;
        }
    }

    QDir outputDir(cp.value(outputDirectory));
    if (!outputDir.exists()) {
        if (!QDir::current().mkpath(outputDir.absolutePath())) {
            qWarning() << "Can't create the" << outputDir.absolutePath() << "directory";
            cp.showHelp(-5);
        }
    } else {
        qErrnoWarning("The output directory have been exists.");
#ifndef QT_DEBUG
        return -1;
#endif
    }

    QMultiHash<QString, QString> symlinksMap;
    if (cp.isSet(symlinkMap)) {
        symlinksMap = parseIconFileSymlinkMap(cp.value(symlinkMap));
    }

    const QStringList nameFilter = cp.isSet(fileFilter) ? cp.values(fileFilter) : QStringList();
    const auto sourceDirectory = cp.positionalArguments();
    for (const auto &sd : qAsConst(sourceDirectory)) {
        QDir sourceDir(sd);
        if (!sourceDir.exists()) {
            qWarning() << "Ignore the non-exists directory:" << sourceDir;
            continue;
        }

        // read all links first
        {
            QDirIterator di(sourceDir.absolutePath(), nameFilter,
                            QDir::NoDotAndDotDot | QDir::Files,
                            QDirIterator::Subdirectories);
            while (di.hasNext()) {
                di.next();
                QFileInfo file = di.fileInfo();
                if (!file.isSymLink())
                    continue;

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
        auto link = file.symLinkTarget();
#else
        auto link = file.readLink();
#endif
                 const QString &linkTarget = QFileInfo(link).completeBaseName();
                 if (!symlinksMap.values(linkTarget).contains(file.completeBaseName())) {
                     symlinksMap.insert(linkTarget, file.completeBaseName());
                     qInfo() << "Add link" << file.completeBaseName() << "->" << linkTarget;
                 } else {
//                        qDebug() << "** link already existed in symlinksMap **";
                 }
            }
        }

        QDirIterator di(sourceDir.absolutePath(), nameFilter,
                        QDir::NoDotAndDotDot | QDir::Files,
                        QDirIterator::Subdirectories);
        while (di.hasNext()) {
            di.next();
            QFileInfo file = di.fileInfo();

            if (file.isSymLink())
                continue;

            if (cp.isSet(fixDarkTheme)) {
                doFixDarkTheme(file, outputDir, symlinksMap);
                continue;
            }

            if (file.path().endsWith(QStringLiteral("/dark"))) {
                qInfo() << "Ignore the dark icon file:"  << file;
                continue;
            }

            QString dirName = file.absoluteDir().dirName();
            bool isNum = false;
            dirName.toInt(&isNum);
            dirName.prepend("/");

            QScopedPointer<DDciFile> dciFile;
            const QString dciFilePath(outputDir.absoluteFilePath(file.completeBaseName()) + surfix + ".dci");
            if (QFileInfo::exists(dciFilePath)) {
                dciFile.reset(new DDciFile(dciFilePath));
                if (dciFile->isValid() && dciFile->exists(dirName)) {
                     qWarning() << "Skip exists dci file:" << dciFilePath << dirName << dciFile->list(dirName);
                     continue;
                }
            }

            qInfo() << "Writing to dci file:" << file.absoluteFilePath() << "==>" << dciFilePath;

            if (dciFile.isNull() || !dciFile->isValid())
                dciFile.reset(new DDciFile);

            QString sizeDir = isNum ? dirName : "/256";  // "/256"
            QString normalLight = sizeDir + "/normal.light";         //  "/256/normal.light"
            QString normalDark = sizeDir + "/normal.dark";          //   "/256/normal.dark"

            dciChecker(dciFile->mkdir(sizeDir), [&]{return dciFile->lastErrorString();});
            dciChecker(dciFile->mkdir(normalLight), [&]{return dciFile->lastErrorString();});
            if (!writeImage(*dciFile, file.filePath(), normalLight))
                continue;

            dciChecker(dciFile->mkdir(normalDark), [&]{return dciFile->lastErrorString();});
            QFileInfo darkIcon(file.dir().absoluteFilePath("dark/" + file.fileName()));
            if (darkIcon.exists()) {
                writeImage(*dciFile, darkIcon.filePath(), normalDark);
            } else {
                dciChecker(recursionLink(*dciFile, normalLight, normalDark), [&]{return dciFile->lastErrorString();});
            }

            dciChecker(dciFile->writeToFile(dciFilePath), [&]{return dciFile->lastErrorString();});

            makeLink(file, outputDir, dciFilePath, symlinksMap);
        }
    }

    return 0;
}
