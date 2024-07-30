/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef METHODS_H
#define METHODS_H

#include "Method.h"

#include <list>

class Methods
{
public:
    Methods();

    static bool isMethod(const QString &line);

    void parse(const QString &line);
    bool finalize();

    std::list<Method> methods() const;

private:
    std::list<Method> m_methods;
    Method *m_currentMethod = nullptr;
};

#endif // METHODS_H
