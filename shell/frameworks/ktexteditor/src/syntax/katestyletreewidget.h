/*
    SPDX-FileCopyrightText: 2001-2003 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Mirko Stocker <me@misto.ch>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATESTYLETREEWIDGET_H
#define KATESTYLETREEWIDGET_H

#include <QTreeWidget>

#include "attribute.h"

/**
 * QTreeWidget that automatically adds columns for KateStyleListItems and provides a
 * popup menu and a slot to edit a style using the keyboard.
 * Added by anders, jan 23 2002.
 */
class KateStyleTreeWidget : public QTreeWidget
{
    Q_OBJECT

    friend class KateStyleListItem;

public:
    explicit KateStyleTreeWidget(QWidget *parent = nullptr, bool showUseDefaults = false);

    void emitChanged();

    void addItem(QTreeWidgetItem *parent,
                 const QString &styleName,
                 KTextEditor::Attribute::Ptr defaultstyle,
                 KTextEditor::Attribute::Ptr data = KTextEditor::Attribute::Ptr());
    void addItem(const QString &styleName, KTextEditor::Attribute::Ptr defaultstyle, KTextEditor::Attribute::Ptr data = KTextEditor::Attribute::Ptr());

    void resizeColumns();

    bool readOnly() const;
    void setReadOnly(bool readOnly);

Q_SIGNALS:
    void changed();

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void showEvent(QShowEvent *event) override;
    bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) override;

private Q_SLOTS:
    void changeProperty();
    void unsetColor();
    void updateGroupHeadings();

private:
    bool m_readOnly = false;
};

#endif
