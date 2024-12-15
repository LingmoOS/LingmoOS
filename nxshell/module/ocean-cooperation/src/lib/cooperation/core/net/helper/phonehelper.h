// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PHONEHELPER_H
#define PHONEHELPER_H

#include "discover/deviceinfo.h"

#include <QSize>

namespace cooperation_core {
class MainWindow;
class ScreenMirroringWindow;
class PhoneHelper : public QObject
{
    Q_OBJECT

public:
    static PhoneHelper *instance();

    void registConnectBtn(MainWindow *window);
    static void buttonClicked(const QString &id, const DeviceInfoPointer info);
    static bool buttonVisible(const QString &id, const DeviceInfoPointer info);

    int notifyMessage(const QString &message, QStringList actions);
    void generateQRCode(const QString &ip, const QString &port, const QString &pin);
    void resetScreenMirroringWindow();

public Q_SLOTS:
    void onConnect(const DeviceInfoPointer info, int w, int h);
    void onDisconnect(const DeviceInfoPointer info);
    void onScreenMirroring();
    void onScreenMirroringStop();
    void onScreenMirroringResize(int w, int h);

Q_SIGNALS:
    void addMobileInfo(const DeviceInfoPointer info);
    void disconnectMobile();
    void setQRCode(const QString &code);

private:
    explicit PhoneHelper(QObject *parent = nullptr);
    ~PhoneHelper();
    DeviceInfoPointer m_mobileInfo { nullptr };
    ScreenMirroringWindow *m_screenwindow { nullptr };

    bool isInNotify =false;

    QSize m_viewSize;
};

}   // namespace cooperation_core

#endif   // PHONEHELPER_H
