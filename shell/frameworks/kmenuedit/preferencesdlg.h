/*
 *   SPDX-FileCopyrightText: 2008 Laurent Montel <montel@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#ifndef PREFERENCESDLG_h
#define PREFERENCESDLG_h

#include <KPageDialog>

class SpellCheckingPage;
class MiscPage;
class QCheckBox;

namespace Sonnet
{
class ConfigWidget;
}

class PreferencesDialog : public KPageDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget *parent);

protected Q_SLOTS:
    void slotSave();

private:
    SpellCheckingPage *m_pageSpellChecking = nullptr;
    MiscPage *m_pageMisc = nullptr;
};

class SpellCheckingPage : public QWidget
{
    Q_OBJECT
public:
    explicit SpellCheckingPage(QWidget *);
    void saveOptions();

private:
    Sonnet::ConfigWidget *m_confPage = nullptr;
};

class MiscPage : public QWidget
{
    Q_OBJECT
public:
    explicit MiscPage(QWidget *);
    void saveOptions();

private:
    QCheckBox *m_showHiddenEntries = nullptr;
};

#endif
