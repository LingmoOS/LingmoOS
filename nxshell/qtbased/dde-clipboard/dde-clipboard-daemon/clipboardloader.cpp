// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipboardloader.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDir>
#include <QStandardPaths>
#include <QImageReader>
#include <QImageWriter>

const QString PixCacheDir = QStringLiteral("/clipboard-pix");  // 图片缓存目录名
const int MAX_BETYARRAY_SIZE = 10*1024*1024;    // 最大支持的文本大小
const int X11_PROTOCOL = 0;                     // x11协议
const int WAYLAND_PROTOCOL = 1;                 // wayland协议

QByteArray Info2Buf(const ItemInfo &info)
{
    QByteArray buf;

    QByteArray iconBuf;
    if (info.m_formatMap.keys().contains("x-dfm-copied/file-icons")) {
        iconBuf = info.m_formatMap["x-dfm-copied/file-icons"];
    }

    QDataStream stream(&buf, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_11);
    stream << info.m_formatMap
           << info.m_type
           << info.m_urls
           << info.m_hasImage;
    if (info.m_hasImage) {
        stream << info.m_variantImage;
        stream << info.m_pixSize;
    }
    stream  << info.m_enable
            << info.m_text
            << info.m_createTime
            << iconBuf;

    return buf;
}

ItemInfo Buf2Info(const QByteArray &buf)
{
    QByteArray tempBuf = buf;

    ItemInfo info;

    QDataStream stream(&tempBuf, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_11);
    int type;
    QByteArray iconBuf;
    stream >> info.m_formatMap
           >> type
           >> info.m_urls
           >> info.m_hasImage;
    if (info.m_hasImage) {
        stream >> info.m_variantImage;
        stream >> info.m_pixSize;
    }

    stream >> info.m_enable
           >> info.m_text
           >> info.m_createTime
           >> iconBuf;

    QDataStream stream2(&iconBuf, QIODevice::ReadOnly);
    stream2.setVersion(QDataStream::Qt_5_11);
    for (int i = 0 ; i < info.m_urls.size(); ++i) {
        FileIconData data;
        stream2 >> data.cornerIconList >> data.fileIcon;
        info.m_iconDataList.push_back(data);
    }

    info.m_type = static_cast<DataType>(type);

    return info;
}

QString ClipboardLoader::m_pixPath;

ClipboardLoader::ClipboardLoader(QObject *parent)
    : QObject(parent)
    , m_board(nullptr)
    , m_waylandCopyClient(nullptr)
{
    if (qEnvironmentVariable("XDG_SESSION_TYPE").contains("wayland")) {
        m_waylandCopyClient = new WaylandCopyClient(this);
        m_waylandCopyClient->init();

        connect(m_waylandCopyClient, &WaylandCopyClient::dataChanged, this, [this] {
            this->doWork(WAYLAND_PROTOCOL);
        });
    } else {
        m_board = qApp->clipboard();
        connect(m_board, &QClipboard::dataChanged, this, [this] {
            this->doWork(X11_PROTOCOL);
        });
    }
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + PixCacheDir);
    if (dir.exists() && dir.removeRecursively()) {
        qDebug() << "ClipboardLoader startup, remove old cache, path:" << dir.path();
    }
}

void ClipboardLoader::dataReborned(const QByteArray &buf)
{
    ItemInfo info;
    info.m_variantImage = 0;
    info = Buf2Info(buf);

    QMimeData *mimeData = new QMimeData;
    QMapIterator<QString, QByteArray> it(info.m_formatMap);
    while (it.hasNext()) {
        it.next();
        mimeData->setData(it.key(), it.value());
    }

    switch (info.m_type) {
    case DataType::Image:
        setImageData(info, mimeData);
        break;
    default:
        break;
    }

    if (m_board)
        m_board->setMimeData(mimeData);

    if (m_waylandCopyClient)
        m_waylandCopyClient->setMimeData(mimeData);
}

void ClipboardLoader::doWork(int protocolType)
{
    ItemInfo info;
    info.m_variantImage = 0;

    // 快速复制时mimedata很可能是无效的(一般表现为获取的数据为空), 下面是qt的说明
    // The pointer returned might become invalidated when the contents
    // of the clipboard changes; either by calling one of the setter functions
    // or externally by the system clipboard changing.`
    const QMimeData *mimeData = protocolType == WAYLAND_PROTOCOL ? m_waylandCopyClient->mimeData() : m_board->mimeData();
    if (!mimeData || mimeData->formats().isEmpty())
        return;

    // 适配厂商云桌面粘贴问题
    if (mimeData->formats().contains("uos/remote-copy")) {
        qDebug() << "FROM_SHENXINFU_CLIPBOARD_MANAGER";
        return;
    }

    // 转移系统剪贴板所有权时造成的两次内容变化不需要显示，以下为与系统约定好的标识
    if (mimeData->data("FROM_DEEPIN_CLIPBOARD_MANAGER") == "1") {
        qDebug() << "FROM_DEEPIN_CLIPBOARD_MANAGER";
        return;
    }

    // 过滤重复数据
    QByteArray currTimeStamp = mimeData->data("TIMESTAMP");
    if (currTimeStamp == m_lastTimeStamp
            && m_lastTimeStamp != QByteArray::fromHex("00000000")// FIXME:TIM截图的时间戳不变，这里特殊处理
            && !currTimeStamp.isEmpty()) {// FIXME:连续双击两次图像，会异常到这里
        qDebug() << "TIMESTAMP:" << currTimeStamp << m_lastTimeStamp;
        return;
    }

    //图片类型的数据直接吧数据拿出来，不去调用mimeData->data()方法，会导致很卡
    if (mimeData->hasImage()) {
        const QPixmap &srcPix = qvariant_cast<QPixmap>(mimeData->imageData()) ;
        if (srcPix.isNull())
            return;

        info.m_pixSize = srcPix.size();
        if (!cachePixmap(srcPix, info)) {
            info.m_variantImage = srcPix;
        }

        info.m_formatMap.insert(ApplicationXQtImageLiteral, info.m_variantImage.toByteArray());
        info.m_formatMap.insert("TIMESTAMP", currTimeStamp);
        if (info.m_variantImage.isNull())
            return;

        // 正常数据时间戳不为空，这里增加判断限制 时间戳为空+图片内容不变 重复数据不展示
        // wayland下时间戳可能为空
        if(currTimeStamp.isEmpty() && srcPix.toImage() == m_lastPix.toImage() && !qEnvironmentVariable("XDG_SESSION_TYPE").contains("wayland")) {
            qDebug() << "system repeat image";
            return;
        }
        m_lastPix = srcPix;

        info.m_hasImage = true;
        info.m_type = Image;
    } else if (mimeData->hasUrls()) {
        info.m_urls = mimeData->urls();
        if (info.m_urls.isEmpty())
            return;

        //文件类型吧整个formats信息都拿出来，里面包含了文件的图标，以及文件的url数据等。
        for (const QString &format : mimeData->formats()) {
            const QByteArray &data = mimeData->data(format);
            if (!data.isEmpty())
                info.m_formatMap.insert(format, data);
        }
        if (info.m_formatMap.isEmpty())
            return;

        info.m_type = File;
    } else {
        if (mimeData->hasText()) {
            info.m_text = mimeData->text();
        } else if (mimeData->hasHtml()) {
            info.m_text = mimeData->html();
        } else {
            return;
        }

        const QByteArray &textByteArray = info.m_text.toUtf8();
        if (info.m_text.isEmpty() || textByteArray.size() > MAX_BETYARRAY_SIZE)
            return;

        info.m_formatMap.insert(TextPlainLiteral, std::move(textByteArray));
        info.m_type = Text;
    }

    info.m_createTime = QDateTime::currentDateTime();
    info.m_enable = true;

    m_lastTimeStamp = currTimeStamp;

    QByteArray buf;
    buf = Info2Buf(info);

    Q_EMIT dataComing(buf);
}

bool ClipboardLoader::cachePixmap(const QPixmap &srcPix, ItemInfo &info)
{
    if (initPixPath()) {
        QString pixFileName = m_pixPath + QString("/%1.png").arg(QDateTime::currentMSecsSinceEpoch());
        QImageWriter writer(pixFileName);
        if (!writer.canWrite() || !writer.write(srcPix.toImage())) {
            QFile cacheFile(pixFileName);
            if (!cacheFile.open(QIODevice::WriteOnly)) {
                qDebug() << "open file failed, file name:" << pixFileName;
                return false;
            }
            QDataStream stream(&cacheFile);
            stream.setVersion(QDataStream::Qt_5_11);
            stream << srcPix;
            cacheFile.close();
        }

        info.m_variantImage = srcPix.width() * PixmapHeight > srcPix.height() * PixmapWidth ?
                              srcPix.scaledToWidth(PixmapWidth, Qt::SmoothTransformation) :
                              srcPix.scaledToHeight(PixmapHeight, Qt::SmoothTransformation);

        // "text/uri-list":"file:///${XDG_CACHE_HOME}/deepin/dde-clipboard-daemon/clipboard-pix/xxx"
        info.m_formatMap.insert(TextUriListLiteral, QUrl::fromLocalFile(pixFileName).toEncoded());

        info.m_urls.push_back(QUrl::fromLocalFile(pixFileName));
        return true;
    }
    return false;
}

bool ClipboardLoader::initPixPath()
{
    if (!m_pixPath.isEmpty()) {
        return true;
    }

    QDir dir;
    m_pixPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + PixCacheDir;
    if (dir.exists(m_pixPath)) {
        qDebug() << "dir exists:" << m_pixPath;
        return true;
    }

    if (dir.mkdir(m_pixPath)) {
        qDebug() << "mkdir:" << m_pixPath;
        return true;
    }

    qDebug() << "mkdir failed:" << m_pixPath;
    m_pixPath.clear();
    return false;
}

void ClipboardLoader::setImageData(const ItemInfo &info, QMimeData *&mimeData)
{
    //正常处理的图片会有一个缓存url
    if (info.m_urls.size() != 1) {
        qDebug() << "url size error, size:" << info.m_urls.size();
        mimeData->setImageData(info.m_variantImage);
        return;
    }

    const QString &fileName = info.m_urls.front().path();
    QPixmap pix;
    QImageReader reader(fileName);
    if (reader.canRead())
        pix = QPixmap::fromImageReader(&reader);

    if (!pix.isNull()) {
        QFile cacheFile(fileName);
        if (!cacheFile.open(QIODevice::ReadOnly)) {
            qDebug() << "open pixmap cache file failed, file name:" << fileName;
            mimeData->setImageData(info.m_variantImage);
            return;
        }

        QDataStream stream(&cacheFile);
        stream.setVersion(QDataStream::Qt_5_11);
        stream >> pix;
        cacheFile.close();
    }

    if (pix.isNull()) {
        qDebug() << "read pixmap cache file failed, file name:" << fileName;
        mimeData->setImageData(info.m_variantImage);
        return;
    }

    mimeData->setImageData(pix);
}
