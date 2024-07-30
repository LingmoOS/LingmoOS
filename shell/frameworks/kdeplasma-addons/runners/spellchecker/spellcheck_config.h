/*
    SPDX-FileCopyrightText: 2008 Ryan P. Bitanga <ryan.bitanga@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SPELLCHECKCONFIG_H
#define SPELLCHECKCONFIG_H

#include <KCModule>

#include "ui_spellcheck_config.h"

class SpellCheckConfigForm : public QWidget, public Ui::SpellCheckConfigUi
{
    Q_OBJECT
public:
    explicit SpellCheckConfigForm(QWidget *parent);
};

class SpellCheckConfig : public KCModule
{
    Q_OBJECT
public:
    explicit SpellCheckConfig(QObject *parent, const KPluginMetaData &metaData);
    ~SpellCheckConfig() override;

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;

    void toggleTriggerWord(int state);
    void openKcm();

private:
    SpellCheckConfigForm *m_ui;
};

#endif
