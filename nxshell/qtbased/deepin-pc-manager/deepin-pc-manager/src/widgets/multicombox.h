// Copyright (C) 2021 ~ 2021 Uniontech Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTICOMBOX_H
#define MULTICOMBOX_H

#include <QComboBox>
#include <QWidget>

class MenuItem;
class QLineEdit;
class QListView;
class QStandardItemModel;

class MultiComBox : public QComboBox
{
    Q_OBJECT

public:
    explicit MultiComBox(QWidget *parent = nullptr);
    ~MultiComBox() override;

    // 添加Iiem
    void addItems(QList<MenuItem *> menus);

    QStringList getItemsText();
    QList<int> getItemsId();

    void setActivated(int row);
    QString getSelectedRows();
    QString getSelectedIndexs();

Q_SIGNALS:
    void sendSelected(QList<int> list);

private:
    void updateText();

public Q_SLOTS:
    void sltActivated(const QModelIndex &index);

protected:
    void hidePopup() override;

private:
    QList<MenuItem> m_Menus;
    QLineEdit *m_pLineEdit;
    QListView *m_pListView;
    QStandardItemModel *m_pItemModel;
};

#endif // MULTICOMBOX_H
