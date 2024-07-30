/*
    SPDX-FileCopyrightText: KDE Developers

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATEVI_CONFIG_TAB_H
#define KATEVI_CONFIG_TAB_H

#include <dialogs/kateconfigpage.h>
#include <vimode/mappings.h>

class QTableWidget;

namespace KateVi
{
namespace Ui
{
class ConfigWidget;
}

class ConfigTab : public KateConfigPage
{
public:
    explicit ConfigTab(QWidget *parent, Mappings *mappings);
    ~ConfigTab() override;

    QString name() const override;

protected:
    Ui::ConfigWidget *ui;

private:
    void applyTab(QTableWidget *mappingsTable, Mappings::MappingMode mode);
    void reloadTab(QTableWidget *mappingsTable, Mappings::MappingMode mode);

public:
    void apply() override;
    void reload() override;
    void reset() override;
    void defaults() override;

private:
    void showWhatsThis(const QString &text);
    void addMappingRow();
    void removeSelectedMappingRows();
    void importNormalMappingRow();

private:
    Mappings *m_mappings;
};
}

#endif /* KATEVI_CONFIG_TAB_H */
