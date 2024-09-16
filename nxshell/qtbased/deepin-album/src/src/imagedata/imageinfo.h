// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEINFO_H
#define IMAGEINFO_H

#include <QObject>
#include <QUrl>
#include <QSharedPointer>

class ImageInfoData;
class ImageInfo : public QObject
{
    Q_OBJECT
    Q_ENUMS(Status)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int type READ type NOTIFY typeChanged)
    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(int frameIndex READ frameIndex WRITE setFrameIndex NOTIFY frameIndexChanged)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY frameCountChanged)
    Q_PROPERTY(bool exists READ exists NOTIFY existsChanged)
    Q_PROPERTY(bool hasCachedThumbnail READ hasCachedThumbnail)

public:
    explicit ImageInfo(QObject *parent = nullptr);
    explicit ImageInfo(const QUrl &source, QObject *parent = nullptr);
    ~ImageInfo() override;

    enum Status { Null, Ready, Loading, Error };
    Status status() const;
    Q_SIGNAL void statusChanged();

    void setSource(const QUrl &source);
    QUrl source() const;
    Q_SIGNAL void sourceChanged();

    int type() const;
    Q_SIGNAL void typeChanged();

    int width() const;
    Q_SIGNAL void widthChanged();
    int height() const;
    Q_SIGNAL void heightChanged();
    void swapWidthAndHeight();

    void setFrameIndex(int index);
    int frameIndex() const;
    Q_SIGNAL void frameIndexChanged();
    int frameCount() const;
    Q_SIGNAL void frameCountChanged();

    bool exists() const;
    Q_SIGNAL void existsChanged();
    bool hasCachedThumbnail() const;

    Q_SIGNAL void infoChanged();
    Q_INVOKABLE void reloadData();

    void clearCurrentCache();
    static void clearCache();

protected:
    void setStatus(Status status);
    bool updateData(const QSharedPointer<ImageInfoData> &newData);
    void refreshDataFromCache(bool reload = false);
    Q_SLOT void onLoadFinished(const QString &path, int frameIndex = 0);
    Q_SLOT void onSizeChanged(const QString &path, int frameIndex = 0);

protected:
    QUrl imageUrl;
    Status imageStatus = Null;
    int currentIndex = 0;
    QSharedPointer<ImageInfoData> data;
};

#endif  // IMAGEINFO_H
