// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHOW_BUTTONS_H
#define SHOW_BUTTONS_H

#include <DWidget>
#include <QObject>
#include <QMap>
#include <QVector>

DWIDGET_USE_NAMESPACE

class ShowButtons : public DWidget
{
    Q_OBJECT
public:
    static const struct KeyDescription {
        QString text;
        char x11Key;
        int Qtkey; // wayland 用Qtkey码
    } keyDescriptions[];


    explicit ShowButtons(DWidget *parent = nullptr);

signals:
    void keyShowSignal(const QString &key);

public slots:
     // x11 下按键按下
    void showContentButtons(unsigned char keyCode);
     // x11 下按键释放
    void releaseContentButtons(unsigned char keyCode);
    // Wayland 下按键按下
    void showContentButtons(const int key);
    // Wayland 下按键释放
    void releaseContentButtons(const int key);
    QString getKeyCodeFromEvent(unsigned char keyCode);
    QString getKeyCodeFromEventWayland(const int keyCode);

private:
//    QMap<QString, DPushButton*> m_textButtonMap;
//    QString m_keyCode;//保存上次的输入按钮，防止长按
    QVector<QString> m_keyCodeVec;//保存上次的输入按钮，防止长按
};

#endif // SHOW_BUTTONS_H
