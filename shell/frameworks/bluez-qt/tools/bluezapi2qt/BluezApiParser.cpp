/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "BluezApiParser.h"

BluezApiParser::BluezApiParser()
{
}

bool BluezApiParser::parse(QTextStream &stream)
{
    while (!stream.atEnd()) {
        // Get next line
        auto line = stream.readLine();
        // Just look for section markers
        if (line.startsWith(QLatin1Char('='))) {
            m_interfaces.emplace_back(Interface());
            m_currentInterface = &m_interfaces.back();
        } else if (m_currentInterface) {
            if (!m_currentInterface->parse(line)) {
                m_currentInterface = nullptr;
            }
        }
    }

    return true;
}

bool BluezApiParser::finalize()
{
    bool success = true;

    m_interfaces.erase(std::remove_if(m_interfaces.begin(),
                                      m_interfaces.end(),
                                      [](const Interface &interface) {
                                          return interface.methods().methods().empty() && interface.properties().properties().empty();
                                      }),
                       m_interfaces.end());

    for (auto &interface : m_interfaces) {
        success &= interface.finalize();
    }

    return success;
}

std::list<Interface> BluezApiParser::interfaces() const
{
    return m_interfaces;
}
