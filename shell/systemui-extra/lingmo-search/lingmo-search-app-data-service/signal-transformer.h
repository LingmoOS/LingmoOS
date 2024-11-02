/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#ifndef SIGNALTRANSFORMER_H
#define SIGNALTRANSFORMER_H

#include <QObject>
#include <QDBusInterface>
#include <QMutexLocker>
#include "app-db-manager.h"

using namespace LingmoUISearch;

class SignalTransformer : public QObject
{
    Q_CLASSINFO("D-Bus Interface","org.lingmo.search.signalTransformer")

    Q_OBJECT

public:
    enum SignalType{
        Invalid = -1,
        Delete = 0,
        Insert,
        UpdateAll,
        Update
    };
    static SignalTransformer *getTransformer();

    static QMutex s_mutex;
public Q_SLOTS:
    void handleItemUpdate(const ApplicationInfoMap &item);
    void handleItemUpdateAll(const QString &desktopFilePath);
    void handleItemInsert(const QString &desktopFilePath);
    void handleItemDelete(const QString &desktopFilePath);
    void handleSignalTransform();

private:
    SignalTransformer(QObject *parent = nullptr);
    SignalTransformer(const SignalTransformer &) = delete;
    SignalTransformer& operator = (const SignalTransformer&) = delete;
    void transform();

    ApplicationInfoMap m_items2BUpdate;
    QStringList m_items2BUpdateAll;
    QStringList m_items2BInsert;
    QStringList m_items2BDelete;
    SignalType m_signalType = SignalType::Invalid;

Q_SIGNALS:
    void appDBItemsUpdateAll(const QStringList&);
    void appDBItemsUpdate(const ApplicationInfoMap&);
    void appDBItemsAdd(const QStringList&);
    void appDBItemsDelete(const QStringList&);
};

#endif // SIGNALTRANSFORMER_H
