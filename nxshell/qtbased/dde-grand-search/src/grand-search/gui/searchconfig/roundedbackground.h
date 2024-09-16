// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ROUNDEDBACKGROUND_H
#define ROUNDEDBACKGROUND_H

#include <DWidget>

namespace GrandSearch {

class RoundedBackground : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit RoundedBackground(QWidget *parent = nullptr);
    inline void setBottomRound(bool round) {
        m_bottomRound = round;
    }
    inline void setTopRound(bool round) {
        m_topRound = round;
    }
protected:
    void paintEvent(QPaintEvent *event);
protected:
    bool m_topRound = false;
    bool m_bottomRound = false;
};

}

#endif // ROUNDEDBACKGROUND_H
