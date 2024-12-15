// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mediawidget.h"
#include "util_updateui.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWheelEvent>

MediaWidget::MediaWidget(QWidget *parent) : QWidget(parent)
{
    initUI();
    initConnect();
}

void MediaWidget::initUI()
{
    m_dmprisWidget = new DMPRISControl;
    m_dmprisWidget->setAccessibleName("MPRISWidget");
    m_dmprisWidget->setPictureVisible(false);
    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setMargin(0);
    mainlayout->addWidget(m_dmprisWidget);

    setLayout(mainlayout);

    //updateStyle(":/skin/mediawidget.qss", this);
}

void MediaWidget::initConnect()
{
    connect(m_dmprisWidget, &DMPRISControl::mprisAcquired, this, &MediaWidget::changeVisible);
    connect(m_dmprisWidget, &DMPRISControl::mprisLosted, this, &MediaWidget::changeVisible);

    changeVisible();
}

void MediaWidget::initMediaPlayer()
{
    QDBusInterface dbusInter("org.freedesktop.DBus", "/", "org.freedesktop.DBus", QDBusConnection::sessionBus(), this);

    QDBusPendingCall call = dbusInter.asyncCall("ListNames");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, [ = ] {
        if (!call.isError()) {
            QDBusReply<QStringList> reply = call.reply();
            //qDebug() << "one key Login User Name is : " << reply.value();

            const QStringList &serviceList = reply.value();
            QString service = QString();
            for (const QString &serv : serviceList) {
                if (!serv.startsWith("org.mpris.MediaPlayer2.")) {
                    continue;
                }
                service = serv;
                break;
            }
            if (service.isEmpty()) {
                qDebug() << "media player dbus has not started, waiting for signal...";
                QDBusConnectionInterface *dbusDaemonInterface = QDBusConnection::sessionBus().interface();
                connect(dbusDaemonInterface, &QDBusConnectionInterface::serviceOwnerChanged, this,
                        [ = ](const QString &name, const QString &oldOwner, const QString &newOwner) {
                            Q_UNUSED(oldOwner)
                            if (name.startsWith("org.mpris.MediaPlayer2.") && !newOwner.isEmpty()) {
                                initMediaPlayer();
                                disconnect(dbusDaemonInterface);
                            }
                        });
                return;
            }

            qDebug() << "got media player dbus service: " << service;

            m_dbusInter = new DBusMediaPlayer2(service, "/org/mpris/MediaPlayer2", QDBusConnection::sessionBus(), this);

            m_dbusInter->MetadataChanged();
            m_dbusInter->PlaybackStatusChanged();
            m_dbusInter->VolumeChanged();
        } else {
            qWarning() << "init media player error: " << call.error().message();
        }

        watcher->deleteLater();
    });
}

void MediaWidget::changeVisible()
{
    const bool isWorking = m_dmprisWidget->isWorking();
    m_dmprisWidget->setVisible(isWorking);
}
