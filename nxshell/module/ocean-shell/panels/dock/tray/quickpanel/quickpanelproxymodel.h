// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QtQml/qqml.h>
#include <QSortFilterProxyModel>
#include <QQmlParserStatus>

namespace dock {

class QuickPanelProxyModel : public QSortFilterProxyModel, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QString trayItemPluginId READ trayItemPluginId WRITE setTrayItemPluginId NOTIFY trayItemPluginIdChanged FINAL)
    Q_PROPERTY(QObject* trayItemSurface READ trayItemSurface NOTIFY trayItemSurfaceChanged FINAL)
    Q_PROPERTY(QAbstractItemModel* trayPluginModel READ trayPluginModel WRITE setTrayPluginModel NOTIFY trayPluginModelChanged FINAL)
    QML_NAMED_ELEMENT(QuickPanelProxyModel)
    Q_INTERFACES(QQmlParserStatus)
public:
    explicit QuickPanelProxyModel(QObject *parent = nullptr);

    Q_INVOKABLE QString getTitle(const QString &pluginId) const;
    Q_INVOKABLE bool isQuickPanelPopup(const QString &pluginId, const QString &itemKey) const;
    Q_INVOKABLE void openSystemSettings();

    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QObject *trayItemSurface() const;

    QString trayItemPluginId() const;
    void setTrayItemPluginId(const QString &newTrayItemPluginId);

    QAbstractItemModel *trayPluginModel() const;
    void setTrayPluginModel(QAbstractItemModel *newTrayPluginModel);

signals:
    void trayItemSurfaceChanged();
    void trayItemPluginIdChanged();

    void trayPluginModelChanged();

public:
    void classBegin() override;
    void componentComplete() override;

protected:
    bool lessThan(const QModelIndex &sourceLeft, const QModelIndex &sourceRight) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    void updateQuickPlugins();
    void watchingCountChanged();

    int pluginOrder(const QModelIndex &index) const;
    int surfaceType(const QModelIndex &index) const;
    int surfaceOrder(const QModelIndex &index) const;
    QString surfacePluginId(const QModelIndex &index) const;
    QString surfaceItemKey(const QModelIndex &index) const;
    QString surfaceDisplayName(const QModelIndex &index) const;
    QVariant surfaceValue(const QModelIndex &index, const QByteArray &roleName) const;
    QModelIndex surfaceIndex(const QString &pluginId) const;
    QObject *surfaceObject(const QModelIndex &index) const;
    QObject *traySurfaceObject(const QString &pluginId) const;
    QString traySurfaceItemKey(const QString &pluginId) const;
    int roleByName(const QByteArray &roleName) const;
    QAbstractListModel *surfaceModel() const;
private slots:
    void updateTrayItemSurface();

private:
    QStringList m_quickPlugins;
    QStringList m_hideInPanelPlugins;
    QString m_trayItemPluginId;
    QAbstractItemModel *m_trayPluginModel = nullptr;
};

}
