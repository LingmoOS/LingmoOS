/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef KBETTERTHANKDIALOG_H
#define KBETTERTHANKDIALOG_H

#include <QDialog>

#include "ui_kbetterthankdialogbase.h"

class KBetterThanKDialog : public QDialog, private Ui_KBetterThanKDialogBase
{
    Q_OBJECT

public:
    explicit KBetterThanKDialog(QWidget *parent = nullptr);

    void init();
    void setLabel(const QString &label);

protected Q_SLOTS:
    void accept() override;
    void reject() override;

private Q_SLOTS:
    void allowOnceClicked();
    void allowAlwaysClicked();
    void denyClicked();
    void denyForeverClicked();
};

#endif
