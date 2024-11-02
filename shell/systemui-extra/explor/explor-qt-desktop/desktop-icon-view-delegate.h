/*
 * Peony-Qt
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef DESKTOPICONVIEWDELEGATE_H
#define DESKTOPICONVIEWDELEGATE_H

#include <QStyledItemDelegate>

class QPushButton;

namespace Peony {

class DesktopIconView;

class DesktopIconViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit DesktopIconViewDelegate(QObject *parent = nullptr);
    ~DesktopIconViewDelegate() override;

    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        return QStyledItemDelegate::initStyleOption(option, index);
    }
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    DesktopIconView *getView() const;

    //初始化option
     void initIndexOption(QStyleOptionViewItem *option,
                          const QModelIndex &index) const;
protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    //edit
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

private Q_SLOT:
    void slot_finishEdit();/* 编辑完成 */

private:
    QPushButton *m_styled_button;
};

}

#endif // DESKTOPICONVIEWDELEGATE_H
