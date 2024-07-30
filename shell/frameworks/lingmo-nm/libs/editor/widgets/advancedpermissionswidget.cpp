/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "advancedpermissionswidget.h"
#include "ui_advancedpermissionswidget.h"

#include <KAcceleratorManager>
#include <KLocalizedString>
#include <KUser>
#include <QList>

#define UserTagRole Qt::UserRole + 1

class AdvancedPermissionsWidgetPrivate
{
public:
    Ui_AdvancedPermissions ui;
};

AdvancedPermissionsWidget::AdvancedPermissionsWidget(QWidget *parent)
    : QDialog(parent)
    , d_ptr(new AdvancedPermissionsWidgetPrivate)
{
    Q_D(AdvancedPermissionsWidget);
    d->ui.setupUi(this);

    for (const KUser &user : KUser::allUsers()) {
        if (user.userId().nativeId() >= 1000 && user.loginName() != QLatin1String("nobody"))
            d->ui.availUsers->addTopLevelItem(constructItem(user));
    }
    setupCommon();
}

AdvancedPermissionsWidget::AdvancedPermissionsWidget(const QHash<QString, QString> &users, QWidget *parent)
    : QDialog(parent)
    , d_ptr(new AdvancedPermissionsWidgetPrivate)
{
    Q_D(AdvancedPermissionsWidget);
    d->ui.setupUi(this);

    for (const KUser &user : KUser::allUsers()) {
        const QString name = user.loginName();
        if (!users.contains(name) && user.userId().nativeId() >= 1000 && user.loginName() != QLatin1String("nobody"))
            d->ui.availUsers->addTopLevelItem(constructItem(user));
        else if (users.contains(name))
            d->ui.currentUsers->addTopLevelItem(constructItem(user, users.value(name)));
    }
    setupCommon();
}

AdvancedPermissionsWidget::~AdvancedPermissionsWidget()
{
    Q_D(AdvancedPermissionsWidget);
    while (QTreeWidgetItem *item = d->ui.currentUsers->takeTopLevelItem(0)) {
        delete item;
    }
    while (QTreeWidgetItem *item = d->ui.availUsers->takeTopLevelItem(0)) {
        delete item;
    }
    delete d_ptr;
}

void AdvancedPermissionsWidget::setupCommon()
{
    Q_D(AdvancedPermissionsWidget);
    connect(d->ui.arrowLeft, &QPushButton::clicked, this, &AdvancedPermissionsWidget::leftArrowClicked);
    connect(d->ui.arrowRight, &QPushButton::clicked, this, &AdvancedPermissionsWidget::rightArrowClicked);
    d->ui.availUsers->sortByColumn(FullName, Qt::AscendingOrder);
    d->ui.currentUsers->sortByColumn(FullName, Qt::AscendingOrder);
    d->ui.availUsers->setSortingEnabled(true);
    d->ui.currentUsers->setSortingEnabled(true);

    KAcceleratorManager::manage(this);
}

QTreeWidgetItem *AdvancedPermissionsWidget::constructItem(const KUser &user, const QString &itemData)
{
    QStringList data;
    QString name = user.property(KUser::FullName).toString();
    QString nametooltip;
    if (name.isEmpty()) {
        name = i18nc("@item:intable shortcut for Not Available", "N/A");
        nametooltip = i18nc("@info:tooltip real user name is not available", "Not Available");
    } else {
        nametooltip = name;
    }
    data << name << user.loginName();
    auto item = new QTreeWidgetItem(data);
    item->setData(LoginName, UserTagRole, itemData);
    item->setToolTip(FullName, nametooltip);
    item->setToolTip(LoginName, user.loginName());
    return item;
}

QHash<QString, QString> AdvancedPermissionsWidget::currentUsers() const
{
    Q_D(const AdvancedPermissionsWidget);
    QHash<QString, QString> permissions;
    const int itemNumber = d->ui.currentUsers->topLevelItemCount();
    for (int i = 0; i < itemNumber; i++) {
        QTreeWidgetItem *item = d->ui.currentUsers->topLevelItem(i);
        QString username = item->data(LoginName, Qt::DisplayRole).toString();
        QString tags = item->data(LoginName, UserTagRole).toString();
        permissions.insert(username, tags);
    }
    return permissions;
}

void AdvancedPermissionsWidget::rightArrowClicked()
{
    Q_D(AdvancedPermissionsWidget);
    for (QTreeWidgetItem *item : d->ui.availUsers->selectedItems()) {
        const int index = d->ui.availUsers->indexOfTopLevelItem(item);
        d->ui.availUsers->takeTopLevelItem(index);
        d->ui.currentUsers->addTopLevelItem(item);
    }
}

void AdvancedPermissionsWidget::leftArrowClicked()
{
    Q_D(AdvancedPermissionsWidget);
    for (QTreeWidgetItem *item : d->ui.currentUsers->selectedItems()) {
        if (item->data(LoginName, Qt::DisplayRole) != KUser().loginName()) {
            const int index = d->ui.currentUsers->indexOfTopLevelItem(item);
            d->ui.currentUsers->takeTopLevelItem(index);
            d->ui.availUsers->addTopLevelItem(item);
        }
    }
}

#include "moc_advancedpermissionswidget.cpp"
