// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datahander.h"
#include "sitemdata.h"
#include "dialog.h"
#include "pagecontext.h"
#include "setting.h"


#include <QFileInfo>
#include <QDir>
#include <QImageWriter>
#include <QImageReader>
#include <QApplication>
#include <QDesktopWidget>

DataHander::DataHander(QObject *parent) : QObject(parent)
{

}

bool DataHander::isLegalFile(const QString &path)
{
    if (path.isEmpty()) {
        return false;
    }

#ifdef Q_OS_WIN
    QRegExp regExp("[\\*\\?\"<>\\|]");
#else
    QRegExp regExp("[:\\*\\?\"<>\\|]");
#endif

    if (path.contains(regExp)) {
        return false;
    }

    QRegExp splitExp("[/\\\\]");


    int pos = splitExp.indexIn(path, 0);

    while (pos != -1) {
        QString dirStr = path.left(pos + 1);
        if (dirStr.count() > 1) {
            QDir dir(dirStr);
            if (!dir.exists()) {
                return false;
            }
        }
        pos = splitExp.indexIn(path, pos + 1);
    }

    bool isdir = (path.endsWith('/') || path.endsWith('\\'));
    return !isdir;
}

QString DataHander::toLegalFile(const QString &filePath)
{
    QString result = filePath;
    QFileInfo info(filePath);
    if (!info.exists()) {
        QUrl url(filePath);
        if (url.isLocalFile()) {
            result = url.toLocalFile();
        }
    }

    if (!isLegalFile(result))
        return "";

    return result;
}

bool DataHander::checkFileBeforeSave(const QString &file)
{
    QFileInfo info(file);
    if (!isSupportedFile(file)) {
        messageUnSupport(file);
        setError(EUnSupportFile, tr("Unable to open \"%1\", unsupported file format").arg(info.fileName()));
        return false;
    }


    if (info.exists() && !info.isWritable()) {
        messageUnWritable(file);
        setError(EUnWritableFile, tr("This file is read-only, please save with another name"));
        return false;
    }

    return true;
}

bool DataHander::checkFileBeforeLoad(const QString &file)
{
    QString legeFile = toLegalFile(file);

    QFileInfo info(legeFile);
    if (!info.exists()) {
        messageFileNotExist(legeFile);
        setError(EFileNotExist, tr("The file does not exist"));
        return false;
    }

    if (!isSupportedFile(legeFile)) {
        messageUnSupport(file);
        setError(EUnSupportFile, tr("Unable to open \"%1\", unsupported file format").arg(info.fileName()));
        return false;
    }

    if (!info.isReadable()) {
        messageUnReadable(legeFile);
        setError(EUnReadableFile, tr("Unable to open the write-only file \"%1\"").arg(info.fileName()));
        return false;
    }

    return true;
}

void DataHander::messageUnSupport(const QString &file)
{
    QFileInfo info(file);
    QString infoName = info.fileName();
    emit message(tr("Unable to open \"%1\", unsupported file format").arg(infoName));
}

void DataHander::messageUnReadable(const QString &file)
{
    QFileInfo info(file);
    QString infoName = info.fileName();
    emit message(tr("Unable to open the write-only file \"%1\"").arg(infoName));
}

void DataHander::messageUnWritable(const QString &file)
{
    emit message(tr("This file is read-only, please save with another name"));
}

void DataHander::messageFileNotExist(const QString &file)
{
    emit message(tr("The file does not exist"));
}

void DataHander::messageMd5valid(const QString &file)
{
    QFileInfo info(file);
    QString infoName = info.fileName();
    emit message(tr("Unable to open the broken file \"%1\"").arg(infoName));
}

void DataHander::setError(int error, const QString &errorString)
{
    _error = error;
    _errorString = errorString;
}

int DataHander::error() const
{
    return _error;
}

QString DataHander::errorString() const
{
    return _errorString;
}

void DataHander::unsetError()
{
    setError(0, "");
}

bool ImageHander::isSupportedFile(const QString &file)
{
    QFileInfo info(file);
    auto stuffix = info.suffix().toLower();
    auto allSupSuffix = Setting::instance()->readableFormats();
    allSupSuffix.removeFirst();
    return allSupSuffix.contains(stuffix);
}

ImageHander::ImageHander(QObject *parent): DataHander(parent)
{

}

QImage loadImage_helper(const QString &path)
{
    auto fOptimalConditions = [ = ](const QImageReader & reader) {
        QSize orgSz = reader.size();
        return (orgSz.width() > 1920 && orgSz.height() > 1080);
    };

    QImageReader reader;
    reader.setFileName(path);
    bool shouldOptimal = fOptimalConditions(reader);
    qreal radio = shouldOptimal ? 0.5 : 1.0;
    QSize orgSize = reader.size();
    QSize newSize = orgSize * radio;
    reader.setScaledSize(newSize);

    if (shouldOptimal)
        reader.setQuality(30);

    //当不能读取数据时,设置以内容判断格式进行加载,这样可以避免后缀名修改后文件无法打开的情况
    if (!reader.canRead()) {
        reader.setAutoDetectImageFormat(true);
        reader.setDecideFormatFromContent(true);
        reader.setFileName(path); //必须重新设置一下文件,才能触发内部加载方式的切换
    }

    if (reader.canRead()) {
        QImage img = reader.read();
        auto desktop = QApplication::desktop();
        if (Q_NULLPTR != desktop && img.logicalDpiX() != desktop->logicalDpiX()) {//图片Dpi值与屏幕会导致在图片上绘制位置错误
            img.setDotsPerMeterX(qRound(desktop->logicalDpiX() * 100 / 2.54));
            img.setDotsPerMeterY(qRound(desktop->logicalDpiY() * 100 / 2.54));
        }

        //维持原大小
        bool haveOptimal = shouldOptimal;
        if (haveOptimal) {
            img = img.scaled(orgSize);
        }
        return img;
    }
    return QImage();
}

QImage ImageHander::load(const QString &file)
{
    if (checkFileBeforeLoad(file)) {
        emit progressBegin("");
        QImage img = loadImage_helper(file);
        // 设置图片加载错误信息
        if (img.isNull()) {
            setError(EDamagedFile, tr("Damaged file, unable to open it"));
        }
        img = img.convertToFormat(QImage::Format_ARGB32);
        emit progressEnd(0, "");
        return img;
    }
    return QImage();
}

bool ImageHander::save(const QImage &img, const QString &file, int qulit)
{
    if (checkFileBeforeSave(file)) {
        emit progressBegin("");
        QFileInfo info(file);
        bool b = img.save(file, info.suffix().toUtf8(), qulit);
        emit progressEnd(0, "");
        return b;
    }
    return false;
}

//bool ImageHander::save(const QString &file,
//                       const QVariant &saveObj,
//                       const QList<QVariant> &params)
//{
//    auto image = saveObj.value<QImage>();
//    return save(image, file, params.first().toInt());
//}

//QVariant ImageHander::load(const QString &file, const QList<QVariant> &params)
//{
//    return load(file);
//}
