/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include "kscreen_export.h"
#include "types.h"

#include <QDebug>
#include <QMetaType>
#include <QObject>
#include <QSize>

namespace KScreen
{
class KSCREEN_EXPORT Mode : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id WRITE setId NOTIFY modeChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY modeChanged)
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY modeChanged)
    Q_PROPERTY(float refreshRate READ refreshRate WRITE setRefreshRate NOTIFY modeChanged)

public:
    explicit Mode();
    ~Mode() override;

    ModePtr clone() const;

    const QString id() const;
    void setId(const QString &id);

    QString name() const;
    void setName(const QString &name);

    QSize size() const;
    void setSize(const QSize &size);

    float refreshRate() const;
    void setRefreshRate(float refresh);

Q_SIGNALS:
    void modeChanged();

private:
    Q_DISABLE_COPY(Mode)

    class Private;
    Private *const d;

    Mode(Private *dd);
};

} // KSCreen namespace

KSCREEN_EXPORT QDebug operator<<(QDebug dbg, const KScreen::ModePtr &mode);
