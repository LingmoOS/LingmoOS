// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <QtQml/qqml.h>
#include "notifyentity.h"

class QQmlEngine;
class QJSEngine;
namespace notification {
class DataAccessor;
}

namespace notifycenter {

using namespace notification;
/**
 * @brief The NotifyAccessor class
 */
class NotifyAccessor : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString dataInfo READ dataInfo NOTIFY dataInfoChanged FINAL)
    Q_PROPERTY(QStringList apps READ apps NOTIFY appsChanged FINAL)
    Q_PROPERTY(bool debugging READ debugging NOTIFY debuggingChanged)
public:
    static NotifyAccessor *instance();
    static NotifyAccessor *create(QQmlEngine *, QJSEngine *);

    void setDataAccessor(DataAccessor *accessor);
    void setDataUpdater(QObject *updater);

    bool enabled() const;
    void setEnabled(bool enabled);

    void invokeAction(const NotifyEntity &entity, const QString &actionId);
    void pinApplication(const QString &appName, bool pin);
    bool applicationPin(const QString &appName) const;
    Q_INVOKABLE void openNotificationSetting();

    NotifyEntity fetchEntity(qint64 id) const;
    int fetchEntityCount(const QString &appName) const;
    NotifyEntity fetchLastEntity(const QString &appName) const;
    QList<NotifyEntity> fetchEntities(const QString &appName, int maxCount = -1);
    QStringList fetchApps(int maxCount = -1) const;
    void removeEntity(qint64 id);
    void removeEntityByApp(const QString &appName);
    void clear();

    void closeNotify(const NotifyEntity &entity);
    void invokeNotify(const NotifyEntity &entity, const QString &actionId);

signals:
    void entityReceived(qint64 id);
    void stagingEntityReceived(qint64 id);
    void stagingEntityClosed(qint64 id);

public slots:
    void addNotify(const QString &appName, const QString &content);
    void fetchDataInfo();

signals:
    void dataInfoChanged();
    void appsChanged();
    void debuggingChanged();

private slots:
    void onNotificationStateChanged(qint64 id, int processedType);
    void onReceivedRecord(const QString &id);

private:
    explicit NotifyAccessor(QObject *parent = nullptr);

    QString dataInfo() const;
    QStringList apps() const;
    bool debugging() const;

private:
    DataAccessor *m_accessor = nullptr;
    QObject *m_dataUpdater = nullptr;
    QStringList m_pinnedApps;
    QStringList m_apps;
    bool m_debugging = false;
    QString m_dataInfo;
    bool m_enabled = false;
};
}
