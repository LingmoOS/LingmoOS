// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FRAME_H
#define FRAME_H

#include <QFrame>

class Frame : public QFrame
{
    Q_OBJECT
public:
    Frame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
signals:
    void sigFrameResize();
protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // FRAME_H
