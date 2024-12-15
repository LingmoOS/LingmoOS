// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PHONEWIDGET_H
#define PHONEWIDGET_H

#include "global_defines.h"
#include "discover/deviceinfo.h"
#include <QWidget>

class QStackedLayout;
namespace cooperation_core {
class NoNetworkWidget;
class DeviceListWidget;
class BottomLabel;
class QRCodeWidget;

class PhoneWidget : public QWidget
{
    Q_OBJECT
public:
    enum PageName {
        kQRCodeWidget = 0,
        kDeviceListWidget,
        kNoNetworkWidget,

        kUnknownPage = 99,
    };

    explicit PhoneWidget(QWidget *parent = nullptr);
    void initUI();
    void switchWidget(PageName page);

public slots:
    void setDeviceInfo(const DeviceInfoPointer info);
    void addOperation(const QVariantMap &map);
    void onSetQRcodeInfo(const QString &info);

private:
    QStackedLayout *stackedLayout { nullptr };
    CooperationSearchEdit *searchEdit { nullptr };
    NoNetworkWidget *nnWidget { nullptr };
    DeviceListWidget *dlWidget { nullptr };
    QRCodeWidget *qrcodeWidget { nullptr };
};

class QRCodeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QRCodeWidget(QWidget *parent = nullptr);

    void initUI();
    QPixmap generateQRCode(const QString &text, int scale = 10);
    void setQRcodeInfo(const QString &info);

private:
    QLabel *qrCode { nullptr };
};

}   // namespace cooperation_core

#endif   // PHONEWIDGET_H
