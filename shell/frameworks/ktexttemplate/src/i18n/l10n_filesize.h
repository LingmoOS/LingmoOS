/*
 * This file is part of the KTextTemplate library
 *
 * SPDX-FileCopyrightText: 2020 Matthias Fehring <mf@huessenbergnetz.de>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef L10N_FILESIZE_H
#define L10N_FILESIZE_H

#include "node.h"

namespace KTextTemplate
{
class Parser;
}

using namespace KTextTemplate;

class L10nFileSizeNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    L10nFileSizeNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class L10nFileSizeVarNodeFactory : public AbstractNodeFactory
{
    Q_OBJECT
public:
    L10nFileSizeVarNodeFactory();

    Node *getNode(const QString &tagContent, Parser *p) const override;
};

class L10nFileSizeNode : public Node
{
    Q_OBJECT
public:
    L10nFileSizeNode(const FilterExpression &size,
                     const FilterExpression &unitSystem,
                     const FilterExpression &precision,
                     const FilterExpression &multiplier,
                     QObject *parent = nullptr);

    void render(OutputStream *stream, Context *c) const override;

private:
    FilterExpression m_size;
    FilterExpression m_unitSystem;
    FilterExpression m_precision;
    FilterExpression m_multiplier;
};

class L10nFileSizeVarNode : public Node
{
    Q_OBJECT
public:
    L10nFileSizeVarNode(const FilterExpression &size,
                        const FilterExpression &unitSystem,
                        const FilterExpression &precision,
                        const FilterExpression &multiplier,
                        const QString &resultName,
                        QObject *parent = nullptr);

    void render(OutputStream *stream, Context *c) const override;

private:
    FilterExpression m_size;
    FilterExpression m_unitSystem;
    FilterExpression m_precision;
    FilterExpression m_multiplier;
    QString m_resultName;
};

#endif
