// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILETHUMBNAILPROVIDER_H
#define DFILETHUMBNAILPROVIDER_H

#include <dtkgui_global.h>
#include <DObject>

#include <QThread>
#include <QFileInfo>

#include <functional>

QT_BEGIN_NAMESPACE
class QMimeType;
QT_END_NAMESPACE

DGUI_BEGIN_NAMESPACE

class DThumbnailProviderPrivate;
class DThumbnailProvider : public QThread, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT

public:
    enum Size {
        Small = 64,
        Normal = 128,
        Large = 256,
    };

    static DThumbnailProvider *instance();

    bool hasThumbnail(const QFileInfo &info) const;
    bool hasThumbnail(const QMimeType &mimeType) const;

    QString thumbnailFilePath(const QFileInfo &info, Size size) const;

    QString createThumbnail(const QFileInfo &info, Size size);
    typedef std::function<void(const QString &)> CallBack;
    void appendToProduceQueue(const QFileInfo &info, Size size, CallBack callback = 0);
    void removeInProduceQueue(const QFileInfo &info, Size size);

    QString errorString() const;

    qint64 defaultSizeLimit() const;
    void setDefaultSizeLimit(qint64 size);

    qint64 sizeLimit(const QMimeType &mimeType) const;
    void setSizeLimit(const QMimeType &mimeType, qint64 size);

Q_SIGNALS:
    void thumbnailChanged(const QString &sourceFilePath, const QString &thumbnailPath) const;
    void createThumbnailFinished(const QString &sourceFilePath, const QString &thumbnailPath) const;
    void createThumbnailFailed(const QString &sourceFilePath) const;

protected:
    explicit DThumbnailProvider(QObject *parent = 0);
    ~DThumbnailProvider();

    void run() Q_DECL_OVERRIDE;

private:
    D_DECLARE_PRIVATE(DThumbnailProvider)
};

DGUI_END_NAMESPACE

#endif // DFILETHUMBNAILPROVIDER_H
