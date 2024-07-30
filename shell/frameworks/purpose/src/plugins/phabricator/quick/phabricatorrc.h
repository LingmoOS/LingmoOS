/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHABRICATORRC_H
#define PHABRICATORRC_H

#include <QObject>
#include <QUrl>

class PhabricatorRC : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl path READ path WRITE setPath NOTIFY dataChanged)
public:
    PhabricatorRC(QObject *parent = nullptr);

    void setPath(const QUrl &path);

    QUrl path() const
    {
        return m_path;
    }

Q_SIGNALS:
    void dataChanged();

private:
    QUrl m_path;
};

#endif
