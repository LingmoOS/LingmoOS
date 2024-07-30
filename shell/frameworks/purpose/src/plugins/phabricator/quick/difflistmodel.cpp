/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "difflistmodel.h"
#include "phabricatorjobs.h"

#include <QBrush>
#include <QDebug>
#include <QDir>
#include <QTemporaryDir>

DiffListModel::DiffListModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_initialDir(QDir::currentPath())
    , m_tempDir(nullptr)
{
    refresh();
}

void DiffListModel::refresh()
{
    if (m_tempDir) {
        qCritical() << "DiffListModel::refresh() called while still active!";
        return;
    }

    beginResetModel();
    m_values.clear();
    endResetModel();

    // our CWD should be the directory from which the application was launched, which
    // may or may not be a git, mercurial or svn working copy, so we create a temporary
    // directory in which we initialise a git repository. This may be an empty repo.

    m_initialDir = QDir::currentPath();
    m_tempDir = new QTemporaryDir;
    if (!m_tempDir->isValid()) {
        qCritical() << "DiffListModel::refresh() failed to create temporary directory" << m_tempDir->path() << ":" << m_tempDir->errorString();
    } else {
        if (QDir::setCurrent(m_tempDir->path())) {
            // the directory will be removed in receivedDiffRevs()
            m_tempDir->setAutoRemove(false);
            QProcess initGit;
            bool ok = false;
            // create the virgin git repo. This is a very cheap operation that should
            // never fail in a fresh temporary directory we ourselves created, so it
            // should be OK to do this with a synchronous call.
            initGit.start(QLatin1String("git init"), QStringList());
            if (initGit.waitForStarted(1000)) {
                ok = initGit.waitForFinished(500);
            }
            if (!ok) {
                qCritical() << "DiffListModel::refresh() : couldn't create temp. git repo:" << initGit.errorString();
            }
        } else {
            qCritical() << "DiffListModel::refresh() failed to chdir to" << m_tempDir->path();
        }
    }
    // create a list request with the current (= temp.) directory as the project directory.
    // This request is executed asynchronously, which is why we cannot restore the initial
    // working directory just yet, nor remove the temporary directory.
    Phabricator::DiffRevList *repo = new Phabricator::DiffRevList(QDir::currentPath(), this);
    connect(repo, &Phabricator::DiffRevList::finished, this, &DiffListModel::receivedDiffRevs);
    repo->start();
}

void DiffListModel::receivedDiffRevs(KJob *job)
{
    if (job->error() != 0) {
        qWarning() << "error getting differential revision list" << job->errorString();
        beginResetModel();
        m_values.clear();
        endResetModel();
        return;
    }

    const auto diffRevList = dynamic_cast<Phabricator::DiffRevList *>(job);
    const auto revs = diffRevList->reviews();

    beginResetModel();
    m_values.clear();
    for (const auto &review : revs) {
        auto status = diffRevList->statusMap()[review.second];
        m_values += Value{review.second, review.first, status};
    }
    endResetModel();

    // now we can restore the initial working directory and remove the temp directory
    // (in that order!).
    if (!QDir::setCurrent(m_initialDir)) {
        qCritical() << "DiffListModel::receivedDiffRevs() failed to restore initial directory" << m_initialDir;
    }
    if (m_tempDir) {
        m_tempDir->remove();
        delete m_tempDir;
        m_tempDir = nullptr;
    }
}

QHash<int, QByteArray> DiffListModel::roleNames() const
{
    const QHash<int, QByteArray> roles = {{Qt::DisplayRole, QByteArrayLiteral("display")},
                                          {Qt::ToolTipRole, QByteArrayLiteral("toolTip")},
                                          {Qt::ForegroundRole, QByteArrayLiteral("textColor")}};
    return roles;
}

QVariant DiffListModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid() || idx.column() != 0 || idx.row() >= m_values.size()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole:
        return m_values[idx.row()].summary;
    case Qt::ToolTipRole:
        return m_values[idx.row()].id;
    case Qt::ForegroundRole:
        // Use the colours arc also uses
        switch (m_values[idx.row()].status.value<Phabricator::DiffRevList::Status>()) {
        case Phabricator::DiffRevList::Accepted:
            // alternative: KColorScheme::ForegroundRole::PositiveText
            return QBrush(Qt::green);
        case Phabricator::DiffRevList::NeedsReview:
            // alternative: KColorScheme::ForegroundRole::NeutralText
            return QBrush(Qt::magenta);
        case Phabricator::DiffRevList::NeedsRevision:
            // alternative: KColorScheme::ForegroundRole::NegativeText
            return QBrush(Qt::red);
        default:
            return {};
        }
    }
    return {};
}

int DiffListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_values.count();
}

QVariant DiffListModel::get(int row, const QByteArray &role)
{
    return index(row, 0).data(roleNames().key(role));
}

void DiffListModel::setStatus(const QString &status)
{
    if (m_status != status) {
        m_status = status;
        refresh();
    }
}

#include "moc_difflistmodel.cpp"
