// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusdraw_adaptor.h"

#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QWidget>
#include <QDebug>

dbusdraw_adaptor::dbusdraw_adaptor(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

dbusdraw_adaptor::~dbusdraw_adaptor()
{
    // destructor
}

bool dbusdraw_adaptor::openFiles(QList<QVariant> filePaths)
{
    //将画板窗口最小化，点击启动器中的画板激活窗口
    if (filePaths.isEmpty()) {
        QWidget *pMainWidget = qobject_cast<QWidget *>(parent());
        if (pMainWidget != nullptr) {
            pMainWidget->show();
            pMainWidget->raise();
            pMainWidget->activateWindow();
        }
    }

    QStringList paths;
    foreach (QVariant path, filePaths) {
        paths << path.toString().toLocal8Bit();
    }

    QMetaObject::invokeMethod(parent(), "openFiles",
                              Q_ARG(QStringList, paths), Q_ARG(bool, true));
    return true;
}

//void dbusdraw_adaptor::openImages(QList<QVariant> images)
//{
//    for (QVariant img : images) {
//        QByteArray data = img.toByteArray();
//        QString tmp_data = QString::fromLatin1(data.data(), data.size());
//        //data = QByteArray::fromBase64(tmp_data.toLatin1());
//        //data = qUncompress(data);
//        QByteArray srcData = QByteArray::fromBase64(tmp_data.toLatin1());
//        data = qUncompress(srcData);
//        QImage image;
//        image.loadFromData(data);
//        QMetaObject::invokeMethod(parent(), "openImage",
//                                  Q_ARG(QImage, image), Q_ARG(const QByteArray &, srcData));
//    }
//}

bool dbusdraw_adaptor::openFile(QString filePath)
{
    QString _filePath;
    QFileInfo fInfo(filePath);
    if (fInfo.exists() && fInfo.isFile()) {
        _filePath = filePath;
    } else {
        QUrl url(filePath);
        if (url.isLocalFile()) {
            _filePath = url.toLocalFile();
        }
    }

    QWidget *pMainWidget = qobject_cast<QWidget *>(parent());

    if (pMainWidget != nullptr) {
        pMainWidget->show();
        pMainWidget->raise();
        pMainWidget->activateWindow();

        if (!_filePath.isEmpty()) {
            QStringList paths;
            paths.append(_filePath);
            QMetaObject::invokeMethod(parent(), "openFiles",
                                      Q_ARG(QStringList, paths), Q_ARG(bool, true));
            return true;
        }
    }
    return false;
}

