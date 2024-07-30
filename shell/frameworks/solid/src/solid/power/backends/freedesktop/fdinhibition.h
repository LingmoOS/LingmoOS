/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_FD_INHIBITION_H
#define SOLID_FD_INHIBITION_H

#include "backends/abstractinhibition.h"
#include "inhibition.h"
#include "solid/power.h"

#include <QDBusMessage>
#include <qglobal.h>

namespace Solid
{
class FdInhibition : public AbstractInhibition
{
    Q_OBJECT
public:
    explicit FdInhibition(Power::InhibitionTypes inhibitions, const QString &description, QObject *parent = nullptr);
    virtual ~FdInhibition();

    void start() override;
    void stop() override;
    Inhibition::State state() const override;

    void setDescription(const QString &description);

private Q_SLOTS:
    void slotDBusReply(const QDBusMessage &msg);
    void slotDBusError(const QDBusError &error);

private:
    void setState(const Inhibition::State &state);

    int m_fd;
    Inhibition::State m_state;
    QString m_description;
    Power::InhibitionTypes m_inhibitions;
};
}

#endif // SOLID_FD_INHIBITION_H
