/*
    This file is part of KDE.

    SPDX-FileCopyrightText: 2008 Cornelius Schumacher <schumacher@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#ifndef ATTICA_ATTICABASEJOB_H
#define ATTICA_ATTICABASEJOB_H

#include <memory>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QObject>
#include <QSharedPointer>
#include <QUrl>

#include "attica_export.h"
#include "metadata.h"

class QNetworkReply;

namespace Attica
{
class PlatformDependent;

/**
 * @class BaseJob atticabasejob.h
 *
 * The baseclass for all job classes.
 */
class ATTICA_EXPORT BaseJob : public QObject
{
    Q_OBJECT

public:
    ~BaseJob() override;

    Metadata metadata() const;

    enum NetworkRequestCustomAttributes {
        UserAttribute = QNetworkRequest::User + 1,
        PasswordAttribute,
    };

    /**
     * @returns whether abort() has been called on the job
     *
     * @since 5.87
     */
    bool isAborted() const;

public Q_SLOTS:
    void start();
    void abort();

Q_SIGNALS:
    void finished(Attica::BaseJob *job);

protected Q_SLOTS:
    void dataFinished();

protected:
    BaseJob(PlatformDependent *internals);

    void setMetadata(const Metadata &data) const;

    virtual QNetworkReply *executeRequest() = 0;
    virtual void parse(const QString &xml) = 0;
    PlatformDependent *internals();
    void setError(int errorCode);

private Q_SLOTS:
    ATTICA_NO_EXPORT void doWork();
    ATTICA_NO_EXPORT void authenticationRequired(QNetworkReply *, QAuthenticator *);

private:
    BaseJob(const BaseJob &other) = delete;
    BaseJob &operator=(const BaseJob &other) = delete;

    class Private;
    std::unique_ptr<Private> d;
};

}

#endif
