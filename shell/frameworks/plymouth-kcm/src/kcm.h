/*
 *  SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef _KCM_PLYMOUTH_H
#define _KCM_PLYMOUTH_H

#include <KQuickConfigModule>
#include <QDir>
#include <QStandardItemModel>

#include <KNSCore/Entry>

class QQuickItem;

class KCMPlymouth : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(QStandardItemModel *themesModel READ themesModel CONSTANT)
    Q_PROPERTY(QString selectedPlugin READ selectedPlugin WRITE setSelectedPlugin NOTIFY selectedPluginChanged)
    Q_PROPERTY(int selectedPluginIndex READ selectedPluginIndex NOTIFY selectedPluginIndexChanged)
    Q_PROPERTY(bool busy READ busy WRITE setBusy NOTIFY busyChanged)

public:
    enum Roles {
        DescriptionRole = Qt::UserRole + 1,
        PluginNameRole,
        ScreenhotRole,
        UninstallableRole,
    };
    explicit KCMPlymouth(QObject *parent, const KPluginMetaData &metaData);
    ~KCMPlymouth() override;

    QStandardItemModel *themesModel();

    QString selectedPlugin() const;
    void setSelectedPlugin(const QString &plugin);

    int selectedPluginIndex() const;

    bool busy() const;
    void setBusy(const bool &busy);

    Q_INVOKABLE void reloadModel();
    Q_INVOKABLE void onEntryEvent(const KNSCore::Entry &entry);
    Q_INVOKABLE void uninstall(const QString &plugin);

public Q_SLOTS:
    void load() override;
    void save() override;
    void defaults() override;

Q_SIGNALS:
    void selectedPluginChanged();
    void selectedPluginIndexChanged();

    void busyChanged();

    void showSuccessMessage(const QString &message);
    void showErrorMessage(const QString &message);

private:
    QStandardItemModel *const m_model;
    QString m_selectedPlugin;
    bool m_busy = false;
};

#endif
