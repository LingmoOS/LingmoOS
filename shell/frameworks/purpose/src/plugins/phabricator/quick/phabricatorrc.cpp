/*
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "phabricatorrc.h"

PhabricatorRC::PhabricatorRC(QObject *parent)
    : QObject(parent)
{
}

void PhabricatorRC::setPath(const QUrl &filePath)
{
    if (filePath == m_path || !filePath.isLocalFile()) {
        return;
    }
    m_path = filePath;

    //.arcconfig files are JSON files
    // TODO figure out the if/what/how of .arcconfig file contents
    Q_EMIT dataChanged();
}

#include "moc_phabricatorrc.cpp"
