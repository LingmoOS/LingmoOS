// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "timewidget.h"
#include "dde-dock/constants.h"

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DStyle>
#include <DSysInfo>

#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QPixmap>
#include <QThread>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QPainterPath>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

DCORE_USE_NAMESPACE

#define RECORDER_TIME_LEVEL_ICON_SIZE 23
#define RECORDER_TIME_VERTICAL_ICON_SIZE 16
#define RECORDER_TIME_FONT DFontSizeManager::instance()->t8()
#define RECORDER_PADDING 1

TimeWidget::TimeWidget(DWidget *parent):
    DWidget(parent),
    m_timer(nullptr),
    m_dockInter(nullptr),
    m_lightIcon(nullptr),
    m_shadeIcon(nullptr),
    m_currentIcon(nullptr),
    m_bRefresh(true),
    m_position(Dock::Position::Bottom),
    m_hover(false),
    m_pressed(false)
{
    setContentsMargins(0, 0, 0, 0);

    auto *layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    m_iconLabel = new QLabel(this);
    m_textLabel = new QLabel(this);
    layout->addWidget(m_iconLabel);
    layout->addWidget(m_textLabel);

    m_textLabel->setFont(RECORDER_TIME_FONT);
    m_textLabel->setText("00:00:00");
    QPalette textPalette = m_textLabel->palette();
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        textPalette.setColor(QPalette::WindowText, Qt::black);
    }else{
        textPalette.setColor(QPalette::WindowText, Qt::white);
    }
    m_textLabel->setPalette(textPalette);
    m_textLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_timer = new QTimer(this);
    m_dockInter = new DBusDock("com.deepin.dde.daemon.Dock", "/com/deepin/dde/daemon/Dock", QDBusConnection::sessionBus(), this);
    connect(m_dockInter, &DBusDock::PositionChanged, this, &TimeWidget::onPositionChanged);
    m_position = m_dockInter->position();
    m_lightIcon = new QIcon(":/res/1070/light.svg");
    m_shadeIcon = new QIcon(":/res/1070/shade.svg");

    m_currentIcon = m_lightIcon;

    updateIcon();

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

}

TimeWidget::~TimeWidget()
{
    if (nullptr != m_lightIcon) {
        delete m_lightIcon;
        m_lightIcon = nullptr;
    }
    if (nullptr != m_shadeIcon) {
        delete m_shadeIcon;
        m_shadeIcon = nullptr;
    }
    if (nullptr != m_timer) {
        m_timer->deleteLater();
        m_timer = nullptr;
    }
    if (nullptr != m_dockInter) {
        m_dockInter->deleteLater();
        m_dockInter = nullptr;
    }
}

bool TimeWidget::enabled()
{
    return isEnabled();
}

void TimeWidget::onTimeout()
{
    if (m_bRefresh) {
        if (m_currentIcon == m_lightIcon)
            m_currentIcon = m_shadeIcon;
        else
            m_currentIcon = m_lightIcon;
    }
    m_bRefresh = !m_bRefresh;
    QTime showTime(0, 0, 0);
    int time = m_baseTime.secsTo(QTime::currentTime());
    showTime = showTime.addSecs(time);
    m_textLabel->setText(showTime.toString("hh:mm:ss"));
    updateIcon();
}

void TimeWidget::updateIcon() {
    if (Dock::Position::Top == m_position || Dock::Position::Bottom == m_position) {
        m_pixmap = QIcon::fromTheme(QString("recordertime"), *m_currentIcon).pixmap(QSize(RECORDER_TIME_VERTICAL_ICON_SIZE, RECORDER_TIME_VERTICAL_ICON_SIZE));
    } else {
        m_pixmap = QIcon::fromTheme(QString("recordertime"), *m_currentIcon).pixmap(QSize(RECORDER_TIME_VERTICAL_ICON_SIZE, RECORDER_TIME_VERTICAL_ICON_SIZE));
    }

    m_iconLabel->setPixmap(m_pixmap);
}

void TimeWidget::onPositionChanged(int value)
{
    qInfo() << "====================== onPositionChanged ====start=================";
    m_position = value;

    if (m_position == Dock::Position::Top || m_position == Dock::Position::Bottom) {
        m_textLabel->show();
    } else {
        m_textLabel->hide();
    }

    qInfo() << "====================== onPositionChanged ====end=================";
}

void TimeWidget::paintEvent(QPaintEvent *e)
{
    //qInfo() << "====================== paintEvent ====start=================";
    QPainter painter(this);
    //qInfo() <<  ">>>>>>>>>> rect().width(): " << rect().width() << " , this->height(): " << this->height();

    if (rect().height() > PLUGIN_BACKGROUND_MIN_SIZE) {
        QColor color;
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
            color = Qt::black;
            painter.setOpacity(0);
            if (m_hover) {
                painter.setOpacity(0.6);
            }

            if (m_pressed) {
                painter.setOpacity(0.3);
            }
        } else {
            color = Qt::white;
            painter.setOpacity(0);
            if (m_hover) {
                painter.setOpacity(0.2);
            }

            if (m_pressed) {
                painter.setOpacity(0.05);
            }
        }
        painter.setRenderHint(QPainter::Antialiasing, true);
        DStyleHelper dstyle(style());
        const int radius = dstyle.pixelMetric(DStyle::PM_FrameRadius);
        QPainterPath path;
        if ((Dock::Position::Top == m_position || Dock::Position::Bottom == m_position) ||
            ((Dock::Position::Right == m_position || Dock::Position::Left == m_position) && rect().width() > RECORDER_TIME_LEVEL_ICON_SIZE)) {
            QRect rc(RECORDER_PADDING, RECORDER_PADDING, rect().width() - RECORDER_PADDING * 2, rect().height() - RECORDER_PADDING * 2);
            path.addRoundedRect(rc, radius, radius);
        }
        painter.fillPath(path, color);
    }

    QWidget::paintEvent(e);
    //qInfo() << "====================== paintEvent ====end=================";
}

void TimeWidget::mousePressEvent(QMouseEvent *e)
{
    qDebug() << "Click the taskbar plugin! To start!";
    m_pressed = true;
    update();
    QWidget::mousePressEvent(e);
    qDebug() << "Click the taskbar plugin! The end!";
}
//创建缓存文件，只有wayland模式下的mips或部分arm架构适用
bool TimeWidget::createCacheFile()
{
    qDebug() << "createCacheFile start!";
    QString userName = QDir::homePath().section("/", -1, -1);
    std::string path = ("/home/" + userName + "/.cache/deepin/deepin-screen-recorder/").toStdString();
    QDir tdir(path.c_str());
    //判断文件夹路径是否存在
    if (!tdir.exists()) {
        tdir.mkpath(path.c_str());
    }
    path += "stopRecord.txt";
    //判断文件是否存在，若存在则先删除文件
    QFile mFile(path.c_str());
    if (mFile.exists()) {
        remove(path.c_str());
    }
    //打开文件
    int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        qDebug() << "open file fail!" << strerror(errno);
        return false;
    }
    //文件加锁
    int flock = lockf(fd, F_TLOCK, 0);
    if (flock == -1) {
        qDebug() << "lock file fail!" << strerror(errno);
        return false;
    }
    ssize_t ret = -1;
    //文件内容为1，读取文件时会停止录屏
    char wBuffer[2] = {"1"};
    //写文件
    ret = write(fd, wBuffer, 2);
    if (ret < 0) {
        qDebug() << "write file fail!";
        return false;
    }
    flock = lockf(fd, F_ULOCK, 0);
    if (flock == -1) {
        qDebug() << "unlock file fail!" << strerror(errno);
        return false;
    }
    ::close(fd);
    qDebug() << "createCacheFile end!";
    return true;

}

void TimeWidget::mouseReleaseEvent(QMouseEvent *e)
{
    qDebug() << "Mouse release start!";
    if(e->button() == Qt::LeftButton && m_pressed && m_hover){
        m_pressed = false;
        m_hover = false;
        update();
        QWidget::mouseReleaseEvent(e);
        return;
    }
    int width = rect().width();
    bool flag = true;
    if (e->pos().x() > 0 && e->pos().x() < width) {
#if  defined (__mips__) || defined (__sw_64__) || defined (__loongarch_64__) || defined (__aarch64__) || defined (__loongarch__)
        if (isWaylandProtocol()) {
            flag = false;
            if(!createCacheFile()){
                qInfo() << "Create cache file fail!";
                flag = true;
            };
        }
#endif
        if (flag) {
            qDebug() << "Click the taskbar plugin! Dbus call stop recording screen!";
            QDBusInterface notification(QString::fromUtf8("com.deepin.ScreenRecorder"),
                                        QString::fromUtf8("/com/deepin/ScreenRecorder"),
                                        QString::fromUtf8("com.deepin.ScreenRecorder"),
                                        QDBusConnection::sessionBus());
            notification.asyncCall("stopRecord");
            //        QDBusMessage message = notification.call("stopRecord"); //会阻塞任务其他按钮的执行
        }
    }
    m_pressed = false;
    m_hover = false;
    update();
    QWidget::mouseReleaseEvent(e);
    qDebug() << "Mouse release end!";

}

void TimeWidget::mouseMoveEvent(QMouseEvent *e)
{
    m_hover = true;
    update();
    QWidget::mouseMoveEvent(e);
}

void TimeWidget::leaveEvent(QEvent *e)
{
    m_hover = false;
    m_pressed = false;
    update();
    QWidget::leaveEvent(e);
}

void TimeWidget::start()
{
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    m_baseTime = QTime::currentTime();
    m_timer->start(400);
}

void TimeWidget::stop()
{
    disconnect(m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}
//判断是否是wayland协议
bool TimeWidget::isWaylandProtocol()
{
    QProcessEnvironment e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    return XDG_SESSION_TYPE == QLatin1String("wayland") ||  WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
}

void TimeWidget::showEvent(QShowEvent *e)
{
    // 强制重新刷新 sizePolicy 和 size
    onPositionChanged(m_position);
    DWidget::showEvent(e);
}
