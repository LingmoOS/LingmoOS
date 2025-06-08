/*
    SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "AbstractJob.h"

class ScriptJob : public AbstractJob
{
    Q_OBJECT

public:
    void executeOperation(const QFileInfo &fileInfo, const QString &mimeType, bool install) override;

private:
    QString formatScriptCommand(bool install, const QString &installerPath);
};
