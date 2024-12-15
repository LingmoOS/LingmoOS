// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "phonehelper.h"
#include "utils/cooperationutil.h"
#include "../cooconstrants.h"
#include "../networkutil.h"
#include <functional>
#include <QString>
#include <QMetaType>
#include <QVariantMap>

#include <gui/mainwindow.h>
#include <gui/phone/screenmirroringwindow.h>

using namespace cooperation_core;

using ButtonStateCallback = std::function<bool(const QString &, const DeviceInfoPointer)>;
using ClickedCallback = std::function<void(const QString &, const DeviceInfoPointer)>;
Q_DECLARE_METATYPE(ButtonStateCallback)
Q_DECLARE_METATYPE(ClickedCallback)

inline constexpr char ConnectButtonId[] { "connect-button" };
inline constexpr char DisconnectButtonId[] { "disconnect-button" };

#ifdef linux
inline constexpr char Kconnect[] { "connect" };
inline constexpr char Kdisconnect[] { "disconnect" };
#else
inline constexpr char Kconnect[] { ":/icons/lingmo/builtin/texts/connect_18px.svg" };
inline constexpr char Kdisconnect[] { ":/icons/lingmo/builtin/texts/disconnect_18px.svg" };
#endif

inline constexpr char KprotocolVer[] { "1.0.0" };
inline constexpr char KdownloadUrl[] { "https://www.chinauos.com/resource/assistant" };

PhoneHelper::PhoneHelper(QObject *parent)
    : QObject(parent), m_viewSize(0, 0)
{
}

PhoneHelper::~PhoneHelper()
{
}

PhoneHelper *PhoneHelper::instance()
{
    static PhoneHelper ins;
    return &ins;
}

void PhoneHelper::registConnectBtn(MainWindow *window)
{
    ClickedCallback clickedCb = PhoneHelper::buttonClicked;
    ButtonStateCallback visibleCb = PhoneHelper::buttonVisible;

    QVariantMap DisconnectInfo { { "id", DisconnectButtonId },
                                 { "description", tr("Disconnect") },
                                 { "icon-name", Kdisconnect },
                                 { "location", 1 },
                                 { "button-style", 0 },
                                 { "clicked-callback", QVariant::fromValue(clickedCb) },
                                 { "visible-callback", QVariant::fromValue(visibleCb) } };

    window->addMobileOperation(DisconnectInfo);
    generateQRCode(CooperationUtil::localIPAddress(), QString::number(COO_SESSION_PORT), COO_HARD_PIN);
}

void PhoneHelper::onConnect(const DeviceInfoPointer info, int w, int h)
{
    m_viewSize.setWidth(w);
    m_viewSize.setHeight(h);

    m_mobileInfo = info;
    emit addMobileInfo(info);
}

void PhoneHelper::onScreenMirroring()
{
    //todo
    if (!m_mobileInfo)
        return;
    QString mes = QString(tr("“%1”apply to initiate screen casting")).arg(m_mobileInfo.data()->deviceName());
    QStringList actions;
    actions.append(tr("cancel"));
    actions.append(tr("comfirm"));

    int res = notifyMessage(mes, actions);
    if (res != 1)
        return;

    m_screenwindow = new ScreenMirroringWindow(m_mobileInfo.data()->deviceName());
    m_screenwindow->initSizebyView(m_viewSize);
    m_screenwindow->show();

    m_screenwindow->connectVncServer(m_mobileInfo.data()->ipAddress(), 5900, "");
}

void PhoneHelper::onScreenMirroringStop()
{
    resetScreenMirroringWindow();
}

void PhoneHelper::onScreenMirroringResize(int w, int h)
{
    if (!m_screenwindow)
        return;
    m_screenwindow->resize(w, h);
}

void PhoneHelper::onDisconnect(const DeviceInfoPointer info)
{
    if (!info)
        m_mobileInfo.reset();

    resetScreenMirroringWindow();

    emit disconnectMobile();

    if (m_mobileInfo && info && m_mobileInfo->ipAddress() == info->ipAddress()) {
        QString mes = QString(tr("“%1”connection disconnected!")).arg(m_mobileInfo.data()->deviceName());
        notifyMessage(mes, QStringList());
    }
}

int PhoneHelper::notifyMessage(const QString &message, QStringList actions)
{
    if (isInNotify)
        return -1;

    isInNotify = true;

    CooperationDialog dlg(qApp->activeWindow());
    dlg.setIcon(QIcon::fromTheme("ocean-cooperation"));
    dlg.setMessage(message);

    if (actions.isEmpty())
        actions.append(tr("comfirm"));

    dlg.addButton(actions.first(), false, CooperationDialog::ButtonNormal);

    if (actions.size() > 1)
        dlg.addButton(actions[1], true, CooperationDialog::ButtonRecommend);

    if (qApp->activeWindow()) {
        QWidget *activeWindow = qApp->activeWindow();
        QSize activeSize = activeWindow->size();
        QSize dlgSize = dlg.size();

        // 计算新位置，使对话框位于活动窗口的中心
        QPoint newPos = activeWindow->pos() + QPoint((activeSize.width() - dlgSize.width()) / 2, (activeSize.height() - dlgSize.height()) / 2);
        dlg.move(newPos);
    }

    int code = dlg.exec();

    isInNotify = false;
    return code;
}

void PhoneHelper::generateQRCode(const QString &ip, const QString &port, const QString &pin)
{
    QString combined = QString("host=%1&port=%2&pin=%3&pv=%4").arg(ip).arg(port).arg(pin).arg(KprotocolVer);

    QByteArray byteArray = combined.toUtf8();
    QByteArray base64 = byteArray.toBase64();
    QString qrContent = QString("%1?mark=%2").arg(KdownloadUrl).arg(QString::fromUtf8(base64));

    emit setQRCode(qrContent);
}

void PhoneHelper::resetScreenMirroringWindow()
{
    if (!m_screenwindow)
        return;

    m_screenwindow->deleteLater();
    m_screenwindow = nullptr;
}

void PhoneHelper::buttonClicked(const QString &id, const DeviceInfoPointer info)
{
    if (id == DisconnectButtonId) {
        QString mes = QString(tr("Are you sure to disconnect and collaborate with '%1'?")).arg(info.data()->deviceName());
        QStringList actions;
        actions.append(tr("cancel"));
        actions.append(tr("disconnect"));

        int res = PhoneHelper::instance()->notifyMessage(mes, actions);
        if (res != 1)
            return;

        NetworkUtil::instance()->disconnectRemote(info->ipAddress());
        PhoneHelper::instance()->onDisconnect(nullptr);
        return;
    }
}

bool PhoneHelper::buttonVisible(const QString &id, const DeviceInfoPointer info)
{
    if (id == ConnectButtonId && info->connectStatus() == DeviceInfo::ConnectStatus::Connectable)
        return true;

    if (id == DisconnectButtonId && info->connectStatus() == DeviceInfo::ConnectStatus::Connected)
        return true;

    return false;
}
