/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kdeplatformfiledialogbase_p.h"

KDEPlatformFileDialogBase::KDEPlatformFileDialogBase()
{
}

void KDEPlatformFileDialogBase::closeEvent(QCloseEvent *e)
{
    Q_EMIT closed();
    QDialog::closeEvent(e);
}

#include "moc_kdeplatformfiledialogbase_p.cpp"
