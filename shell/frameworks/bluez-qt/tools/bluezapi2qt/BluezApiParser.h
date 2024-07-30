/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef PARSER_H
#define PARSER_H

#include <QTextStream>

#include "Interface.h"

class BluezApiParser
{
public:
    BluezApiParser();

    bool parse(QTextStream &stream);
    bool finalize();

    std::list<Interface> interfaces() const;

private:
    std::list<Interface> m_interfaces;
    Interface *m_currentInterface = nullptr;
};

#endif // PARSER_H
