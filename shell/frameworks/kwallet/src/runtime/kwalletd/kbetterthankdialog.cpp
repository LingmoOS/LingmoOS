/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2004 George Staikos <staikos@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kbetterthankdialog.h"
#include <QIcon>

KBetterThanKDialog::KBetterThanKDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(_allowOnce, &QPushButton::clicked, this, &KBetterThanKDialog::allowOnceClicked);
    connect(_allowAlways, &QPushButton::clicked, this, &KBetterThanKDialog::allowAlwaysClicked);
    connect(_deny, &QPushButton::clicked, this, &KBetterThanKDialog::denyClicked);
    connect(_denyForever, &QPushButton::clicked, this, &KBetterThanKDialog::denyForeverClicked);

    init();
}

void KBetterThanKDialog::init()
{
    _allowOnce->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok")));
    _allowAlways->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok")));
    _deny->setIcon(QIcon::fromTheme(QStringLiteral("dialog-cancel")));
    _denyForever->setIcon(QIcon::fromTheme(QStringLiteral("dialog-cancel")));

    _allowOnce->setFocus();
}

void KBetterThanKDialog::setLabel(const QString &label)
{
    _label->setText(label);
}

void KBetterThanKDialog::accept()
{
    setResult(0);
}

void KBetterThanKDialog::reject()
{
    QDialog::reject();
    setResult(2);
}

void KBetterThanKDialog::allowOnceClicked()
{
    done(0);
}

void KBetterThanKDialog::allowAlwaysClicked()
{
    done(1);
}

void KBetterThanKDialog::denyClicked()
{
    done(2);
}

void KBetterThanKDialog::denyForeverClicked()
{
    done(3);
}

#include "moc_kbetterthankdialog.cpp"
