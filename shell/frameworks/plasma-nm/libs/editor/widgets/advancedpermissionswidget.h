/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_ADVANCED_PERMISSIONS_WIDGET_H
#define PLASMA_NM_ADVANCED_PERMISSIONS_WIDGET_H

#include <QDialog>
#include <QHash>

class QTreeWidgetItem;
class AdvancedPermissionsWidgetPrivate;
class KUser;

class AdvancedPermissionsWidget : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AdvancedPermissionsWidget)
public:
    explicit AdvancedPermissionsWidget(QWidget *parent = nullptr);
    AdvancedPermissionsWidget(const QHash<QString, QString> &, QWidget *parent = nullptr);
    ~AdvancedPermissionsWidget() override;

    QHash<QString, QString> currentUsers() const;

protected:
    AdvancedPermissionsWidgetPrivate *const d_ptr;

private:
    void leftArrowClicked();
    void rightArrowClicked();
    enum Columns { FullName = 0, LoginName = 1 };
    void setupCommon();
    QTreeWidgetItem *constructItem(const KUser &user, const QString &itemData = QString());
};

#endif // PLASMA_NM_ADVANCED_PERMISSIONS_WIDGET_H
