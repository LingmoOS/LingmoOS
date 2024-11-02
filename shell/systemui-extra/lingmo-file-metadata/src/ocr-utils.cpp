/*
    SPDX-FileCopyrightText: 2023 Junjie Bai <baijunjie@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "ocr-utils.h"
#include <QDebug>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <QStandardPaths>
#include <QUrl>
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>

using namespace LingmoUIFileMetadata;

static const QString VERSION = "0.0";

QString ocrFailedDir() {
    return QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QString("/OcrCache-%0/fail/").arg(VERSION);
}

void handleFailure(const QString &path) {
    if (!QFile::exists(ocrFailedDir())) {
        QDir failedDir(ocrFailedDir());
        failedDir.mkpath(ocrFailedDir());
    }

    QString md5 = QCryptographicHash::hash(QUrl::fromLocalFile(path).toString().toUtf8(), QCryptographicHash::Md5).toHex();
    QByteArray modifiedTime = QString::number(QFileInfo(path).lastModified().toSecsSinceEpoch()).toUtf8();
    QFile failedFile(ocrFailedDir() + md5 + ".txt");
    if (failedFile.exists()) {
        failedFile.open(QIODevice::ReadOnly);
        QByteArray lastModifiedTime = failedFile.readAll();
        failedFile.close();
        if (lastModifiedTime == modifiedTime) {
            return;
        }
    }
    failedFile.open(QIODevice::WriteOnly | QIODevice::Text);
    failedFile.write(modifiedTime);
    failedFile.close();
}

QString OcrUtils::getTextInPicture(const QString &path) {
    QString content;
    QString md5 = QCryptographicHash::hash(QUrl::fromLocalFile(path).toString().toUtf8(), QCryptographicHash::Md5).toHex();
    QFile failedFile(ocrFailedDir() + md5 + ".txt");
    if (failedFile.exists()) {
        failedFile.open(QIODevice::ReadOnly);
        if (failedFile.readAll() == QString::number(QFileInfo(path).lastModified().toSecsSinceEpoch()).toUtf8()) {
            qWarning() << "Ocr failed because failed record detected" << path;
            return content;
        }
    }

    auto *api = new tesseract::TessBaseAPI();
    if (api->Init(nullptr, "chi_sim")) {
        qWarning() << "Could not initialize tesseract.";
        return content;
    }
    api->SetVariable("user_defined_dpi", "1080");//图片中未标明分辨率的默认设置为1080

    Pix *image = pixRead(path.toStdString().data());
    if (!image) {
        handleFailure(path);
        if (api) {
            api->End();
            delete api;
            api = nullptr;
        }
        return content;
    }
    api->SetImage(image);
    char *tmp = api->GetUTF8Text();
    content = QString::fromLocal8Bit(tmp);
    delete [] tmp;
    pixDestroy(&image);
    api->Clear();

    if (api) {
        api->End();
        delete api;
        api = nullptr;
    }
    if(content.isEmpty()) {
        handleFailure(path);
    }
    return content;
}


