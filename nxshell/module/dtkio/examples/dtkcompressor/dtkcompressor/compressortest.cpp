// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "compressortest.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>

DArchiveManager::MgrFileEntry CompressorTest::fileInfo2Entry(const QFileInfo &fileInfo)
{
    DArchiveManager::MgrFileEntry entry;

    entry.strFullPath = fileInfo.filePath();   // 文件全路径
    entry.strFileName = fileInfo.fileName();   // 文件名
    entry.isDirectory = fileInfo.isDir();   // 是否是文件夹

    if (entry.isDirectory) {
        // 文件夹显示子文件数目
        entry.qSize = QDir(fileInfo.filePath()).entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden).count();   //目录下文件数
    } else {
        // 文件直接显示大小
        entry.qSize = fileInfo.size();
    }

    entry.uLastModifiedTime = fileInfo.lastModified().toTime_t();   // 最后一次修改时间

    return entry;
}

CompressorTest::CompressorTest(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(800, 600);
    QVBoxLayout *vLay = new QVBoxLayout;
    QHBoxLayout *pFileLay = new QHBoxLayout;
    //压缩
    QLineEdit *fileLine = new QLineEdit(this);
    QPushButton *btnFileSel = new QPushButton("select file:");
    QPushButton *btnCompressor = new QPushButton("createArchive");
    connect(btnFileSel, &QPushButton::clicked, [=]() {
        const QString &file_path = QFileDialog::getOpenFileName(this);
        fileLine->setText(file_path);
    });

    connect(btnCompressor, &QPushButton::clicked, [=]() {
        DArchiveManager::MgrFileEntry entry = fileInfo2Entry(QFileInfo(fileLine->text()));
        QList<DArchiveManager::MgrFileEntry> sList;
        sList << entry;
        DArchiveManager::MgrCompressOptions options;
        options.iCompressionLevel = 3;
        DArchiveManager::get_instance()->createArchive(sList, QFileInfo(fileLine->text()).path() + QDir::separator() + "test.tar.gz", options, DArchiveManager::APT_Libarchive);
    });
    //解压
    QLineEdit *UnCompfileLine = new QLineEdit(this);
    QPushButton *btnUnCompFileSel = new QPushButton("UnComp select file:");
    QPushButton *btnUnCompressor = new QPushButton("extractFiles");
    connect(btnUnCompFileSel, &QPushButton::clicked, [=]() {
        const QString &file_path = QFileDialog::getOpenFileName(this);
        UnCompfileLine->setText(file_path);
    });

    connect(btnUnCompressor, &QPushButton::clicked, [=]() {
        DArchiveManager::MgrFileEntry entry = fileInfo2Entry(QFileInfo(UnCompfileLine->text()));
        QList<DArchiveManager::MgrFileEntry> sList;
        sList << entry;
        DArchiveManager::MgrCompressOptions options;
        options.iCompressionLevel = 3;
        DArchiveManager::get_instance()->loadArchive(UnCompfileLine->text());
    });
    connect(DArchiveManager::get_instance(), &DArchiveManager::signalJobFinished, [=](DArchiveManager::ArchiveJobType eJobType, DArchiveManager::MgrPluginFinishType eFinishType, DArchiveManager::MgrErrorType eErrorType) {
        if (eJobType == DArchiveManager::JT_Load) {
            DArchiveManager::MgrExtractionOptions options;
            DArchiveManager::MgrArchiveData *stArchiveData = DArchiveManager::get_instance()->archiveData();
            // 构建解压参数
            options.strTargetPath = QFileInfo(UnCompfileLine->text()).path() + QDir::separator() + "extractFilesTest";
            options.bAllExtract = true;
            options.qSize = stArchiveData->qSize;
            options.qComressSize = stArchiveData->qComressSize;
            DArchiveManager::get_instance()->extractFiles(UnCompfileLine->text(), QList<DArchiveManager::MgrFileEntry>(), options);
        }
        if (eJobType == DArchiveManager::JT_Create) {
            QMessageBox::information(this, "压缩", "压缩成功");
        }
        if (eJobType == DArchiveManager::JT_Extract) {
            QMessageBox::information(this, "解压", "解压成功");
        }
    });

    pFileLay->addWidget(btnFileSel);
    pFileLay->addWidget(fileLine);
    pFileLay->addWidget(btnCompressor);
    QHBoxLayout *pUncomFileLay = new QHBoxLayout;
    pUncomFileLay->addWidget(btnUnCompFileSel);
    pUncomFileLay->addWidget(UnCompfileLine);
    pUncomFileLay->addWidget(btnUnCompressor);
    vLay->addStretch();
    vLay->addLayout(pFileLay);
    vLay->addLayout(pUncomFileLay);
    vLay->addStretch();
    setLayout(vLay);
}
