// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H

#include <DMenu>
DWIDGET_USE_NAMESPACE
enum SaveAction : unsigned int;
class MenuController : public QObject
{
    Q_OBJECT
public:
    explicit MenuController(QObject *parent = nullptr);
    ~MenuController();

signals:
    void shapePressed(QString currentShape);
    void unDoAction();
    void saveBtnPressed(SaveAction action);
    void saveAction();
    void closeAction();
    void menuNoFocus();

public slots:
    void showMenu(QPoint pos);
    void setUndoEnable(bool status);

protected:
    void enterEvent(QEvent *e);
private:
    DMenu *m_menu;
    QAction *m_unDoAct;
    QAction *m_saveAct;
    QAction *m_closeAct;
    qreal m_ration;
};
#endif // MENUCONTROLLER_H
