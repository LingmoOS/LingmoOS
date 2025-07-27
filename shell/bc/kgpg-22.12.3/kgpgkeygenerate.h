/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2009, 2012, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGKEYGENERATE_H
#define KGPGKEYGENERATE_H

#include "ui_kgpgkeygenerate.h"

#include "core/kgpgkey.h"

#include <QDialog>
#include <QPushButton>


class KgpgKeyGenerate : public QDialog, public Ui::kgpgKeyGenerate
{
    Q_OBJECT

public:
    explicit KgpgKeyGenerate(QWidget *parent = nullptr);

    bool isExpertMode() const;
    QString name() const;
    QString email() const;
    QString comment() const;
    KgpgCore::KgpgKeyAlgo algo() const;

    /**
     * @brief return the selected capabilities for the new key
     * @retval 0 default capabilities of the selected algorithm should be used
     */
    KgpgCore::KgpgSubKeyType caps() const;
    uint size() const;
    char expiration() const;
    uint days() const;

private Q_SLOTS:
    void slotOk();
    void slotUser1();
    void slotEnableOk();
    void slotEnableDays(const int state);
    void slotEnableCaps(const int state);

private:
    QPushButton *okButton;

    bool m_expert;
};

#endif // KGPGKEYGENERATE_H
