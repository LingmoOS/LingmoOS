/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/

#ifndef _NEWTYPEDLG_H
#define _NEWTYPEDLG_H

#include <QDialog>

class QDialogButtonBox;
class KLineEdit;
class QComboBox;

/**
 * A dialog for creating a new file type, with
 * - a combobox for choosing the group
 * - a line-edit for entering the name of the file type
 * The rest (description, patterns, icon, apps) can be set later in the filetypesview anyway.
 */
class NewTypeDialog : public QDialog
{
public:
    explicit NewTypeDialog(const QStringList &groups, QWidget *parent);
    QString group() const;
    QString text() const;

private:
    KLineEdit *m_typeEd;
    QComboBox *m_groupCombo;
    QDialogButtonBox *m_buttonBox;
};

#endif
