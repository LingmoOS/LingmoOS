// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASE_COMMANDLINK_BUTTON_H
#define BASE_COMMANDLINK_BUTTON_H

#include <DCommandLinkButton>
/**
 * @brief The BaseCommandLinkButton class The DCommandLinkButton subclass,which will be catched the keyBoard Event.
 */
class BaseCommandLinkButton : public Dtk::Widget::DCommandLinkButton
{
    Q_OBJECT
public:
    explicit BaseCommandLinkButton(const QString name, QWidget *parent);
    virtual ~BaseCommandLinkButton();

protected:
    /**
     * @brief eventFilter Filters events if this object has been installed as an event filter for the watched object.
     * @param obj the obj which will be filter.
     * @param event currentBaseCommandLinkButton Event.
     * @return return the eventFilter function will be complete or uncomplete.
     */
    bool eventFilter(QObject *obj, QEvent *event);
};

#endif // BASE_COMMANDLINK_BUTTON_H
