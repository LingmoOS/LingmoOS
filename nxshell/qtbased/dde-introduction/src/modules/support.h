// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SUPPORT_H
#define SUPPORT_H

#include <QScrollArea>
#include <DGuiApplicationHelper>
#include <DLabel>
#include <DFrame>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class Support : public QScrollArea
{
    Q_OBJECT
public:
    explicit Support(QWidget *parent = nullptr);

signals:

public slots:
//    void initTheme(int type);
private:
    DLabel *m_label;
    QString m_text;
};

#endif // SUPPORT_H
