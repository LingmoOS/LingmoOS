/* This file is part of the KDE project

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <ktexteditor/configpage.h>

class KateProjectPlugin;
class QWidget;
class QCheckBox;
class KUrlRequester;
class QComboBox;

class KateProjectConfigPage : public KTextEditor::ConfigPage
{
public:
    explicit KateProjectConfigPage(QWidget *parent = nullptr, KateProjectPlugin *plugin = nullptr);
    ~KateProjectConfigPage() override
    {
    }

    QString name() const override;
    QString fullName() const override;
    QIcon icon() const override;

public:
    void apply() override;
    void defaults() override;
    void reset() override;

private:
    void slotMyChanged();

private:
    void setupGitConfigUI();

    QCheckBox *m_cbSessionRestoreOpenProjects;
    QCheckBox *m_cbAutoGit;
    QCheckBox *m_cbAutoSubversion;
    QCheckBox *m_cbAutoMercurial;
    QCheckBox *m_cbAutoFossil;
    QCheckBox *m_cbAutoCMake;
    QCheckBox *m_cbIndexEnabled;
    KUrlRequester *m_indexPath;
    QCheckBox *m_cbMultiProjectCompletion;
    QCheckBox *m_cbMultiProjectGoto;
    QComboBox *m_cmbSingleClick;
    QComboBox *m_cmbDoubleClick;
    KateProjectPlugin *m_plugin;
    bool m_changed = false;
};
