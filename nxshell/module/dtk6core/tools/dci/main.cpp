// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QDebug>

#include <unistd.h>

#include "dci/ddcifile.h"

static QString getSymLinkTarget(const QString &file, const QString &originPath) {
    char target[512] = {0};
    const auto ret = readlink(file.toLocal8Bit().constData(), target, 511);
    if (ret <= 0)
        return QString();
    QString tar = QByteArray(target, ret);
    QString path = originPath;
    if (originPath.endsWith('/'))
        path = path.left(path.length() - 1);
    if (tar.startsWith(path)) {
        tar = tar.remove(0, path.length());
    }
    return tar;
}

static bool copyFilesToDci(DDciFile *dci, const QString &targetDir, const QString &sourceDir, const QString &originPath) {
    QDir dir(sourceDir);
    for (const auto &info : dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        const QString &newFile = QDir(targetDir).filePath(info.fileName());
        if (info.isDir()) {
            if (!dci->mkdir(newFile))
                return false;
            if (!copyFilesToDci(dci, newFile, info.absoluteFilePath(), originPath))
                return false;
        } else if (info.isSymLink()) {
            if (!dci->link(getSymLinkTarget(info.absoluteFilePath(), originPath), newFile))
                return false;
        } else if (info.isFile()) {
            QFile file(info.absoluteFilePath());
            if (!file.open(QIODevice::ReadOnly))
                return false;
            if (!dci->writeFile(newFile, file.readAll()))
                return false;
        }
    }

    return true;
}

QString cleanPath(const QString &path) {
    return path.size() < 2 || !path.endsWith(QDir::separator()) ? path : path.chopped(1);
}

bool createTo(const QString &sourceDir, const QString &targetDir) {
    QFileInfo info(cleanPath(sourceDir));
    if (!info.isDir())
        return false;

    const QString &iconName = info.fileName();
    if (iconName.isEmpty()) {
        printf("The icon name is not correctly resolved in the source path: \"%s\".\n", qPrintable(sourceDir));
        return false;
    }

    const auto newFile = QDir(targetDir).filePath(iconName + ".dci");
    if (QFile::exists(newFile)) {
        printf("The path \"%s\" already exists.\n", qPrintable(newFile));
        return false;
    }

    DDciFile dci;
    if (!copyFilesToDci(&dci, "/", sourceDir, sourceDir))
        return false;

    return dci.writeToFile(newFile);
}

static bool copyFilesFromDci(const DDciFile *dci, const QString &targetDir, const QString &sourceDir, QMap<QString, QString> &pathMap) {
    QDir target(targetDir);
    for (const QString &file : dci->list(sourceDir)) {
        const QString &newFileName = QFileInfo(file).fileName();
        const QString &newFilePath = target.filePath(newFileName);
        pathMap.insert(file, newFilePath);
        const auto &type = dci->type(file);
        if (type == DDciFile::Directory) {
            if (!target.mkdir(newFileName))
                return false;
            if (!copyFilesFromDci(dci, newFilePath, file, pathMap))
                return false;
        } else if (type == DDciFile::File) {
            QFile newFile(newFilePath);
            if (!newFile.open(QIODevice::WriteOnly))
                return false;
            const auto &data = dci->dataRef(file);
            if (newFile.write(data) != data.size())
                return false;
        } else if (type == DDciFile::Symlink) {
            // link the real source later
        } else {
            return false;
        }
    }

    return true;
}

bool exportTo(const QString &dciFile, const QString &targetDir) {
    QFileInfo info(dciFile);
    if (!info.isFile() || info.suffix() != "dci")
        return false;
    const QString &newDir = QDir(targetDir).filePath(info.baseName());
    if (QDir(newDir).exists())
        return false;
    if (!QDir::current().mkdir(newDir))
        return false;

    DDciFile dci(dciFile);
    if (!dci.isValid())
        return false;

    QMap<QString, QString> pathMap;
    if (!copyFilesFromDci(&dci, QDir(newDir).absolutePath(), "/", pathMap))
        return false;

    // link to real source
    for (auto it = pathMap.begin(); it != pathMap.end(); it++) {
        if (dci.type(it.key()) == DDciFile::Symlink) {
            const QString &realSource = it.value();
            const QDir &dirInDci(QFileInfo(it.key()).path());
            const QDir &dirRealSrc(QFileInfo(realSource).path());
            const QString &target = dci.symlinkTarget(it.key(), true);
            const QString &absoluteTarget = QDir::cleanPath(dirInDci.absoluteFilePath(target));
            const QString &realTarget = pathMap.value(absoluteTarget);

            // link to relative path(e.g. xx.webp -> ../../normal.light/x/xx.webp)
            if (!QFile::link(dirRealSrc.relativeFilePath(realTarget), realSource)) {
                qErrnoWarning(strerror(errno));
                return false;
            }
        }
    }
    return true;
}

#define SPACE_CHAR    "    "
#define BEGINE_CHAR   "├── "
#define MIDDLE_CHAR   "│   "
#define END_CHAR      "└── "
#define ARROW_CHAR    " -> "

static inline QString pathName(const QString &path)
{
    QDir dir(path);
    return  dir.dirName().isEmpty() ? path : dir.dirName();
}

void print(DDciFile &dci, const QString &dir = QString("/"), QString prefix = QString())
{
    const auto &fileList = dci.list(dir);
    QString dirName = pathName(dir);

    printf("%s\n", qPrintable(prefix + dirName));

    prefix.replace(END_CHAR, SPACE_CHAR);
    prefix.replace(BEGINE_CHAR,  MIDDLE_CHAR);

    for (const auto &file : fileList) {
        QString newPrefix = prefix;
        newPrefix.append(file == fileList.last() ? END_CHAR : BEGINE_CHAR);
        if (dci.type(file) == DDciFile::Directory) {
            print(dci, file, newPrefix);
        } else {
            QString fileName = pathName(file);
            QString symlinkTarget;
            if (dci.type(file) == DDciFile::Symlink) {
                symlinkTarget.append(ARROW_CHAR).append(dci.symlinkTarget(file));
            }
            printf("%s\n", qPrintable(newPrefix + fileName + symlinkTarget));
        }
    }
}

bool tree(const QString &dciFile)
{
    QFileInfo info(dciFile);
    DDciFile dci(dciFile);
    if (!dci.isValid())
        return false;

    print(dci, "/");

    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    a.setOrganizationName("zccrs");
    a.setApplicationName("dci");
    a.setApplicationVersion("0.0.1");

    QCommandLineParser commandParser;
    commandParser.setApplicationDescription("DCI tool is a command tool that automatically packs and unpacks DCI directories.\n"
                                            "If you have created an icon directory according to the correct DCI directory specification,\n"
                                            "you can easily make the DCI icons with this tool.\n"
                                            "The commands of DCI tools can be expressed as follows:\n"
                                            "\t dci --create [target file path] [source directory path]\n"
                                            "\t dci --export [target directory path] [source file path]\n"
                                            "\t dci --tree [target file path]\n"
                                            "For example, the tool is used in the following ways: \n"
                                            "\t dci --create ~/Desktop ~/Desktop/action_add\n"
                                            "\t dci --export ~/Desktop ~/Desktop/action_add.dci\n"
                                            "\t dci --tree ~/Desktop/action_add.dci\n");

    auto options = QList<QCommandLineOption>
    {
        QCommandLineOption("create", "Create the new dci files by the directorys", "targetDirectiry"),
        QCommandLineOption("export", "Export the dci files to the directorys", "targetDirectory"),
        QCommandLineOption("tree", "tree view the dci file", "targetDciFile"),
    };
    commandParser.addOptions(options);
    commandParser.addPositionalArgument("sources", "The directorys of create or the dci files of export",
                                        "[dir1 dir2...]/[file1 file2...]");
    commandParser.addHelpOption();
    commandParser.addVersionOption();
    commandParser.process(a);

    if (commandParser.isSet(options.at(0))) {
        for (const QString &dir : commandParser.positionalArguments()) {
            if (!createTo(dir, commandParser.value(options.at(0)))) {
                printf("Failed on create dci file for \"%s\"\n", qPrintable(dir));
            }
        }
    } else if (commandParser.isSet(options.at(1))) {
        for (const QString &dci : commandParser.positionalArguments()) {
            if (!exportTo(dci, commandParser.value(options.at(1)))) {
                printf("Failed on export the \"%s\" dci file\n", qPrintable(dci));
            }
        }
    } else if (commandParser.isSet(options.at(2))) {
        const QString &dci = commandParser.value(options.at(2));
        if (!tree(dci)) {
            printf("Failed on view the \"%s\" dci file\n", qPrintable(dci));
        }
    } else {
        commandParser.showHelp(-1);
    }

    return 0;
}
