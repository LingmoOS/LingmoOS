/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_PHABRICATORJOBS_H
#define KDEVPLATFORM_PLUGIN_PHABRICATORJOBS_H

#include <QHash>
#include <QList>
#include <QPair>
#include <QUrl>

#include <KJob>
#include <QProcess>

class QNetworkReply;

namespace Phabricator
{
class DifferentialRevision : public KJob
{
    Q_OBJECT
public:
    DifferentialRevision(const QString &id, QObject *parent)
        : KJob(parent)
        , m_id(id)
        , m_commit(QString())
    {
        setPercent(0);
    }
    QString requestId() const
    {
        return m_id;
    }
    void setRequestId(const QString &id)
    {
        m_id = id;
    }
    QString commitRef() const
    {
        return m_commit;
    }
    void setCommitRef(const QString &commit)
    {
        m_commit = commit;
    }
    void start() override;
    QString errorString() const override
    {
        return m_errorString;
    }
    void setErrorString(const QString &msg);
    QString scrubbedResult();
    QStringList scrubbedResultList();

private Q_SLOTS:
    virtual void done(int exitCode, QProcess::ExitStatus exitStatus) = 0;

protected:
    virtual bool buildArcCommand(const QString &workDir, const QString &patchFile = QString(), bool doBrowse = false);
    QProcess m_arcCmd;

private:
    QString m_id;
    QString m_commit;
    QString m_errorString;
    QString m_arcInput;
};

class NewDiffRev : public DifferentialRevision
{
    Q_OBJECT
public:
    NewDiffRev(const QUrl &patch, const QString &project, bool doBrowse = false, QObject *parent = nullptr);
    QString diffURI() const
    {
        return m_diffURI;
    }

private Q_SLOTS:
    void done(int exitCode, QProcess::ExitStatus exitStatus) override;

private:
    QUrl m_patch;
    QString m_project;
    QString m_diffURI;
};

class UpdateDiffRev : public DifferentialRevision
{
    Q_OBJECT
public:
    UpdateDiffRev(const QUrl &patch,
                  const QString &basedir,
                  const QString &id,
                  const QString &updateComment = QString(),
                  bool doBrowse = false,
                  QObject *parent = nullptr);
    QString diffURI() const
    {
        return m_diffURI;
    }

private Q_SLOTS:
    void done(int exitCode, QProcess::ExitStatus exitStatus) override;

private:
    QUrl m_patch;
    QString m_basedir;
    QString m_diffURI;
};

class DiffRevList : public DifferentialRevision
{
    Q_OBJECT
public:
    enum Status {
        Accepted,
        NeedsReview,
        NeedsRevision,
    };
    Q_ENUM(Status)

    DiffRevList(const QString &projectDir, QObject *parent = nullptr);
    // return the open diff. revisions as a list of <diffID,diffDescription> pairs
    QList<QPair<QString, QString>> reviews() const
    {
        return m_reviews;
    }
    // return the open diff. revisions as a map of diffDescription->diffID entries
    QHash<QString, QString> reviewMap() const
    {
        return m_revMap;
    }
    // return the open diff. revision statuses as a map of diffDescription->Status entries
    QHash<QString, Status> statusMap() const
    {
        return m_statusMap;
    }

private Q_SLOTS:
    void done(int exitCode, QProcess::ExitStatus exitStatus) override;

protected:
    bool buildArcCommand(const QString &workDir, const QString &unused = QString(), bool ignored = false) override;

private:
    QList<QPair<QString, QString>> m_reviews;
    QHash<QString, QString> m_revMap;
    QHash<QString, Status> m_statusMap;
    QString m_projectDir;
};
}

#endif
