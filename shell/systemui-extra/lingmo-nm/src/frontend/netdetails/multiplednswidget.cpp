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
#include "multiplednswidget.h"

#include <QApplication>
#include <QDBusInterface>
#include <QPlainTextEdit>
#include <QGSettings>

#include "lingmostylehelper/lingmostylehelper.h"
#include "coninfo.h"

#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"

#define DNS_LISTWIDGET_EMPTY_HEIGHT 79
#define DNS_LISTWIDGET_HEIGHT 188
#define BUTTON_SIZE 36,36
#define ITEM_HEIGHT 36

#define STR_ATTEMPTS "attempts"
#define STR_TIMEOUT "timeout"
#define STR_TYPE "type"

MultipleDnsWidget::MultipleDnsWidget(const QRegExp &rx, bool settingShow, QWidget *parent)
    : m_regExp(rx),
      m_settingShow(settingShow),
      QWidget(parent)
{
    initUI();
    initComponent();
}

void MultipleDnsWidget::initUI()
{
    QVBoxLayout *mulDnsVLayout = new QVBoxLayout(this);
    mulDnsVLayout->setContentsMargins(0, 0, 0, 30);

    m_mulDnsLabel = new QLabel(this);
    m_mulDnsLabel->setText(tr("DNS server(Drag to sort)")); //DNS服务器：

    m_emptyWidget = new QFrame(this);
    m_emptyWidget->setFrameShape(QFrame::Shape::StyledPanel);
    m_emptyWidget->setFixedHeight(DNS_LISTWIDGET_EMPTY_HEIGHT);
    emptyLabel = new QLabel(m_emptyWidget);
    emptyLabel->setAlignment(Qt::AlignCenter);
    emptyLabel->setText(tr("Click \"+\" to configure DNS"));
    QVBoxLayout* emptyLayout = new QVBoxLayout(m_emptyWidget);
    emptyLayout->addWidget(emptyLabel,Qt::AlignCenter);

    m_dnsListWidget = new QListWidget(this);
    m_dnsListWidget->setFixedHeight(DNS_LISTWIDGET_EMPTY_HEIGHT);
    m_dnsListWidget->setBackgroundRole(QPalette::Base);
    m_dnsListWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_dnsListWidget->setFrameShape(QFrame::Shape::StyledPanel);
    m_dnsListWidget->setEditTriggers(QAbstractItemView::DoubleClicked);

    //item可拖拽
    m_dnsListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    m_dnsListWidget->setDragEnabled(true);
    m_dnsListWidget->viewport()->setAcceptDrops(true);
    m_dnsListWidget->setDropIndicatorShown(true);
    m_dnsListWidget->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);

    setDnsListWidgetStyle();

    m_buttonBox = new KButtonBox(this);
    m_buttonBox->setExclusive(false);

    m_addDnsBtn = new KPushButton(this);
    m_addDnsBtn->setIcon(QIcon::fromTheme("list-add-symbolic"));
    m_addDnsBtn->setFixedSize(BUTTON_SIZE);
    m_addDnsBtn->setProperty("useButtonPalette", true);
    m_removeDnsBtn = new KPushButton(this);
    m_removeDnsBtn->setIcon(QIcon::fromTheme("list-remove-symbolic"));
    m_removeDnsBtn->setFixedSize(BUTTON_SIZE);
    m_removeDnsBtn->setProperty("useButtonPalette", true);
    m_removeDnsBtn->setEnabled(false);
    m_buttonBox->addButton(m_addDnsBtn);
    m_buttonBox->addButton(m_removeDnsBtn);


    m_settingsLabel = new KBorderlessButton(this);
    m_settingsLabel->setText(tr("Settings"));

    QHBoxLayout *btnHLayout = new QHBoxLayout();
    btnHLayout->setContentsMargins(0, 0, 0, 0);
    btnHLayout->setSpacing(1);
    btnHLayout->setAlignment(Qt::AlignLeft);

    btnHLayout->addWidget(m_buttonBox);
    btnHLayout->addSpacing(23);
    btnHLayout->addWidget(m_settingsLabel);

    mulDnsVLayout->addWidget(m_mulDnsLabel, Qt::AlignLeft);
    mulDnsVLayout->addWidget(m_emptyWidget);
    mulDnsVLayout->addWidget(m_dnsListWidget);
    mulDnsVLayout->addLayout(btnHLayout);

    m_emptyWidget->show();
    m_dnsListWidget->hide();

    if (!m_settingShow) {
        m_settingsLabel->hide();
    }
}

void MultipleDnsWidget::initComponent()
{
    connect(qApp, &QApplication::paletteChanged, this, &MultipleDnsWidget::setDnsListWidgetStyle);
    connect(m_addDnsBtn, &KPushButton::clicked, this, &MultipleDnsWidget::onAddBtnClicked);
    connect(m_removeDnsBtn, &KPushButton::clicked, this, &MultipleDnsWidget::onRemoveBtnClicked);
    connect(m_dnsListWidget, &QListWidget::itemClicked, this, [=]() {
        if (m_dnsListWidget->count() < 1) {
            m_removeDnsBtn->setEnabled(false);
        } else {
            m_removeDnsBtn->setEnabled(true);
        }
    });
    connect(m_dnsListWidget, &QListWidget::itemDoubleClicked, this, [=](QListWidgetItem *item) {
        m_dnsListWidget->edit(m_dnsListWidget->currentIndex());
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    });

    connect(m_settingsLabel, &KBorderlessButton::clicked, this, [&](){
       showDnsSettingWidget();
    });
}

void MultipleDnsWidget::setEditEnabled(bool state)
{
    m_addDnsBtn->setEnabled(state);

    if (!state) {
        m_dnsListWidget->clear();
    }
}

QList<QHostAddress> MultipleDnsWidget::getDns() const
{
    QStringList dnsList;
    dnsList.clear();
    QList<QHostAddress> ipv4dnsList;
    ipv4dnsList.clear();
    int row = 0;
    QString aDns;
    while (m_dnsListWidget->count() > row) {
        aDns = m_dnsListWidget->item(row)->text();
        if (!dnsList.contains(aDns)) {
            dnsList << aDns;
            ipv4dnsList << QHostAddress(aDns);
        }
        row ++;
    }
    return ipv4dnsList;
}

void MultipleDnsWidget::setDnsListText(const QList<QHostAddress> &dns)
{
    m_dnsListWidget->clear();
    if (!dns.isEmpty()) {
        m_dnsListWidget->setFixedHeight(DNS_LISTWIDGET_HEIGHT);
        m_emptyWidget->hide();
        m_dnsListWidget->show();
    }
    for (int i = 0; i < dns.size(); ++i) {
        QString str = dns.at(i).toString();
        QListWidgetItem *dnsListWidgetItem = new QListWidgetItem(m_dnsListWidget);
        dnsListWidgetItem->setSizeHint(QSize(0,ITEM_HEIGHT));

        ListItemEdit *dnsListItemEdit = new ListItemEdit(m_regExp, m_dnsListWidget);
        m_dnsListWidget->setItemDelegateForRow(i, dnsListItemEdit);
        dnsListWidgetItem->setText(str);
    }
}

void MultipleDnsWidget::AddOneDnsItem(QListWidget *listWidget)
{
    if (m_dnsListWidget->count() == 0) {
        m_emptyWidget->hide();
        m_dnsListWidget->show();
        m_dnsListWidget->setFixedHeight(DNS_LISTWIDGET_HEIGHT);
        Q_EMIT scrollToBottom();
    }
    QListWidgetItem *dnsListWidgetItem = new QListWidgetItem(listWidget);
    dnsListWidgetItem->setSizeHint(QSize(0,ITEM_HEIGHT));
    dnsListWidgetItem->setFlags(dnsListWidgetItem->flags() | Qt::ItemIsEditable);
    listWidget->addItem(dnsListWidgetItem);
    listWidget->setCurrentItem(dnsListWidgetItem);

    ListItemEdit *dnsListItemEdit = new ListItemEdit(m_regExp, listWidget);
    listWidget->setItemDelegateForRow(listWidget->currentIndex().row() , dnsListItemEdit);
    listWidget->editItem(dnsListWidgetItem);
}

void MultipleDnsWidget::RemoveOneDnsItem(QListWidgetItem *aItem, QListWidget *listWidget)
{
    if (aItem) {
        listWidget->removeItemWidget(aItem);
        delete aItem;
    }

    if (m_dnsListWidget->count() == 0) {
        m_emptyWidget->show();
        m_dnsListWidget->hide();
        m_dnsListWidget->setFixedHeight(DNS_LISTWIDGET_EMPTY_HEIGHT);
    }
}

void MultipleDnsWidget::setDnsListWidgetStyle()
{
    QPalette pal = qApp->palette();
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
        QGSettings styleGsettings(style_id);
        QString currentTheme = styleGsettings.get(COLOR_THEME).toString();
        if(currentTheme == "lingmo-default"){
            pal = lightPalette(this);
        }
    }

    this->setPalette(pal);

    m_dnsListWidget->setAlternatingRowColors(true);

    QColor color = pal.color(QPalette::PlaceholderText);
    pal.setColor(QPalette::WindowText, color);
    emptyLabel->setPalette(pal);
}

void MultipleDnsWidget::onAddBtnClicked()
{
    AddOneDnsItem(m_dnsListWidget);

    //避免重复添加空白项
    int row = m_dnsListWidget->count() - 1;
    while (row >= 0) {
        if (!m_dnsListWidget->item(row)->isSelected()
                && m_dnsListWidget->item(row)->text().isEmpty()) {
            m_dnsListWidget->removeItemWidget(m_dnsListWidget->item(row));
            delete m_dnsListWidget->item(row);
        }
        row --;
    }
    m_removeDnsBtn->setEnabled(true);
}

void MultipleDnsWidget::onRemoveBtnClicked()
{
    QListWidgetItem *aItem = m_dnsListWidget->currentItem();
    if (!aItem) {
        return;
    }
    RemoveOneDnsItem(aItem, m_dnsListWidget);
    if (m_dnsListWidget->count()< 1) {
        m_removeDnsBtn->setEnabled(false);
    }
}

void MultipleDnsWidget::showDnsSettingWidget()
{
    QDBusInterface iface("com.lingmo.network.enhancement.optimization",
                         "/com/lingmo/network/enhancement/optimization/DNS",
                         "com.lingmo.network.enhancement.optimization.DNS",
                         QDBusConnection::systemBus());

    if (!iface.isValid()) {
        return;
    }

    QDBusMessage result = iface.call("GetExtraDns", m_uuid);
    const QDBusArgument &dbusArg1st = result.arguments().at( 0 ).value<QDBusArgument>();
    QVariantMap map = result.arguments().at(0).toMap();
    QString timeout, retry, tactic;
    dbusArg1st >> map;

    QString originTimeout,originRetry,originType;
    originTimeout = map.value(STR_TIMEOUT).toString();
    originRetry = map.value(STR_ATTEMPTS).toString();
    originType = map.value(STR_TYPE).toString();

    timeout = !originTimeout.isEmpty() ? map.value(STR_TIMEOUT).toString() : "5";
    retry = !originRetry.isEmpty() ? map.value(STR_ATTEMPTS).toString() : "2";
    tactic = !originType.isEmpty() ? map.value(STR_TYPE).toString() : "order";

    DnsSettingWidget* dialog = new DnsSettingWidget(timeout, retry, tactic);
    kdk::LingmoUIStyleHelper::self()->removeHeader(dialog);
    if (dialog->exec() == QDialog::Accepted) {
        QString timeout, retry, tactic;
        dialog->getDnsSettings(timeout, retry, tactic);
        if (iface.isValid()) {
            iface.call("SetOptions", m_uuid, timeout, retry, tactic);
        }
        if (timeout != originTimeout || retry != originRetry || tactic != originType) {
            m_dnsSettingChanged = true;
        }
    }
    delete dialog;
    dialog = nullptr;
}
