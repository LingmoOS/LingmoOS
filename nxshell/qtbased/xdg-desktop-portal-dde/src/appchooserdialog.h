// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include <QDialog>
#include <QStringList>

class QListView;
class QPushButton;
class AppChooserDialog : public QDialog
{
public:
    explicit AppChooserDialog(QWidget *parent = nullptr);

    QStringList selectChoices();
    void updateChoices(const QStringList &choices);
    void setCurrentChoice(const QString &choice);

private:
    QListView *m_view;
    QStringList m_choices;

    QPushButton *m_cancelBtn;
    QPushButton *m_confirmBtn;
};
