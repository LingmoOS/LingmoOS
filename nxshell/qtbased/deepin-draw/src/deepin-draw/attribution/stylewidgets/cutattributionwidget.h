// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUTATTRIBUTIONREGISTER_H
#define CUTATTRIBUTIONREGISTER_H
#include <QAbstractItemDelegate>
#include "iattributionregister.h"
#include <DToolButton>
#define CUTTYPE 6
#define MAXROWNUM 3
class QLabel;
class CutAttributionWidget: public AttributeWgt
{
    Q_OBJECT
public:
    explicit CutAttributionWidget(DrawBoard *drawBoard, QWidget *parent = nullptr);
    void initUi();
    void initConnect();
    void setCutSize(const QSize &sz, bool setattr);
    void setCutType(const int type);
    //重置裁剪属性
    void resetCutAttribution();
private:
    QSize getCutSzie();
private:
    DrawBoard *m_drawBoard = nullptr;
    QList<QToolButton * > m_buttonList;
    CSpinBox *h_spinbox = nullptr;
    CSpinBox *w_spinbox = nullptr;
    QToolButton *m_confirmbutton = nullptr;
    QToolButton *m_cancelbutton = nullptr;
    QButtonGroup *button_group = nullptr;
    const qreal Radio[CUTTYPE] {-1, -2, 1.0, 2.0 / 3.0, 8.0 / 5.0, 16.0 / 9.0};

    int cutstyle = 0;
    QSize m_cutCutSize;
};

#endif // CUTATTRIBUTIONREGISTER_H
