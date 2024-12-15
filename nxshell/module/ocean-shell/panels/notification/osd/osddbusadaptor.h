// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QDBusVariant>
#include <QDBusAbstractAdaptor>

namespace osd {

class OsdPanel;
class OsdDBusAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.lingmo.ocean.Osd1")

public:
    explicit OsdDBusAdaptor(QObject *parent = nullptr);

public Q_SLOTS: // methods
    void ShowOSD(const QString &text);

private:
    OsdPanel *impl() const;
};

} // osd
