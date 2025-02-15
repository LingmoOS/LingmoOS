/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KLICENSEDIALOG_P_H
#define KLICENSEDIALOG_P_H

// Qt
#include <QDialog>

class KAboutLicense;

/**
 * @internal
 *
 * A dialog to display a license
 */
class KLicenseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KLicenseDialog(const KAboutLicense &license, QWidget *parent = nullptr);
    ~KLicenseDialog() override;

private:
    Q_DISABLE_COPY(KLicenseDialog)
};

#endif
