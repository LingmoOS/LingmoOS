// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOPTICALDISCOPERATOR_H
#define DOPTICALDISCOPERATOR_H

#include <QObject>

#include <DExpected>

#include "dtkburn_global.h"
#include "dtkburntypes.h"

DBURN_BEGIN_NAMESPACE

class DOpticalDiscOperator;
namespace DOpticalDiscManager {
DTK_CORE_NAMESPACE::DExpected<DOpticalDiscOperator *>
createOpticalDiscOperator(const QString &dev, QObject *parent);
}   // namespace DOpticalDiscManager

class DOpticalDiscOperatorPrivate;
class DOpticalDiscOperator final : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DOpticalDiscOperator)

    Q_PROPERTY(int speed READ speed WRITE setSpeed)
    Q_PROPERTY(QString volumeName READ volumeName WRITE setVolumeName)
    Q_PROPERTY(double checkPrecision READ checkPrecision WRITE setCheckPrecision)

public:
    ~DOpticalDiscOperator() override;

    void setSpeed(int speed);
    void setVolumeName(const QString &name);
    void setCheckPrecision(double checkPrecision);

    int speed() const;
    QString volumeName() const;
    double checkPrecision() const;

    DTK_CORE_NAMESPACE::DExpected<bool> burn(const QString &stagePath, const BurnOptions &opts);
    DTK_CORE_NAMESPACE::DExpected<bool> erase();
    DTK_CORE_NAMESPACE::DExpected<bool> checkmedia();
    DTK_CORE_NAMESPACE::DExpected<bool> writeISO(const QString &isoPath);
    DTK_CORE_NAMESPACE::DExpected<bool> dumpISO(const QString &isoPath);

Q_SIGNALS:
    void jobStatusChanged(JobStatus status, int progress, QString speed, QStringList message);

private:
    explicit DOpticalDiscOperator(const QString &dev, QObject *parent = nullptr);

    friend DTK_CORE_NAMESPACE::DExpected<DOpticalDiscOperator *>
    DOpticalDiscManager::createOpticalDiscOperator(const QString &dev, QObject *parent);

private:
    QScopedPointer<DOpticalDiscOperatorPrivate> d_ptr;
};

DBURN_END_NAMESPACE

#endif   // DOPTICALDISCOPERATOR_H
