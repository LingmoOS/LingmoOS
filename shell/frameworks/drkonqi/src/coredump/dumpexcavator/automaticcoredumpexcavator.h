// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QTemporaryDir>

class AutomaticCoredumpExcavator : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    void excavateFrom(const QString &coredumpFilename);

Q_SIGNALS:
    void failed();
    // WARNING: the corepath is only valid as long as the excavator exists!
    void excavated(const QString &corePath);

private:
    std::unique_ptr<QTemporaryDir> m_coreDir;
};
