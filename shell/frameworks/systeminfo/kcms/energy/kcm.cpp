/*
 *   SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QDebug>
#include <QStandardPaths>

#include <QQmlContext>
#include <QQmlEngine>
#include <QStandardItemModel>

#include <Solid/Battery>
#include <Solid/Device>
#include <Solid/DeviceNotifier>

#include <KPluginFactory>
#include <KQuickConfigModule>

#include "batterymodel.h"
#include "statisticsprovider.h"

class KCMEnergyInfo : public KQuickConfigModule
{
    Q_OBJECT
    Q_PROPERTY(BatteryModel *batteries READ batteries CONSTANT)
public:
    explicit KCMEnergyInfo(QObject *parent, const KPluginMetaData &data)
        : KQuickConfigModule(parent, data)
    {
        qmlRegisterAnonymousType<BatteryModel>("org.kde.kinfocenter.energy.private", 1);

        qmlRegisterType<StatisticsProvider>("org.kde.kinfocenter.energy.private", 1, 0, "HistoryModel");
        qmlRegisterUncreatableType<BatteryModel>("org.kde.kinfocenter.energy.private", 1, 0, "BatteryModel", QStringLiteral("Use BatteryModel"));

        m_batteries = new BatteryModel(this);
    }

    BatteryModel *batteries() const
    {
        return m_batteries;
    }

private:
    BatteryModel *m_batteries = nullptr;
};

Q_DECLARE_METATYPE(QList<QPointF>)
K_PLUGIN_CLASS_WITH_JSON(KCMEnergyInfo, "kcm_energyinfo.json")

#include "kcm.moc"
