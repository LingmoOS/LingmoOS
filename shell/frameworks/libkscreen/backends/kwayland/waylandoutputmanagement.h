/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include "qwayland-kde-output-management-v2.h"
#include "qwayland-kde-output-order-v1.h"

#include <QObject>
#include <QSize>

namespace KScreen
{
class WaylandConfig;
class WaylandOutputDevice;

class WaylandOutputConfiguration : public QObject, public QtWayland::kde_output_configuration_v2
{
    Q_OBJECT
public:
    WaylandOutputConfiguration(struct ::kde_output_configuration_v2 *object);

Q_SIGNALS:
    void applied();
    void failed();

protected:
    void kde_output_configuration_v2_applied() override;
    void kde_output_configuration_v2_failed() override;
};

class WaylandOutputManagement : public QObject, public QtWayland::kde_output_management_v2
{
    Q_OBJECT
public:
    WaylandOutputManagement(struct ::wl_registry *registry, int id, int version);

    WaylandOutputConfiguration *createConfiguration();
};

class WaylandOutputOrder : public QObject, public QtWayland::kde_output_order_v1
{
    Q_OBJECT
public:
    WaylandOutputOrder(struct ::wl_registry *registry, int id, int version);
    ~WaylandOutputOrder();

    QList<QString> order() const;

Q_SIGNALS:
    void outputOrderChanged(const QList<QString> &outputs);

private:
    void kde_output_order_v1_output(const QString &output_name) override;
    void kde_output_order_v1_done() override;

    QList<QString> m_outputOrder;
    QList<QString> m_pendingOutputOrder;
};
}
