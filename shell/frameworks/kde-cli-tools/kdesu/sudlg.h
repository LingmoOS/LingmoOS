/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * SPDX-FileCopyrightText: 2000 Geert Jansen <jansen@kde.org>
 * SPDX-License-Identifier: Artistic-2.0
 */

#ifndef __SuDlg_h_Included__
#define __SuDlg_h_Included__

#include <QByteArray>

#include <KPasswordDialog>
#include <KDESu/SuProcess>

using namespace KDESu;

class KDEsuDialog : public KPasswordDialog
{
    Q_OBJECT

public:
    KDEsuDialog(QByteArray user, QByteArray authUser, bool enableKeep, const QString &icon, bool withIgnoreButton);
    ~KDEsuDialog() override;

    enum ResultCodes { AsUser = 10 };

private Q_SLOTS:
    void slotUser1();

protected:
    bool checkPassword() override;

private:
    SuProcess proc;
};

#endif // __SuDlg_h_Included__
