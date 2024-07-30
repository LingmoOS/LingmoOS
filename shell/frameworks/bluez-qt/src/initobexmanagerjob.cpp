/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "initobexmanagerjob.h"
#include "debug.h"
#include "obexmanager.h"
#include "obexmanager_p.h"

namespace BluezQt
{
class InitObexManagerJobPrivate : public QObject
{
public:
    explicit InitObexManagerJobPrivate(InitObexManagerJob *q, ObexManager *manager);

    void doStart();
    void initError(const QString &errorText);
    void initFinished();

    InitObexManagerJob *q;
    ObexManager *m_manager;
};

InitObexManagerJobPrivate::InitObexManagerJobPrivate(InitObexManagerJob *q, ObexManager *manager)
    : QObject(q)
    , q(q)
    , m_manager(manager)
{
}

void InitObexManagerJobPrivate::doStart()
{
    if (m_manager->d->m_initialized) {
        qCWarning(BLUEZQT) << "InitObexManagerJob: ObexManager already initialized!";
        q->emitResult();
        return;
    }

    connect(m_manager->d.get(), &ObexManagerPrivate::initError, this, &InitObexManagerJobPrivate::initError);
    connect(m_manager->d.get(), &ObexManagerPrivate::initFinished, this, &InitObexManagerJobPrivate::initFinished);

    m_manager->d->init();
}

void InitObexManagerJobPrivate::initError(const QString &errorText)
{
    qCWarning(BLUEZQT) << "InitObexManagerJob Error:" << errorText;

    q->setError(InitObexManagerJob::UserDefinedError);
    q->setErrorText(errorText);
    q->emitResult();
}

void InitObexManagerJobPrivate::initFinished()
{
    q->emitResult();
}

InitObexManagerJob::InitObexManagerJob(ObexManager *manager)
    : Job(manager)
    , d(new InitObexManagerJobPrivate(this, manager))
{
}

InitObexManagerJob::~InitObexManagerJob()
{
    if (isRunning()) {
        qCWarning(BLUEZQT) << "InitObexManagerJob Error: Job was deleted before finished!";

        setError(UserDefinedError);
        setErrorText(QStringLiteral("Job was deleted before finished."));
        emitResult();
    }
}

ObexManager *InitObexManagerJob::manager() const
{
    return d->m_manager;
}

void InitObexManagerJob::doStart()
{
    d->doStart();
}

void InitObexManagerJob::doEmitResult()
{
    Q_EMIT result(this);
}

} // namespace BluezQt

#include "moc_initobexmanagerjob.cpp"
