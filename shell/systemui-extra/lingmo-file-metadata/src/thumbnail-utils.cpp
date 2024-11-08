/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */

#include "thumbnail-utils.h"
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QCryptographicHash>
#include <QDateTime>
#include <QImage>

#include <QDebug>

using namespace LingmoUIFileMetadata;

QString thumbnailFailedDir(const QString& pluginName, const QString& version) {
    return QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/thumbnails/fail/" + QString("LingmoUIFileMetaData%0ThumbnailGenerator-%1").arg(pluginName, version);
}

int getTargetDir(const QSizeF& targetSize, QString& targetPath) {
    QString targetDir = "/normal";
    int length = 128;
    if (targetSize.isValid()) {
        qreal max = qMax(targetSize.width(), targetSize.height());
        if (max > 512) {
            targetDir = "/xx-large";
            length = 1024;
        } else if (max > 256) {
            targetDir = "/x-large";
            length = 512;
        } else if (max > 128) {
            length = 256;
            targetDir = "/large";
        }
    }
    targetPath = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + "/thumbnails" + targetDir;
    return length;
}

void generateThumbnail(const QString& url, QImage& thumbnail, QString& savePath, const QString& softwareName) {
    thumbnail.setText("Thumb::MTime", QString::number(QFileInfo(url).lastModified().toSecsSinceEpoch()));
    thumbnail.setText("Thumb::URI", QUrl::fromLocalFile(url).toString());
    thumbnail.setText("Software", softwareName);
    thumbnail.save(savePath, "png");
    qDebug() << softwareName <<  "generate thumbnail" << savePath;
}

void ThumbnailUtils::setThumbnail(ExtractionResult *result, QImage& thumbnail, const QString& pluginName, const QString& version) {
    if (thumbnail.format() & QImage::Format_Invalid) {
        handleFailureOfThumbnail(result->inputUrl(), pluginName, version);
        return;
    }
    QString softwareName = QString("LingmoUIFileMetaData%0ThumbnailGenerator-%1").arg(pluginName, version);

    QString md5 = QCryptographicHash::hash(QUrl::fromLocalFile(result->inputUrl()).toString().toUtf8(), QCryptographicHash::Md5).toHex();

    QString path;
    int thumbnailLength = getTargetDir(QSizeF(result->thumbnailRequest().targetSize()) / result->thumbnailRequest().devicePixelRatio(), path);
    if (!QFile::exists(path)) {
        QDir thumbnailDir(path);
        thumbnailDir.mkpath(path);
    }
    path += "/" + md5 + ".png";
    thumbnail = thumbnail.scaled(QSize{thumbnailLength, thumbnailLength}, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    generateThumbnail(result->inputUrl(), thumbnail, path, softwareName);
    result->addThumbnail(Thumbnail(thumbnail.scaled(result->thumbnailRequest().targetSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation)));
}

void ThumbnailUtils::handleFailureOfThumbnail(const QString& filePath, const QString& pluginName, const QString& version) {
    QString softwareName = QString("LingmoUIFileMetaData%0ThumbnailGenerator-%1").arg(pluginName, version);

    if (!QFile::exists(thumbnailFailedDir(pluginName, version))) {
        QDir failureDir(thumbnailFailedDir(pluginName, version));
        failureDir.mkpath(failureDir.path());
    }

    QString md5 = QCryptographicHash::hash(QUrl::fromLocalFile(filePath).toString().toUtf8(), QCryptographicHash::Md5).toHex();
    QString path = thumbnailFailedDir(pluginName, version) + "/" + md5 + ".png";

    QImage emptyPng;
    emptyPng.setText("Software", softwareName);
    emptyPng.setText("Thumb::MTime", QString::number(QFileInfo(filePath).lastModified().toSecsSinceEpoch()));
    emptyPng.save(path, "png");
}

bool ThumbnailUtils::needGenerateThumbnail(ExtractionResult *result, const QString& pluginName, const QString& version) {
    QString filePath = result->inputUrl();
    QString md5 = QCryptographicHash::hash(QUrl::fromLocalFile(filePath).toString().toUtf8(), QCryptographicHash::Md5).toHex();
    QString path;
    getTargetDir(QSizeF(result->thumbnailRequest().targetSize()) / result->thumbnailRequest().devicePixelRatio(), path);
    path = path + "/" + md5 + ".png";
    if (QFile::exists(path)) {
        QImage thumbnail(path);
        if (thumbnail.text("Thumb::MTime") == QString::number(QFileInfo(filePath).lastModified().toSecsSinceEpoch())) {
            result->addThumbnail(Thumbnail(thumbnail.scaled(result->thumbnailRequest().targetSize(), Qt::KeepAspectRatio)));
            qDebug() << "Thumbnail has already been generated!";
            return false;
        }
    } else {
        path = thumbnailFailedDir(pluginName, version) + "/" + md5 + ".png";
        if (QFile::exists(path)) {
            QImage failedThumbnail(path);
            if (failedThumbnail.text("Thumb::MTime") == QString::number(QFileInfo(filePath).lastModified().toSecsSinceEpoch())) {
                qDebug() << "Failed thumbnail has already been generated!";
                return false;
            }
        }
    }
    return true;
}
