// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dopticaldiscoperator_p.h"

#include <QDebug>
#include <QUrl>
#include <QPointer>

#include <DError>

#include "dxorrisoengine.h"
#include "dudfburnengine.h"
#include "dopticaldiscmanager.h"
#include "dopticaldiscinfo.h"

DBURN_BEGIN_NAMESPACE

using DTK_CORE_NAMESPACE::DError;
using DTK_CORE_NAMESPACE::DExpected;
using DTK_CORE_NAMESPACE::DUnexpected;

DOpticalDiscOperatorPrivate::DOpticalDiscOperatorPrivate(const QString &dev, DOpticalDiscOperator *qq)
    : q_ptr(qq), curDev(dev)
{
    qRegisterMetaType<JobStatus>("JobStatus");
}

bool DOpticalDiscOperatorPrivate::makeStageFiles(const QString &stagePath, QString *errMsg)
{
    QUrl diskUrl { QUrl::fromLocalFile(stagePath) };

    if (diskUrl.isEmpty() || !diskUrl.isValid()) {
        *errMsg = "Invalid disk path";
        return false;
    }

    files.first = stagePath;
    files.second = "/";

    return true;
}

void DOpticalDiscOperatorPrivate::onUDFEngineJobStatusChanged(JobStatus status, int progress)
{
    Q_Q(DOpticalDiscOperator);

    if (sender())
        currentEngine = sender();

    auto engine { qobject_cast<DUDFBurnEngine *>(currentEngine) };
    if (!engine) {
        qWarning() << "[dtkburn] null udf engine pointer";
        Q_EMIT q->jobStatusChanged(status, progress, {}, {});
        return;
    }

    if (status == JobStatus::Failed)
        Q_EMIT q->jobStatusChanged(status, progress, {}, engine->lastErrorMessage());
    else
        Q_EMIT q->jobStatusChanged(status, progress, {}, {});
}

void DOpticalDiscOperatorPrivate::onXorrisoEngineJobStatusChanged(JobStatus status, int progress, QString speed)
{
    Q_Q(DOpticalDiscOperator);

    if (sender())
        currentEngine = sender();

    auto engine { qobject_cast<DXorrisoEngine *>(currentEngine) };
    if (!engine) {
        qWarning() << "[dtkburn] null xorriso engine pointer";
        Q_EMIT q->jobStatusChanged(status, progress, speed, {});
        return;
    }

    Q_EMIT q->jobStatusChanged(status, progress, speed, engine->takeInfoMessages());
}

DOpticalDiscOperator::~DOpticalDiscOperator()
{
}

void DOpticalDiscOperator::setSpeed(int speed)
{
    Q_D(DOpticalDiscOperator);

    d->curSpeed = speed;
}

void DOpticalDiscOperator::setVolumeName(const QString &name)
{
    Q_D(DOpticalDiscOperator);

    d->curVolName = name;
}

void DOpticalDiscOperator::setCheckPrecision(double precision)
{
    Q_D(DOpticalDiscOperator);

    d->curPrecision = precision;
}

int DOpticalDiscOperator::speed() const
{
    Q_D(const DOpticalDiscOperator);

    return d->curSpeed;
}

QString DOpticalDiscOperator::volumeName() const
{
    Q_D(const DOpticalDiscOperator);

    return d->curVolName;
}

double DOpticalDiscOperator::checkPrecision() const
{
    Q_D(const DOpticalDiscOperator);

    return d->curPrecision;
}

DExpected<bool> DOpticalDiscOperator::burn(const QString &stagePath, const BurnOptions &opts)
{
    Q_D(DOpticalDiscOperator);

    bool ret { false };
    QString msg;
    if (!d->makeStageFiles(stagePath, &msg))
        return DUnexpected<> { DError { -1, QString("StagePath Error: %1").arg(msg) } };

    if (opts.testFlag(BurnOption::UDF102Supported)) {
        QScopedPointer<DUDFBurnEngine> udfEngine { new DUDFBurnEngine };
        connect(udfEngine.data(), &DUDFBurnEngine::jobStatusChanged,
                d, &DOpticalDiscOperatorPrivate::onUDFEngineJobStatusChanged);
        ret = udfEngine->doBurn(d->curDev, d->files, d->curVolName);
    } else {
        QScopedPointer<DXorrisoEngine> xorrisoEngine { new DXorrisoEngine };
        connect(xorrisoEngine.data(), &DXorrisoEngine::jobStatusChanged,
                d, &DOpticalDiscOperatorPrivate::onXorrisoEngineJobStatusChanged, Qt::DirectConnection);

        if (!xorrisoEngine->acquireDevice(d->curDev))
            qWarning() << "[dtkburn] Cannot acquire device";

        using XJolietSupport = DXorrisoEngine::JolietSupported;
        using XRockRageSupport = DXorrisoEngine::RockRageSupported;
        using XKeepAppendable = DXorrisoEngine::KeepAppendable;
        XJolietSupport joliet = opts.testFlag(BurnOption::JolietSupported)
                ? XJolietSupport::True
                : XJolietSupport::False;
        XRockRageSupport rockRage = opts.testFlag(BurnOption::RockRidgeSupported)
                ? XRockRageSupport::True
                : XRockRageSupport::False;
        XKeepAppendable keepAppendable = opts.testFlag(BurnOption::KeepAppendable)
                ? XKeepAppendable::True
                : XKeepAppendable::False;

        ret = xorrisoEngine->doBurn(d->files, d->curSpeed, d->curVolName, joliet, rockRage, keepAppendable);
        xorrisoEngine->releaseDevice();
    }

    if (ret)
        return true;

    return DUnexpected<> { DError { -1, "Unknown Error" } };
}

DExpected<bool> DOpticalDiscOperator::erase()
{
    Q_D(DOpticalDiscOperator);

    bool ret { false };
    QScopedPointer<DXorrisoEngine> engine { new DXorrisoEngine };
    connect(engine.data(), &DXorrisoEngine::jobStatusChanged,
            d, &DOpticalDiscOperatorPrivate::onXorrisoEngineJobStatusChanged, Qt::DirectConnection);

    if (!engine->acquireDevice(d->curDev))
        qWarning() << "[dtkburn] Cannot acquire device";

    ret = engine->doErase();
    engine->releaseDevice();

    if (ret)
        return true;

    return DUnexpected<> { DError { -1, "Unknown Error" } };
}

DExpected<bool> DOpticalDiscOperator::checkmedia()
{
    Q_D(DOpticalDiscOperator);

    double gud, slo, bad;
    bool ret { false };
    quint64 blocks { 0 };

    {
        auto result { DOpticalDiscManager::createOpticalDiscInfo(d->curDev) };
        QScopedPointer<DOpticalDiscInfo> info { result ? result.value() : nullptr };
        if (!info)
            return DUnexpected<> { DError { -1, QString("Cannot create disc info by: %1 ").arg(d->curDev) } };
        blocks = info->dataBlocks();
    }

    QScopedPointer<DXorrisoEngine> engine { new DXorrisoEngine };
    connect(engine.data(), &DXorrisoEngine::jobStatusChanged,
            d, &DOpticalDiscOperatorPrivate::onXorrisoEngineJobStatusChanged, Qt::DirectConnection);

    if (!engine->acquireDevice(d->curDev))
        qWarning() << "[dtkburn] Cannot acquire device";

    ret = engine->doCheckmedia(blocks, &gud, &slo, &bad);
    engine->releaseDevice();

    if (ret) {
        if (bad > d->curPrecision)
            return DUnexpected<> { DError { -1, QString("Precision check failed: %1 %2 ").arg(bad).arg(d->curPrecision) } };
        return true;
    }

    return DUnexpected<> { DError { -1, "Unknown Error" } };
}

DExpected<bool> DOpticalDiscOperator::writeISO(const QString &isoPath)
{
    Q_D(DOpticalDiscOperator);

    bool ret { false };
    QScopedPointer<DXorrisoEngine> engine { new DXorrisoEngine };
    connect(engine.data(), &DXorrisoEngine::jobStatusChanged,
            d, &DOpticalDiscOperatorPrivate::onXorrisoEngineJobStatusChanged, Qt::DirectConnection);

    if (!engine->acquireDevice(d->curDev))
        qWarning() << "[dfm-burn] Cannot acquire device";

    if (QUrl(isoPath).isEmpty() || !QUrl(isoPath).isValid())
        return DUnexpected<> { DError { -1, QString("Invalid path: %1 ").arg(isoPath) } };

    ret = engine->doWriteISO(isoPath, d->curSpeed);
    engine->releaseDevice();

    if (ret)
        return true;

    return DUnexpected<> { DError { -1, "Unknown Error" } };
}

DExpected<bool> DOpticalDiscOperator::dumpISO(const QString &isoPath)
{
    Q_D(DOpticalDiscOperator);

    bool ret { false };
    quint64 blocks { 0 };

    {
        auto result { DOpticalDiscManager::createOpticalDiscInfo(d->curDev) };
        QScopedPointer<DOpticalDiscInfo> info { result ? result.value() : nullptr };
        if (!info)
            return DUnexpected<> { DError { -1, QString("Cannot create disc info by: %1 ").arg(d->curDev) } };
        blocks = info->dataBlocks();
    }

    QScopedPointer<DXorrisoEngine> engine { new DXorrisoEngine };
    connect(engine.data(), &DXorrisoEngine::jobStatusChanged,
            d, &DOpticalDiscOperatorPrivate::onXorrisoEngineJobStatusChanged, Qt::DirectConnection);

    if (!engine->acquireDevice(d->curDev))
        qWarning() << "[dtkburn] Cannot acquire device";

    if (QUrl(isoPath).isEmpty() || !QUrl(isoPath).isValid())
        return DUnexpected<> { DError { -1, QString("Invalid path: %1 ").arg(isoPath) } };

    ret = engine->doDumpISO(blocks, isoPath);
    engine->releaseDevice();

    if (ret)
        return true;

    return DUnexpected<> { DError { -1, "Unknown Error" } };
}

DOpticalDiscOperator::DOpticalDiscOperator(const QString &dev, QObject *parent)
    : QObject(parent), d_ptr(new DOpticalDiscOperatorPrivate(dev, this))
{
}

DBURN_END_NAMESPACE
