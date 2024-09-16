// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "avatar.h"

#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

#include <unistd.h>

Avatar::Avatar(QWidget *parent)
    : QWidget(parent)
{
    m_accountsInter = new AccountsInter("org.deepin.dde.Accounts1",
                                        "/org/deepin/dde/Accounts1",
                                        QDBusConnection::systemBus(), this);

    m_userInter = new UserInter("org.deepin.dde.Accounts1",
                                QString("/org/deepin/dde/Accounts1/User%1").arg(getuid()),
                                QDBusConnection::systemBus(), this);

    this->setAccessibleDescription("This is the head image of the Launcher, which can quickly access the account in the control center");
    setFixedSize(32, 32);
    setFilePath(m_userInter->iconFile());

    connect(m_userInter, &UserInter::IconFileChanged, this, &Avatar::setFilePath);
}

void Avatar::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainterPath painterPath;
    painterPath.addRoundedRect(QRect(0, 0, width(), height()), 6, 6);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipPath(painterPath);

    painter.drawPixmap(e->rect(), m_avatarPixmap);
}

void Avatar::mouseReleaseEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);

    if (e->button() == Qt::LeftButton) {
        emit clicked();
    }
}

void Avatar::setFilePath(const QString &filePath)
{
    const auto ratio = devicePixelRatioF();

    QString avatarPath = filePath;
    if (ratio > 1.0) {
        avatarPath.replace("icons/", "icons/bigger/");
    }

    QUrl url(filePath);
    if (!QFile(url.toLocalFile()).exists()) {
        url = QUrl(filePath);
    }

    m_avatarPixmap = QPixmap(url.toLocalFile()).scaled(size() * ratio, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_avatarPixmap.setDevicePixelRatio(ratio);

    update();
}
