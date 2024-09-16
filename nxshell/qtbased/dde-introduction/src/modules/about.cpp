// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "about.h"

//关于页面 暂时不用
About::About(QWidget *parent)
    : QScrollArea(parent)
{
    DLabel *label = new DLabel;
    label->setWordWrap(true);
    setWidgetResizable(true);
    setFocusPolicy(Qt::NoFocus);
    setFrameStyle(QFrame::NoFrame);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Expanding);
    setContentsMargins(0, 0, 0, 0);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setWidget(label);

    label->setMargin(20);
    label->setOpenExternalLinks(true);

    /*QString text = QString("<br><p style=\"font-family: SourceHanSansSC-Normal;font-size: 12px;color: #303030;line-height: 18px;text-indent:26px;\">%1<a href=https://github.com>%2</a>%3</p>"
                   "<p style=\"font-family: SourceHanSansSC-Medium;font-size: 16px;color: #000000;line-height: 18px;\">%4"
                   "<p style=\"font-family: SourceHanSansSC-Medium;font-size: 12px;color: #303030;line-height: 18px;\">%5</p>"
                   "<p style=\"font-family: SourceHanSansSC-Medium;font-size: 16px;color: #000000;line-height: 18px;\">%6"
                   "<p style=\"font-family: SourceHanSansSC-Medium;font-size: 12px;color: #303030;line-height: 18px;\">%7</br>"
                   "<br style=\"font-family: SourceHanSansSC-Medium;font-size: 12px;color: #303030;line-height: 18px;\">%8</br>"
                   "<br style=\"font-family: SourceHanSansSC-Medium;font-size: 12px;color: #303030;line-height: 18px;\">%9</br>"
                   "<br style=\"font-family: SourceHanSansSC-Medium;font-size: 12px;color: #303030;line-height: 18px;\">%10</p>"
                   "<p style=\"font-family: SourceHanSansSC-Medium;font-size: 16px;color: #000000;line-height: 18px;\">%11")
           .arg(("    Deep is an open source operating system development team in China, which has a history of 14 years. Many of our members have benefited from open source projects around the world, and based on the Linux open source project, the Deep Team has also received some business revenue to support its development. Therefore, it is very desirable to feed back the open source community through the deep operating system, so that more users around the world can use the best open source Linux operating system products. Our source code is open on "))
           .arg(("github"))
           .arg((", and anyone who has doubts is welcome to know or use our code."))
           .arg(("■target"))
           .arg(("The World's Most Beautiful and Useful Linux Distribution"))
           .arg(("■History"))
           .arg(("Hiweed Linux was first released in 2004"))
           .arg(("Hiweed Linux renamed deepin in 2008"))
           .arg(("A new V15 version will be released in 2016"))
           .arg(("X Month X, 2018, latest version 1.5.1"))
           .arg(("■Open source projects"));

    label->setText(text);*/
}
