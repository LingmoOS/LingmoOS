/*
    SPDX-FileCopyrightText: 2009 Andrey Batyiev <batyiev@gmail.com>
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PLASMA_NM_EDIT_LIST_DIALOG_H
#define PLASMA_NM_EDIT_LIST_DIALOG_H

#include <KEditListWidget>
#include <QDialog>

class QValidator;

class EditListDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditListDialog(QWidget *parent = nullptr);
    ~EditListDialog() override;

    void setItems(const QStringList &items);
    QStringList items() const;

    void setValidator(const QValidator *validator);
    const QValidator *validator() const;

    void setToolTip(const QString &toolTip);

Q_SIGNALS:
    void itemsEdited(const QStringList &items);

protected Q_SLOTS:
    void dialogAccepted();

private:
    KEditListWidget *const editListWidget;

    void removeEmptyItems(QStringList &list);
};

#endif // PLASMA_NM_EDIT_LIST_DIALOG_H
