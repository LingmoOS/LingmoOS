// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICELISTWIDGET_H
#define DEVICELISTWIDGET_H

#include "global_defines.h"
#include "deviceitem.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QMouseEvent>
namespace cooperation_core {

class DeviceListWidget : public QScrollArea
{
    Q_OBJECT
public:
    explicit DeviceListWidget(QWidget *parent = nullptr);

    void appendItem(const DeviceInfoPointer info);
    void insertItem(int index, const DeviceInfoPointer info);
    void updateItem(int index, const DeviceInfoPointer info);
    void removeItem(int index);
    void moveItem(int srcIndex, int toIndex);
    int indexOf(const QString &ipStr);
    DeviceInfoPointer findDeviceInfo(const QString &ipStr);
    int itemCount();

    void groupItem(int index, const DeviceInfoPointer info);

    void addItemOperation(const QVariantMap &map);

    void clear();

private:
    void initUI();

private:
    QVBoxLayout *mainLayout { nullptr };
    QList<DeviceItem::Operation> operationList;

protected:
    bool event(QEvent *e) override;
};

}   // namespace cooperation_core

#endif   // DEVICELISTWIDGET_H
