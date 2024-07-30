/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#ifndef COMBOBOX_DELEGATE_H
#define COMBOBOX_DELEGATE_H

#include <QItemDelegate>

static const char *sTypes[] = {"int",
                               "qreal",
                               "QString",
                               "QStringList",
                               "QDateTime",
                               "QPoint",
                               "QFile",
                               "QDir",
                               "QUrl",
                               "QSize",
                               "QRect",
                               "QObject *",
                               "QVariant",
                               "QModelIndex",
                               "QColor",
                               "QWidget *",
                               "QAction *",
                               "QModelIndex"};

class ComboBoxDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    enum Type { NotEditable, Editable };
    ComboBoxDelegate(const QStringList &data, Type type = NotEditable, QObject *parent = 0);

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    mutable QHash<int, QSize> m_sizes;
};

#endif
