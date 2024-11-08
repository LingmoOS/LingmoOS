/*  This file is part of the Kate project.
 *
 *  SPDX-FileCopyrightText: 2017 Héctor Mesa Jiménez <hector@lcc.uma.es>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "../kateprojectcodeanalysistool.h"

/**
 * Information provider for flake8
 */
class KateProjectCodeAnalysisToolFlake8 : public KateProjectCodeAnalysisTool
{
public:
    explicit KateProjectCodeAnalysisToolFlake8(QObject *parent = nullptr);

    ~KateProjectCodeAnalysisToolFlake8() override;

    QString name() const override;

    QString description() const override;

    QString fileExtensions() const override;

    virtual QStringList filter(const QStringList &files) const override;

    QString path() const override;

    QStringList arguments() override;

    QString notInstalledMessage() const override;

    FileDiagnostics parseLine(const QString &line) const override;

    QString stdinMessages() override;
};
