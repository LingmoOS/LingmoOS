// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RESULTINFO_H
#define RESULTINFO_H

#include <DLabel>

DWIDGET_USE_NAMESPACE

class Resultinfo : public DLabel
{
    Q_OBJECT
public:
    explicit Resultinfo(QWidget *parent = nullptr);
    ~Resultinfo() override;

    void setResult(bool result);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    bool mResult = true; //对局结果
    QString strResult = ""; //文字结果
};

#endif // RESULTINFO_H
