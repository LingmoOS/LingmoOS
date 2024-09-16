// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIPBOARDLOADER_H
#define CLIPBOARDLOADER_H

#include "constants.h"
#include "waylandcopyclient.h"
#include "iteminfo.h"

#include <QObject>
#include <QClipboard>
#include <QDBusMetaType>
#include <QIcon>
#include <QDateTime>
#include <QList>
#include <QDebug>
#include <QDBusArgument>
#include <QDateTime>
#include <QUrl>

class ClipboardLoader : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.deepin.dde.ClipboardLoader1")

public:
    explicit ClipboardLoader(QObject *parent = nullptr);

    bool cachePixmap(const QPixmap &srcPix, ItemInfo &info);
    void setImageData(const ItemInfo &info, QMimeData *&mimeData);

    static bool initPixPath();

public Q_SLOTS:
    void dataReborned(const QByteArray &buf);

private Q_SLOTS:
    void doWork(int protocolType);

Q_SIGNALS:
    void dataComing(const QByteArray &buf);

private:
    QClipboard *m_board;
    QByteArray m_lastTimeStamp;
    QPixmap m_lastPix;
    WaylandCopyClient *m_waylandCopyClient;

    static QString m_pixPath;
};

#endif // CLIPBOARDLOADER_H
