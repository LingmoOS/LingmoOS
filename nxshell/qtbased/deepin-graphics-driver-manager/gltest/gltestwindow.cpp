// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gltestwindow.h"
#include <QApplication>
#include <QKeyEvent>
#include <QScreen>

const QString GraphicMangerServiceName = "com.deepin.graphicmanger";
const QString GraphicMangerPath = "/com/deepin/graphicmanger";

GLTestWindow::GLTestWindow(QWidget *parent)
    : QWidget(parent)
{
    m_graphicsDriver = new ComDeepinDaemonGraphicsDriverInterface(
                GraphicMangerServiceName,
                GraphicMangerPath,
                QDBusConnection::systemBus(),
                nullptr);

    m_acceptBtn = new QPushButton(tr("Apply"));
    m_acceptBtn->setFocusPolicy(Qt::NoFocus);
    m_cancelBtn = new QPushButton(tr("Cancel"));
    m_cancelBtn->setFocusPolicy(Qt::NoFocus);
    m_tipsLabel = new QLabel(tr("Please ensure the driver works normally without blurred screen and screen tearing"));
    m_tipsLabel->setAlignment(Qt::AlignCenter);

    m_glTestWidget = new GLTestWidget;
    m_glTestWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *btnsLayout = new QHBoxLayout;
    btnsLayout->addStretch();
    btnsLayout->addWidget(m_cancelBtn);
    btnsLayout->addWidget(m_acceptBtn);
    btnsLayout->addStretch();

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_glTestWidget);
    centralLayout->addWidget(m_tipsLabel);
    centralLayout->addLayout(btnsLayout);

    setLayout(centralLayout);
    setFixedSize(qApp->primaryScreen()->geometry().size());
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    connect(m_acceptBtn, &QPushButton::clicked, this, &GLTestWindow::onAccept);
    connect(m_cancelBtn, &QPushButton::clicked, this, &GLTestWindow::onCancel);
}

void GLTestWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Escape:    qApp->quit();       break;
#endif
    default:;
    }
}


void GLTestWindow::onAccept()
{
    QDBusPendingReply<void> reply = m_graphicsDriver->TestSuccess();
    reply.waitForFinished();
    QProcess::startDetached("dbus-send --system --print-reply --dest=org.freedesktop.login1 /org/freedesktop/login1 org.freedesktop.login1.Manager.Reboot boolean:true");
    qApp->exit(0);
}

void GLTestWindow::onCancel()
{
    QDBusPendingReply<void> reply = m_graphicsDriver->CancelInstall();
    reply.waitForFinished();
    QProcess::startDetached("dbus-send --system --print-reply --dest=org.freedesktop.login1 /org/freedesktop/login1 org.freedesktop.login1.Manager.Reboot boolean:true");
    qApp->exit(-1);
}
