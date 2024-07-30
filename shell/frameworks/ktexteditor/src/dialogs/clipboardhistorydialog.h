/*
    SPDX-FileCopyrightText: 2022 Eric Armbruster <eric1@armbruster-online.de>
    SPDX-FileCopyrightText: 2022 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef CLIPBOARD_HISTORY_DIALOG_H
#define CLIPBOARD_HISTORY_DIALOG_H

#include "kateglobal.h"

#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPointer>
#include <QTreeView>

class ClipboardHistoryModel;
class ClipboardHistoryFilterModel;

namespace KTextEditor
{
class DocumentPrivate;
class ViewPrivate;
}

class ClipboardHistoryDialog : public QMenu
{
public:
    ClipboardHistoryDialog(QWidget *mainwindow, KTextEditor::ViewPrivate *mainWindow);

    void resetValues();
    void openDialog(const QList<KTextEditor::EditorPrivate::ClipboardEntry> &clipboardHistory);

private:
    void slotReturnPressed();

private:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void updateViewGeometry();
    void clearLineEdit();
    void showSelectedText(const QModelIndex &idx);
    void showEmptyPlaceholder();

private:
    QTreeView m_treeView;
    QLineEdit m_lineEdit;
    QPointer<QWidget> m_mainWindow;

    /*
     * View containing the currently open document
     */
    KTextEditor::ViewPrivate *m_viewPrivate;

    ClipboardHistoryModel *m_model;
    ClipboardHistoryFilterModel *m_proxyModel;

    /*
     * Document for the selected text to paste
     */
    KTextEditor::DocumentPrivate *m_selectedDoc;

    /*
     * View containing the selected text to paste
     */
    KTextEditor::ViewPrivate *m_selectedView;

    QLabel *m_noEntries;
};

#endif
