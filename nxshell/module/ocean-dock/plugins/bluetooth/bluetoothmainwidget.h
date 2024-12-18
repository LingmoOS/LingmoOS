// Copyright (C) 2022 ~ 2022 Lingmo Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef BLUETOOTHMAINWIDGET_H
#define BLUETOOTHMAINWIDGET_H

#include <DIconButton>

#include <QWidget>

class AdaptersManager;
class QLabel;
class Adapter;

DWIDGET_USE_NAMESPACE

class BluetoothMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BluetoothMainWidget(AdaptersManager *adapterManager, QWidget *parent = nullptr);
    ~BluetoothMainWidget();

Q_SIGNALS:
    void requestExpand();

protected:
    bool eventFilter(QObject *watcher, QEvent *event) override;

private:
    void initUi();
    void initConnection();

    void updateExpandIcon();

    bool isOpen() const;
    QString bluetoothIcon(bool isOpen) const;

private Q_SLOTS:
    void onAdapterChanged();
    void onPaletteChanged();

private:
    AdaptersManager *m_adapterManager;
    DIconButton *m_iconButton;
    QLabel *m_nameLabel;
    QLabel *m_stateLabel;
    QLabel *m_expandLabel;
    bool m_mouseEnter;
};

#endif // BLUETOOTHMAINWIDGET_H
