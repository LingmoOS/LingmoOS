// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DAYHUANGLILABEL_H
#define DAYHUANGLILABEL_H

#include <DLabel>

DWIDGET_USE_NAMESPACE
class CDayHuangLiLabel : public DLabel
{
    Q_OBJECT

public:
    explicit CDayHuangLiLabel(QWidget *parent = nullptr);
    void setbackgroundColor(QColor backgroundColor);
    void setTextInfo(QColor tcolor, QFont font);
    void setHuangLiText(QStringList vhuangli, int type = 0);

protected:
    void paintEvent(QPaintEvent *e) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QStringList m_vHuangli;
    QColor m_backgroundColor;
    QColor m_textcolor;
    QFont m_font;
    int m_type = 0;
    int m_leftMagin = 14;
    int m_topMagin = 18;
};

#endif // DAYHUANGLILABEL_H
