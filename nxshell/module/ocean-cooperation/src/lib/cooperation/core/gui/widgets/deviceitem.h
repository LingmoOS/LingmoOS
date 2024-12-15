// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICEITEM_H
#define DEVICEITEM_H

#include "global_defines.h"
#include "discover/deviceinfo.h"
#include "backgroundwidget.h"
#include "common/qtcompat.h"

#include <QIcon>
#include <QMap>

namespace cooperation_core {

class ButtonBoxWidget;
class StateLabel : public CooperationLabel
{
    Q_OBJECT
public:
    explicit StateLabel(QWidget *parent = nullptr);

    void setState(DeviceInfo::ConnectStatus state) { st = state; }
    DeviceInfo::ConnectStatus state() const { return st; }

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    DeviceInfo::ConnectStatus st;
};

class DeviceItem : public BackgroundWidget
{
    Q_OBJECT
public:
    using ButtonStateCallback = std::function<bool(const QString &, const DeviceInfoPointer)>;
    using ClickedCallback = std::function<void(const QString &, const DeviceInfoPointer)>;
    struct Operation
    {
        QString id;
        QString description;
        QString icon;
        int location;
        int style;
        ButtonStateCallback visibleCb;
        ButtonStateCallback clickableCb;
        ClickedCallback clickedCb;
    };

    explicit DeviceItem(QWidget *parent = nullptr);
    ~DeviceItem() override;

    void setDeviceInfo(const DeviceInfoPointer info);
    DeviceInfoPointer deviceInfo() const;

    void setOperations(const QList<Operation> &operations);
    void updateOperations();

public Q_SLOTS:
    void onButtonClicked(int index);

protected:
    void enterEvent(EnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initUI();
    void initConnect();
    void setDeviceName(const QString &name);
    void setDeviceStatus(DeviceInfo::ConnectStatus status);

private:
    CooperationLabel *iconLabel { nullptr };
    CooperationLabel *nameLabel { nullptr };
    CooperationLabel *ipLabel { nullptr };
    StateLabel *stateLabel { nullptr };
    ButtonBoxWidget *btnBoxWidget { nullptr };

    QMap<int, Operation> indexOperaMap;
    DeviceInfoPointer devInfo;
};

}   // namespace cooperation_core

Q_DECLARE_METATYPE(cooperation_core::DeviceItem::ButtonStateCallback)
Q_DECLARE_METATYPE(cooperation_core::DeviceItem::ClickedCallback)

#endif   // DEVICEITEM_H
