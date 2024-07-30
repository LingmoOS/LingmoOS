/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef INTERFACE_H
#define INTERFACE_H

#include "Methods.h"
#include "Properties.h"

class Interface
{
public:
    Interface();

    bool parse(const QString &line);
    bool finalize();

    QStringList comment() const;
    QString service() const;
    QString name() const;
    QString objectPath() const;
    Methods methods() const;
    Properties properties() const;

private:
    enum class State {
        Comment,
        Service,
        Interface,
        ObjectPath,
        Methods,
        Properties,
    };

    void parseComment(const QString &line);
    void parseService(const QString &line);
    void parseInterface(const QString &line);
    void parseObjectPath(const QString &line);

    State m_state = State::Comment;

    QStringList m_comment;
    QString m_service;
    QString m_name;
    QString m_objectPath;
    Methods m_methods;
    Properties m_properties;
};

#endif // INTERFACE_H
