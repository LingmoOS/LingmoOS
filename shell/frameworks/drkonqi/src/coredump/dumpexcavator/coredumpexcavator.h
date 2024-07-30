// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>

class QFile;

class CoredumpExcavator : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    void excavateFromTo(const QString &coreFile, const std::shared_ptr<QFile> &coreFileTarget);

Q_SIGNALS:
    void excavated(int exitCode);
};
