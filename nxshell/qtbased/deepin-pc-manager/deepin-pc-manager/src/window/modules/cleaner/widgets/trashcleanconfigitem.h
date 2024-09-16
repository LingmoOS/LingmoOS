// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCLEANCONFIGITEM_H
#define TRASHCLEANCONFIGITEM_H

#include <DFrame>
#include <DLabel>

#include <QCheckBox>
#include <QWidget>

DWIDGET_BEGIN_NAMESPACE
class DLable;
class DTipLabel;
class DBackgroundGroup;
DWIDGET_END_NAMESPACE

class TrashCleanConfigItem : public Dtk::Widget::DFrame
{
    Q_OBJECT

public:
    explicit TrashCleanConfigItem(QWidget *parent = nullptr);

    ~TrashCleanConfigItem() { }

    void setTitles(const QString &, const QString &);
    void setCheckBoxStatus(bool);
    void setIconPath(const QString &path);

    inline void setIndex(int index) { m_configIndex = index; }

Q_SIGNALS:
    void stageChanged(int, int);

private:
    void initUI();
    void initConnection();

private:
    QCheckBox *m_checkbox;
    Dtk::Widget::DLabel *m_icon;
    Dtk::Widget::DLabel *m_title;
    Dtk::Widget::DTipLabel *m_tips;
    Dtk::Widget::DBackgroundGroup *m_backgroup;
    Dtk::Widget::DFrame *m_textFrame;
    int m_configIndex;
};

#endif // TRASHCLEANCONFIGITEM_H
