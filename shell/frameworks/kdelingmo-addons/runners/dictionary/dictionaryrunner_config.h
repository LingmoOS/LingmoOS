/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 * SPDX-FileCopyrightText: 2010, 2012 Jason A. Donenfeld <Jason@zx2c4.com>
 */

#ifndef DICTIONARYRUNNERCONFIG_H
#define DICTIONARYRUNNERCONFIG_H

#include <KCModule>
class QLineEdit;

static const char CONFIG_TRIGGERWORD[] = "triggerWord";

class DictionaryRunnerConfig : public KCModule
{
    Q_OBJECT

public:
    explicit DictionaryRunnerConfig(QObject *parent, const KPluginMetaData &metaData);

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;

private:
    QLineEdit *m_triggerWord;
};
#endif
