/* SPDX-FileCopyrightText: 2010 Anton Kreuzkamp <akreuzkamp@web.de>
 * SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef CHARRUNNERCONFIG_H
#define CHARRUNNERCONFIG_H

#include "ui_charrunner_config.h"

// KF
#include <KCModule>

class CharacterRunnerConfigForm : public QWidget, public Ui::CharacterRunnerConfigUi
{
    Q_OBJECT

public:
    explicit CharacterRunnerConfigForm(QWidget *parent);
};

class CharacterRunnerConfig : public KCModule
{
    Q_OBJECT

public:
    explicit CharacterRunnerConfig(QObject *parent, const KPluginMetaData &metaData);

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;

private Q_SLOTS:
    void addItem();
    void deleteItem();
    void validateAddButton();
    void validateDeleteButton();

private:
    CharacterRunnerConfigForm *m_ui;
};
#endif
