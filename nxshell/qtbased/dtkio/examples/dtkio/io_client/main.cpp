// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QMetaProperty>
#include <QDebug>

#include <DFile>
#include <DFileInfo>
#include <DFileOperator>

DIO_USE_NAMESPACE

QUrl localFileUrl(const QString &filename)
{
    QString filePath = QCoreApplication::applicationDirPath() + "/" + filename;
    return QUrl::fromLocalFile(filePath);
}

bool exist(const QUrl &url)
{
    DFile *file = new DFile(url);
    if (!file)
        return false;
    if (!file->exists()) {
        qInfo() << "file does not exist";
    } else {
        delete file;
        return true;
    }

    delete file;
    return false;
}

void hanldeResult(const DTK_CORE_NAMESPACE::DExpected<bool> &expected)
{
    if (expected) {
        if (expected.value())
            qInfo() << "Success";
        else
            qInfo() << "Fail";
    } else {
        qWarning() << expected.error();
    }
}

// copy file
void copyfile(const QString &fromFile, const QString &toFile)
{
    QUrl fromurl = localFileUrl(fromFile);
    QUrl toUrl = localFileUrl(toFile);

    DFileOperator *fileOperator = new DFileOperator(fromurl);
    if (!fileOperator)
        return;

    auto expected = fileOperator->copyFile(toUrl, CopyFlag::None);
    hanldeResult(expected);

    delete fileOperator;
}

// delete file
void deletefile(const QString &file)
{
    QUrl url = localFileUrl(file);

    if (!exist(url))
        return;

    DFileOperator *fileOperator = new DFileOperator(url);
    if (!fileOperator)
        return;
    auto expected = fileOperator->deleteFile();
    hanldeResult(expected);

    delete fileOperator;
}

// rename file
void renamefile(const QString &file, const QString &newName)
{
    QUrl url = localFileUrl(file);
    QUrl newUrl = localFileUrl(newName);

    if (!exist(url))
        return;

    DFileOperator *fileOperator = new DFileOperator(url);
    if (!fileOperator)
        return;
    auto expected = fileOperator->renameFile(newName);
    hanldeResult(expected);

    delete fileOperator;
}

// cat file
void catfile(const QString &fileName)
{
    QUrl url = localFileUrl(fileName);

    if (!exist(url))
        return;

    DFile *file = new DFile(url);
    if (!file)
        return;
    file->open(OpenFlag::ReadOnly);
    auto expected = file->readAll();
    file->close();
    if (expected) {
        qInfo() << expected.value();
    } else {
        qWarning() << expected.error();
    }

    delete file;
}

// move file
void movefile(const QString &file, const QString &toUrl)
{
    QUrl url = localFileUrl(file);
    QUrl destUrl = QUrl::fromLocalFile(toUrl);
    if (!exist(url))
        return;

    DFileOperator *fileOperator = new DFileOperator(url);
    if (!fileOperator)
        return;
    auto expected = fileOperator->moveFile(destUrl, CopyFlag::None);
    hanldeResult(expected);

    delete fileOperator;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // 创建命令行解析对象
    QCommandLineParser parser;
    // 定义帮助信息
    parser.setApplicationDescription(QString("%1 helper").arg(QCoreApplication::applicationName()));
    // 定义参数， addOption() 方法
    parser.addOption(QCommandLineOption("cat", "Cat files."));
    parser.addOption(QCommandLineOption("copy", "Copy files."));
    parser.addOption(QCommandLineOption("remove", "Delete files."));
    parser.addOption(QCommandLineOption("rename", "Rename files."));
    parser.addOption(QCommandLineOption("move", "Move files."));

    parser.addHelpOption();
    // 解析参数，parse() 方法
    parser.process(app);

    if (argc == 1)
        parser.showHelp();

    //执行IO相关操作
    {
        if (parser.isSet("remove")) {
            deletefile(argv[2]);
            exit(0);
        }

        if (parser.isSet("copy")) {
            copyfile(argv[2], argv[3]);
            exit(0);
        }

        if (parser.isSet("move")) {
            movefile(argv[2], argv[3]);
            exit(0);
        }

        if (parser.isSet("cat")) {
            catfile(argv[2]);
            exit(0);
        }

        if (parser.isSet("rename")) {
            renamefile(argv[2], argv[3]);
            exit(0);
        }
        parser.showHelp();
    }

    return app.exec();
}
