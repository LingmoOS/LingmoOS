// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "worker.h"

Worker *Worker::Instance()
{
    static Worker *instance = new Worker;
    return instance;
}

bool Worker::isWaylandType()
{
    bool bRet = false;
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)){
        bRet = true;
    }
    else {
        bRet = false;
    }

    return bRet;
}

/*******************************************************************************
 1. @函数:    setDesktopMode
 2. @作者:
 3. @日期:    2020-12-15
 4. @说明:    设置桌面样式发送到dock栏
*******************************************************************************/
void Worker::setDesktopMode(Model::DesktopMode mode)
{
    QStringList args;
    switch (mode) {
        case Model::EfficientMode:
            args << "--print-reply"
                 << "--dest=com.deepin.dde.daemon.Launcher"
                 << "/com/deepin/dde/daemon/Launcher"
                 << "org.freedesktop.DBus.Properties.Set"
//                 << "string:com.deepin.dde.daemon.Launcher"
//                 << "string:Fullscreen"
                 << "variant:boolean:false";
            break;
        case Model::FashionMode:
            args << "--print-reply"
                 << "--dest=com.deepin.dde.daemon.Launcher"
                 << "/com/deepin/dde/daemon/Launcher"
                 << "org.freedesktop.DBus.Properties.Set"
//                 << "string:com.deepin.dde.daemon.Launcher"
//                 << "string:Fullscreen"
                 << "variant:boolean:true";
            break;
    }

    QProcess::startDetached("dbus-send", args);

    m_dockInter->setDisplayMode(mode);
}

/*******************************************************************************
 1. @函数:    setWMMode
 2. @作者:
 3. @日期:    2020-12-15
 4. @说明:    设置运行模式发送DBUS修改电脑的窗口特效打开或关闭 zyf修改成控制中心调用DBUS代码
*******************************************************************************/
void Worker::setWMMode(Model::WMType type)
{
    if (m_model->wmType() != type){
        bool value;
        type == Model::WMType::WM_2D ? value = false : value = true;

        QDBusInterface Interface("com.deepin.wm",
                                 "/com/deepin/wm",
                                 "org.freedesktop.DBus.Properties",
                                 QDBusConnection::sessionBus());
        QDBusMessage reply = Interface.call("Set","com.deepin.wm","compositingEnabled",
                                            QVariant::fromValue(QDBusVariant(value)));
         if (reply.type() == QDBusMessage::ErrorMessage) {
             qDebug() << "reply.type() = " << reply.type();
         }
     }
}

/*******************************************************************************
 1. @函数:    setIcon
 2. @作者:
 3. @日期:    2020-12-15
 4. @说明:    设置主题图标改变电脑的主题图标配置
*******************************************************************************/
void Worker::setIcon(const IconStruct &icon)
{
    m_iconInter->Set("icon", icon.Id);
}

/*******************************************************************************
 1. @函数:    onWMChanged
 2. @作者:
 3. @日期:    2020-12-15
 4. @说明:    运行模式变化保存到model槽
*******************************************************************************/
void Worker::onWMChanged(const QString &wm)
{
    m_model->setWmType(wm == "deepin wm" ? Model::WM_3D : Model::WM_2D);
}

/*******************************************************************************
 1. @函数:    onWMChang
 2. @作者:
 3. @日期:    2020-12-15
 4. @说明:    目前不使用，等后续研究
*******************************************************************************/
void Worker::onWMChang(/*const quint32 &wm*/)
{
    //Add by ut001000 renfeixiang 2020-12-06 删除代码，解决无法识别特效模式
    //（复现方法：打开欢迎从特效模式切换到普通模式再切换回特效模式，关闭欢迎，再打开欢迎，模式变成普通模式，并无法点击成切换换成特效模式）
    //m_model->setWmType(m_windowManage->windowManagerName() == DWindowManagerHelper::WMName::DeepinWM
    //                       ? Model::WM_3D
    //                       : Model::WM_2D);
}

/*******************************************************************************
 1. @函数:    onDisplayModeChanged
 2. @作者:
 3. @日期:    2020-12-15
 4. @说明:    桌面样式变化保存到model槽
*******************************************************************************/
void Worker::onDisplayModeChanged(int mode)
{
    m_model->setDesktopMode(static_cast<Model::DesktopMode>(mode));
}

/*******************************************************************************
 1. @函数:    onIconRefreshed
 2. @作者:
 3. @日期:    2020-12-15
 4. @说明:    图标刷新槽
*******************************************************************************/
void Worker::onIconRefreshed(const QString &name)
{
    if (name == "icon") {
        //QDBusPendingReply类包含对异步方法调用的答复。
        QDBusPendingReply<QString> icon = m_iconInter->List("icon");
        //QDBusPendingCallWatcher提供了一种等待异步响应的便捷方法
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(icon, this);
        //图片列表刷新响应函数
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &Worker::onIconClickChange);
    }
}

void Worker::onIconClickChange(QDBusPendingCallWatcher *w) {
    if (w->isError()) {
        qDebug() << w->error().message();
    } else {
        QDBusPendingReply<QString> reply = *w;
        onIconListChanged(reply.value());
    }
    w->deleteLater();
}

/*******************************************************************************
 1. @函数:    onIconListChanged
 2. @作者:
 3. @日期:    2020-12-16
 4. @说明:    图标主题列表变化响应槽
*******************************************************************************/
void Worker::onIconListChanged(const QString &value)
{
    const QJsonArray &array = QJsonDocument::fromJson(value.toUtf8()).array();

    QStringList currentIconIdList;
    for (const QJsonValue &value : array) {
        QDBusPendingReply<QString> icon =
            m_iconInter->Thumbnail("icon", value.toObject()["Id"].toString());
        const QJsonObject &obj = value.toObject();

        currentIconIdList << obj["Id"].toString();

        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(icon, this);
        watcher->setProperty("Json", obj);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, &Worker::onIconPixmapFinished);
    }

    for (const IconStruct &icon : m_model->iconList()) {
        if (!currentIconIdList.contains(icon.Id)) {
            m_model->removeIcon(icon);
        }
    }
}

/*******************************************************************************
 1. @函数:    onIconPixmapFinished
 2. @作者:
 3. @日期:    2020-12-16
 4. @说明:    图标图片完成响应槽
*******************************************************************************/
void Worker::onIconPixmapFinished(QDBusPendingCallWatcher *w)
{
    QDBusPendingReply<QString> reply = *w;
    if (!reply.isError()) {
        QJsonObject obj = w->property("Json").toJsonObject();
        obj["Pixmap"] = reply.value();
        m_model->addIcon(IconStruct::fromJson(obj));
    }
    w->deleteLater();
}

Worker::Worker(QObject *parent)
    : QObject(parent)
    , m_model(Model::Instance())
    , m_iconInter(new Icon("com.deepin.daemon.Appearance", "/com/deepin/daemon/Appearance",
                           QDBusConnection::sessionBus(), this))
    , m_wmInter(new WMSwitcher("com.deepin.WMSwitcher", "/com/deepin/WMSwitcher",
                               QDBusConnection::sessionBus(), this))
    , m_dockInter(new Dock("com.deepin.dde.daemon.Dock", "/com/deepin/dde/daemon/Dock",
                           QDBusConnection::sessionBus(), this))
{
    connect(m_iconInter, &Icon::Refreshed, this, &Worker::onIconRefreshed);
    connect(m_iconInter, &Icon::IconThemeChanged, m_model, &Model::setCurrentIcon);
    connect(m_wmInter, &WMSwitcher::WMChanged, this, &Worker::onWMChanged);
    connect(m_dockInter, &Dock::DisplayModeChanged, this, &Worker::onDisplayModeChanged);

    m_iconInter->setSync(false);
    m_wmInter->setSync(false);
    m_dockInter->setSync(false);

    m_model->setCurrentIcon(m_iconInter->iconTheme());
    m_windowManage = DWindowManagerHelper::instance();
    connect(m_windowManage, &DWindowManagerHelper::windowManagerChanged, this, &Worker::onWMChang);

    onWMChanged(m_wmInter->CurrentWM());
    onDisplayModeChanged(m_dockInter->displayMode());

    onIconRefreshed("icon");
}
