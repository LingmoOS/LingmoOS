// SPDX-FileCopyrightText: 2019 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DROUNDBGFRAME_H
#define DROUNDBGFRAME_H

#include <DLabel>

DWIDGET_USE_NAMESPACE

class DRoundBgFrame : public QWidget
{
public:
    DRoundBgFrame(QWidget *parent = nullptr, int bgOffsetTop = 0, int bgOffsetBottom = 0);

    void paintEvent(QPaintEvent *) override;

public slots:
    void slotShowHideTopBg(bool bShow);
    void slotShowHideBottomBg(bool bShow);

private:
    int m_bgOffsetTop = 0;
    int m_bgOffsetBottom = 0;
};

#endif  // DROUNDBGFRAME_H
