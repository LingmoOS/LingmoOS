// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMENU_H
#define CMENU_H
#include <QMenu>

class CMenu : public QMenu
{
    Q_OBJECT
public:
    explicit CMenu(QWidget *parent = nullptr);
    explicit CMenu(const QString &title, QWidget *parent = nullptr);

protected:
    void hideEvent(QHideEvent *event) override;

private:
    //遍历设置菜单动作的状态
    void setActionEnableRecursive(QMenu *pMenu, bool enable);
};

#endif // CMENU_H
