// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../global_defines.h"
#include "../discover/deviceinfo.h"

#include <QScopedPointer>

namespace cooperation_core {

class MainWindowPrivate;
class MainWindow : public CooperationMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // DeviceInfoPointer findDeviceInfo(const QString &ip);

#if defined(_WIN32) || defined(_WIN64)
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
#endif
    void closeEvent(QCloseEvent *event) override;

    void minimizedAPP();
#ifdef ENABLE_PHONE
    void addMobileOperation(const QVariantMap &map);
#endif

signals:
    void refreshDevices();
    void searchDevice(const QString &ip);

public Q_SLOTS:
    void setFirstTipVisible();
    void onlineStateChanged(const QString &validIP);
    void onLookingForDevices();
    void onFindDevice(const QString &ip);
    void onDiscoveryFinished(bool hasFound);
    void addDevice(const QList<DeviceInfoPointer> &infoList);
    void removeDevice(const QString &ip);
    void onRegistOperations(const QVariantMap &map);
    void onSwitchMode(CooperationMode mode);

#ifdef ENABLE_PHONE
    //mobile
    void addMobileDevice(const DeviceInfoPointer info);
    void disconnectMobile();
    void onSetQRCode(const QString &code);
#endif

protected:
    void showCloseDialog();

private:
    QScopedPointer<MainWindowPrivate> d;

    bool _userAction { false };
};

}   // namespace cooperation_core

#endif   // MAINWINDOW_H
