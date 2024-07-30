/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include "Property.h"

#include <list>

class Properties
{
public:
    Properties();

    void parse(const QString &line);
    bool finalize();

    std::list<Property> properties() const;

private:
    std::list<Property> m_properties;
    Property *m_currentProperty = nullptr;
};

#endif // PROPERTIES_H
