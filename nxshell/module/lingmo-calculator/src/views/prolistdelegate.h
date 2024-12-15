// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROLISTDELEGATE_H
#define PROLISTDELEGATE_H

#include <QStyledItemDelegate>
#include <QObject>
#include <DGuiApplicationHelper>

DGUI_USE_NAMESPACE

class ProListDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ProListDelegate(QObject *parent = nullptr);
    ~ProListDelegate() override;

public slots:
    void setThemeType(int type);
    void currentfocusindex(QModelIndex index);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                     const QModelIndex &index) override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void drawFocusStatus(QPainter *painter, const QStyleOptionViewItem &option) const;
signals:
    void obtainingHistorical(const QModelIndex &index);

private:
    int m_themeType = 0;
    QModelIndex m_focusindex;
};

#endif // PROLISTDELEGATE_H
