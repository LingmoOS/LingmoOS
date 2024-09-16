// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef SELECTACTIONPAGE_H
#define SELECTACTIONPAGE_H

#include "contentpairpage.h"

class IconLabel;
class UtilityList;
class SelectActionPage : public ContentPairPage
{
    Q_OBJECT

public:
    enum Mode {
        Disk,
        Partition
    };
    Q_ENUM(Mode)

    enum Action {
        Clone,
        Backup,
        Restore
    };
    Q_ENUM(Action)

    explicit SelectActionPage(QWidget *parent = 0);

    Mode mode() const;
    Action action() const;

    void setMode(Mode mode);
    void setAction(Action action);

    QString selectedItemTitle() const;
    QIcon selectedItemIcon() const;

private:
    void init();

    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

    Mode m_mode = Disk;
    Action m_action = Clone;
    IconLabel *m_diskItem = NULL;
    IconLabel *m_partItem = NULL;
    UtilityList *m_rightContent = NULL;
};

#endif // SELECTACTIONPAGE_H
