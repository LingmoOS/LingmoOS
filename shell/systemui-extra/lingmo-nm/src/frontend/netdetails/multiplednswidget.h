/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef MULTIPLEDNSWIDGET_H
#define MULTIPLEDNSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QAbstractItemView>
#include <QString>
#include <QList>
#include <QHostAddress>
#include <QDebug>

#include "listitemedit.h"
#include "dnssettingwidget.h"
#include "kborderlessbutton.h"
#include "kbuttonbox.h"

using namespace kdk;

class MultipleDnsWidget: public QWidget
{
    Q_OBJECT

public:
    MultipleDnsWidget(const QRegExp &rx, bool settingShow = true, QWidget *parent = nullptr);
    ~MultipleDnsWidget() = default;
    void setEditEnabled(bool state);
    QList<QHostAddress> getDns() const;
    void setDnsListText(const QList<QHostAddress> &dns);
    void setUuid(QString uuid) {
        m_uuid = uuid;
    }
    bool getDnsSettingsChanged() {
        return m_dnsSettingChanged;
    }

private:
    void initUI();
    void initComponent();
    void AddOneDnsItem(QListWidget *listWidget);
    void RemoveOneDnsItem(QListWidgetItem *aItem, QListWidget *listWidget);
    void showDnsSettingWidget();

    QLabel *m_mulDnsLabel;
    QLabel *emptyLabel;
    QFrame *m_emptyWidget;
    QListWidget  *m_dnsListWidget = nullptr;
    KPushButton *m_addDnsBtn;
    KPushButton *m_removeDnsBtn;
    KButtonBox *m_buttonBox;
    KBorderlessButton* m_settingsLabel;
    QRegExp m_regExp;
    QString m_uuid;
    bool m_settingShow;
    bool m_dnsSettingChanged = false;

private Q_SLOTS:
    void setDnsListWidgetStyle();
    void onAddBtnClicked();
    void onRemoveBtnClicked();

Q_SIGNALS:
    void scrollToBottom();
};

#endif // MULTIPLEDNSWIDGET_H
