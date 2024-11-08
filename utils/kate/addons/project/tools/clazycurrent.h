/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#pragma once

#include "clazy.h"

class KateProjectCodeAnalysisToolClazyCurrent : public KateProjectCodeAnalysisToolClazy
{
public:
    explicit KateProjectCodeAnalysisToolClazyCurrent(QObject *parent);

    QString name() const override;
    QString description() const override;
    QStringList arguments() override;
};
