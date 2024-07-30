/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef IPV6ROUTESWIDGET_H
#define IPV6ROUTESWIDGET_H

#include <QDialog>
#include <QStyledItemDelegate>

#include <NetworkManagerQt/IpConfig>

class QStandardItem;
class QItemSelection;

class IpV6RoutesWidget : public QDialog
{
    Q_OBJECT
public:
    explicit IpV6RoutesWidget(QWidget *parent = nullptr);
    ~IpV6RoutesWidget() override;

    void setRoutes(const QList<NetworkManager::IpRoute> &list);
    QList<NetworkManager::IpRoute> routes() const;
    void setNeverDefault(bool checked);
    bool neverDefault() const;
    void setIgnoreAutoRoutes(bool checked);
    void setIgnoreAutoRoutesCheckboxEnabled(bool enabled);
    bool ignoreautoroutes() const;

private Q_SLOTS:
    void addRoute();
    void removeRoute();
    /**
     * Update remove IP button depending on if there is a selection
     */
    void selectionChanged(const QItemSelection &);
    void tableViewItemChanged(QStandardItem *);

private:
    class Private;
    Private *const d;
};

#endif // IPV6ROUTESWIDGET_H
