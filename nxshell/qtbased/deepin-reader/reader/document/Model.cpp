// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Model.h"
#include "PDFModel.h"
#include "DjVuModel.h"
#include "dpdfannot.h"
#include "dpdfpage.h"
#include "dpdfdoc.h"

#include <QProcess>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QTemporaryFile>

namespace deepin_reader {
deepin_reader::Document *deepin_reader::DocumentFactory::getDocument(const int &fileType,
                                                                     const QString &filePath,
                                                                     const QString &convertedFileDir,
                                                                     const QString &password,
                                                                     QProcess **pprocess,
                                                                     deepin_reader::Document::Error &error)
{
    deepin_reader::Document *document = nullptr;

    qDebug() << "需要转换的文档: " << filePath;
    if (Dr::PDF == fileType) {
        qDebug() << "当前文档类型为: PDF";
        document = deepin_reader::PDFDocument::loadDocument(filePath, password, error);
    } else if (Dr::DJVU == fileType) {
        qDebug() << "当前文档类型为: DJVU";
        document = deepin_reader::DjVuDocument::loadDocument(filePath, error);
    } else if (Dr::DOCX == fileType) {
        qDebug() << "当前文档类型为: DOCX";
        if (nullptr == pprocess) {
            error = deepin_reader::Document::ConvertFailed;
            return nullptr;
        }
        QString targetDoc = convertedFileDir + "/temp.docx";
        QString tmpHtmlFilePath = convertedFileDir + "/word/temp.html";
        QString realFilePath = convertedFileDir + "/temp.pdf";

        // QDir convertedDir(convertedFileDir);
        // if (!convertedDir.exists()) {
        //     bool flag = convertedDir.mkdir(convertedFileDir);
        //     qDebug() << "创建文件夹" << convertedFileDir << "是否成功？" << flag;
        // } else {
        //     qDebug() << "文件夹" << convertedFileDir << "已经存在！";
        // }
        // qDebug() << "targetDoc: " << targetDoc;
        // qDebug() << "tmpHtmlFilePath: " << tmpHtmlFilePath;
        // qDebug() << "realFilePath: " << realFilePath;

        QString prefix = INSTALL_PREFIX;
        // qDebug() << "应用安装路径: " << prefix;

        QFile file(filePath);
        if (!file.copy(targetDoc)) {
            qInfo() << QString("copy %1 failed.").arg(filePath);
            error = deepin_reader::Document::ConvertFailed;
            return nullptr;
        }

        //解压的目的是为了让资源文件可以被转换的时候使用到，防止转换后丢失图片等媒体信息
        QProcess decompressor;
        *pprocess = &decompressor;
        decompressor.setWorkingDirectory(convertedFileDir);
        qDebug() << "正在解压文档..." << targetDoc;
        QString unzipCommand = "unzip " + targetDoc;
        qDebug() << "执行命令: " << unzipCommand;
        decompressor.start(unzipCommand);
        if (!decompressor.waitForStarted()) {
            qInfo() << "start unzip failed";
            error = deepin_reader::Document::ConvertFailed;
            *pprocess = nullptr;
            return nullptr;
        }
        if (!decompressor.waitForFinished()) {
            qInfo() << "unzip failed";
            error = deepin_reader::Document::ConvertFailed;
            *pprocess = nullptr;
            return nullptr;
        }
        if (!QDir(convertedFileDir + "/word").exists()) {
            qInfo() << "unzip failed! " << (convertedFileDir + "/word") << "is not exists!";
            error = deepin_reader::Document::ConvertFailed;
            if (!(QProcess::CrashExit == decompressor.exitStatus() && 9 == decompressor.exitCode())) {
                *pprocess = nullptr;
            }
            return nullptr;
        }
        qDebug() << "文档解压完成";
        QTemporaryFile tmpFile(convertedFileDir + "/word/" + QCoreApplication::applicationName() + "_XXXXXX.html");
        if( tmpFile.open()) { //fix 232871
             tmpHtmlFilePath = tmpFile.fileName(); // returns the unique file name
         }
        // docx -> html
        QProcess converter;
        *pprocess = &converter;
        converter.setWorkingDirectory(convertedFileDir + "/word");
        qDebug() << "正在将docx文档转换成html..." << tmpHtmlFilePath;
        // QFile targetDocFile(targetDoc);
        // if (targetDocFile.exists()) {
        //     qDebug() << "文档" << targetDocFile.fileName() << "存在！";
        // } else {
        //     qDebug() << "文档" << targetDocFile.fileName() << "不存在！";
        // }
        QString pandocDataDir = prefix + "/share/pandoc/data";
        QString pandocCommand = QString("pandoc %1 --data-dir=%2 -o %3").arg(targetDoc).arg(pandocDataDir).arg(tmpHtmlFilePath);

        qDebug() << "执行命令: " << pandocCommand;
        converter.start(pandocCommand);
        if (!converter.waitForStarted()) {
            qInfo() << "start pandoc failed";
            error = deepin_reader::Document::ConvertFailed;
            *pprocess = nullptr;
            return nullptr;
        }
        if (!converter.waitForFinished()) {
            qInfo() << "pandoc failed";
            error = deepin_reader::Document::ConvertFailed;
            *pprocess = nullptr;
            return nullptr;
        }
        QFile tmpHtmlFile(tmpHtmlFilePath);
        if (!tmpHtmlFile.exists()) {
            qInfo() <<  "pandoc failed! " << tmpHtmlFilePath << " doesn't exist";
            error = deepin_reader::Document::ConvertFailed;
            // 转换过程中关闭应用，docsheet被释放，对应的*pprocess已不存在
            if (!(QProcess::CrashExit == converter.exitStatus() && 9 == converter.exitCode())) {
                *pprocess = nullptr;
            }
            return nullptr;
        }
        qDebug() << "docx转html完成";

        // html -> pdf
        QProcess converter2;
        *pprocess = &converter2;
        converter2.setWorkingDirectory(convertedFileDir + "/word");
        qDebug() << "正在将html转换成pdf..." << realFilePath;

        QString htmltopdfCommand = prefix + "/lib/deepin-reader/htmltopdf " +  tmpHtmlFilePath + " " + realFilePath;
        qDebug() << "执行命令: " << htmltopdfCommand;
        converter2.start(htmltopdfCommand);
        if (!converter2.waitForStarted()) {
            qInfo() << "start htmltopdf failed";
            error = deepin_reader::Document::ConvertFailed;
            *pprocess = nullptr;
            return nullptr;
        }
        if (!converter2.waitForFinished()) {
            qInfo() << "htmltopdf failed";
            error = deepin_reader::Document::ConvertFailed;
            *pprocess = nullptr;
            return nullptr;
        }

        QFile realFile(realFilePath);
        if (!realFile.exists()) {
            qInfo() <<  "htmltopdf failed! " << realFilePath << " doesn't exist";
            error = deepin_reader::Document::ConvertFailed;
            if (!(QProcess::CrashExit == converter.exitStatus() && 9 == converter.exitCode())) {
                *pprocess = nullptr;
            }
            return nullptr;
        }
        qDebug() << "html转pdf完成";

        *pprocess = nullptr;
        document = deepin_reader::PDFDocument::loadDocument(realFilePath, password, error);
    }

    return document;
}

bool SearchResult::setctionsFillText(std::function<QString(int, QRectF)> getText)
{
    bool ret = false;
    for (auto &section : sections) {
        for (auto &line : section) {
            //这里的page比index大1
            int index = page - 1;
            QString text = getText(index, line.rect);
            if (!text.isEmpty()) {
                line.text = text;
                ret = true;
            }
        }
    }
    return ret;
}

QRectF SearchResult::sectionBoundingRect(const PageSection &section)
{
    QRectF ret;
    for (const PageLine &line : section) {
        ret = ret.united(line.rect);
    }
    return ret;
}

}
