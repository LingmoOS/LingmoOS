/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "initmanagerjob.h"
#include "debug.h"
#include "manager.h"
#include "manager_p.h"

namespace BluezQt
{
class InitManagerJobPrivate : public QObject
{
public:
    explicit InitManagerJobPrivate(InitManagerJob *q, Manager *manager);

    void doStart();
    void initError(const QString &errorText);
    void initFinished();

    InitManagerJob *q;
    Manager *m_manager;
};

InitManagerJobPrivate::InitManagerJobPrivate(InitManagerJob *q, Manager *manager)
    : QObject(q)
    , q(q)
    , m_manager(manager)
{
}

void InitManagerJobPrivate::doStart()
{
    if (m_manager->d->m_initialized) {
        qCWarning(BLUEZQT) << "InitManagerJob: Manager already initialized!";
        q->emitResult();
        return;
    }

    m_manager->d->init();
    connect(m_manager->d.get(), &ManagerPrivate::initError, this, &InitManagerJobPrivate::initError);
    connect(m_manager->d.get(), &ManagerPrivate::initFinished, this, &InitManagerJobPrivate::initFinished);
}

void InitManagerJobPrivate::initError(const QString &errorText)
{
    qCWarning(BLUEZQT) << "GetManagerJob Error:" << errorText;

    q->setError(InitManagerJob::UserDefinedError);
    q->setErrorText(errorText);
    q->emitResult();
}

void InitManagerJobPrivate::initFinished()
{
    q->emitResult();
}

InitManagerJob::InitManagerJob(Manager *manager)
    : Job(manager)
    , d(new InitManagerJobPrivate(this, manager))
{
}

InitManagerJob::~InitManagerJob()
{
    if (isRunning()) {
        qCWarning(BLUEZQT) << "InitManagerJob Error: Job was deleted before finished!";

        setError(UserDefinedError);
        setErrorText(QStringLiteral("Job was deleted before finished."));
        emitResult();
    }
}

Manager *InitManagerJob::manager() const
{
    return d->m_manager;
}

void InitManagerJob::doStart()
{
    d->doStart();
}

void InitManagerJob::doEmitResult()
{
    Q_EMIT result(this);
}

} // namespace BluezQt

#include "moc_initmanagerjob.cpp"
