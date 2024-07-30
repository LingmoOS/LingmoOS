/*
 * SPDX-FileCopyrightText: 2013 Abdurrahman AVCI <abdurrahmanavci@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "sessionmodel.h"
#include "config.h"

#include <memory>

#include <QDir>
#include <QFile>
#include <QList>
#include <QTextStream>

#include <KLocalizedString>

class Session
{
public:
    QString file;
    QString name;
    QString exec;
    QString comment;
};

typedef std::shared_ptr<Session> SessionPtr;

class SessionModelPrivate
{
public:
    int lastIndex{0};
    QList<SessionPtr> sessions;
};

SessionModel::SessionModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new SessionModelPrivate())
{
    loadDir(QStringLiteral(XSESSIONS_DIR), SessionTypeX);
    loadDir(QStringLiteral(WAYLAND_SESSIONS_DIR), SessionTypeWayland);
}

SessionModel::~SessionModel()
{
    delete d;
}

void SessionModel::loadDir(const QString &path, SessionType type)
{
    QDir dir(path);
    dir.setNameFilters(QStringList() << QStringLiteral("*.desktop"));
    dir.setFilter(QDir::Files);
    // read session
    const QStringList sessionList = dir.entryList();
    for (const QString &session : sessionList) {
        QFile inputFile(dir.absoluteFilePath(session));
        if (!inputFile.open(QIODevice::ReadOnly)) {
            continue;
        }
        SessionPtr si{new Session{session.chopped(strlen(".desktop")), QString(), QString(), QString()}};
        bool isHidden = false;
        QString current_section;
        QTextStream in(&inputFile);
        while (!in.atEnd()) {
            QString line = in.readLine();

            if (line.startsWith(QLatin1String("["))) {
                // The section name ends before the last ] before the start of a comment
                int end = line.lastIndexOf(QLatin1Char(']'), line.indexOf(QLatin1Char('#')));
                if (end != -1) {
                    current_section = line.mid(1, end - 1);
                }
            }

            if (current_section != QLatin1String("Desktop Entry")) {
                continue; // We are only interested in the "Desktop Entry" section
            }

            if (line.startsWith(QLatin1String("Name="))) {
                si->name = line.mid(5);
                if (type == SessionTypeWayland) {
                    // we want to exactly match the SDDM prompt which is formatted in this way
                    // with the exact same check
                    if (!si->name.endsWith(QLatin1String(" (Wayland)"))) {
                        si->name = i18nc("%1 is the name of a session", "%1 (Wayland)", si->name);
                    }
                }
            }
            if (line.startsWith(QLatin1String("Exec="))) {
                si->exec = line.mid(5);
            }
            if (line.startsWith(QLatin1String("Comment="))) {
                si->comment = line.mid(8);
            }
            if (line.startsWith(QLatin1String("Hidden="))) {
                isHidden = line.mid(7).toLower() == QLatin1String("true");
            }
        }
        if (!isHidden) {
            // add to sessions list
            d->sessions.push_back(si);
        }

        // close file
        inputFile.close();
    }
}

QHash<int, QByteArray> SessionModel::roleNames() const
{
    // set role names
    QHash<int, QByteArray> roleNames;
    roleNames[FileRole] = "file";
    roleNames[NameRole] = "name";
    roleNames[ExecRole] = "exec";
    roleNames[CommentRole] = "comment";

    return roleNames;
}

int SessionModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return d->sessions.length();
}

QVariant SessionModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= d->sessions.count()) {
        return QVariant();
    }

    // get session
    SessionPtr session = d->sessions[index.row()];

    // return correct value
    if (role == FileRole) {
        return session->file;
    } else if (role == NameRole) {
        return session->name;
    } else if (role == ExecRole) {
        return session->exec;
    } else if (role == CommentRole) {
        return session->comment;
    }

    // return empty value
    return QVariant();
}

int SessionModel::indexOf(const QString &sessionId) const
{
    for (int i = 0; i < d->sessions.length(); i++) {
        if (d->sessions[i]->file == sessionId) {
            return i;
        }
    }
    return 0;
}
