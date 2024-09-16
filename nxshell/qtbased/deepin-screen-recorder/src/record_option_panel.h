// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECORDOPTIONALPANEL_H
#define RECORDOPTIONALPANEL_H

#include <DPushButton>
#include "settings.h"

DWIDGET_USE_NAMESPACE

class RecordOptionPanel : public DPushButton
{
    Q_OBJECT

    static const int WIDTH;
    static const int HEIGHT;
    static const int ICON_OFFSET_X;

public:
    explicit RecordOptionPanel(DPushButton *parent = nullptr);
    ~RecordOptionPanel();
    //bool isSaveAsGif();

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *, QEvent *event);

private:
    bool saveAsGif;
    bool saveAsMkv;
    bool isPressGif;
    bool isPressVideo;

    QPixmap gifNormalImg;
    QPixmap gifPressImg;
    QPixmap gifCheckedImg;

    QPixmap videoNormalImg;
    QPixmap videoPressImg;
    QPixmap videoCheckedImg;

    Settings *settings;
};

#endif // RECORDOPTIONPANEL_H
