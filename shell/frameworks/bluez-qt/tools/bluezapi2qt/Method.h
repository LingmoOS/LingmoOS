/*
 * BluezQt - Asynchronous BlueZ wrapper library
 *
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef METHOD_H
#define METHOD_H

#include "Comment.h"
#include "Parameter.h"

class Method
{
public:
    struct Tags {
        bool isOptional = false;
        bool isDeprecated = false;
        bool isExperimental = false;
    };

    Method();

    bool finalize();

    QString name() const;
    QList<Parameter> inParameters() const;
    QList<Parameter> outParameters() const;
    Parameter outParameter() const;
    Tags tags() const;
    QStringList comment() const;

private:
    QString guessOutParameterName() const;

    QString m_name;
    QStringList m_inParameterStrings;
    QStringList m_outParameterStrings;
    Parameter m_outParameter;
    QStringList m_stringTags;
    QString m_limitation;
    Comment m_comment;

    // finalized members
    Tags m_tags;
    QList<Parameter> m_inParameters;
    QList<Parameter> m_outParameters;

    friend class Methods;
};

#endif // METHOD_H
