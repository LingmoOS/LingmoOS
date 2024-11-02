/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef LISTVIEWDELEGATE_H
#define LISTVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include <QTextEdit>
#include "explor-core_global.h"
#include "list-view.h"

class QPushButton;

namespace Peony {

class TextEdit;

class ListViewDelegate : public QStyledItemDelegate
{
    friend class ListView;
    Q_OBJECT
public:
    explicit ListViewDelegate(QObject *parent = nullptr);
    ~ListViewDelegate() override;

    void initIndexOption(QStyleOptionViewItem *option,
                         const QModelIndex &index) const;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    //edit
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    //void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    int getCurrentCheckboxColumn(){
        return m_checkbox_column;
    }
    //QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index ) const override;
    void setSearchKeyword(QString regFindKeyWords);
    void paintLabel(QStyleOptionViewItem &opt, int aalignment, QList<QColor> colors, QPainter *painter) const;

Q_SIGNALS:
    void isEditing(bool editing) const;
    void requestDone(QWidget *editor);

private Q_SLOT:
    void slot_finishEdit();/* 编辑完成 */

private:
    QPushButton *m_styled_button;
    int m_checkbox_column =3;
    QString m_regFindKeyWords = "";
};

class TextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit TextEdit(QWidget *parent = nullptr);
    void adjustText();
    void setMaxLengthLimit(int length);
    void setLimitBytes(bool limitBytes);

Q_SIGNALS:
    void finishEditRequest();

protected:
    void keyPressEvent(QKeyEvent *e);

private:
    int m_max_length_limit = 0;
    bool m_limit_bytes = true;
};

}

#endif // LISTVIEWDELEGATE_H
