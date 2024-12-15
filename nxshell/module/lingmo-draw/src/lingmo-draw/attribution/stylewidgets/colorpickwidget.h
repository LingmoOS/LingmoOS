// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COLORPICKWIDGET_H
#define COLORPICKWIDGET_H
#include <DArrowRectangle>

DWIDGET_USE_NAMESPACE
class ColorSettingWgt;

class ColorPickWidget : public DArrowRectangle
{
    Q_OBJECT
public:
    ColorPickWidget(QWidget *parent = nullptr);
    void  setVar(const QVariant &var);
    QWidget *caller();
    void setShowPos(const QPoint &pos);

    void setCaller(QWidget *pCaller);
signals:
    void colorChanged(const QColor &color, int phase);
protected:
    virtual void paintEvent(QPaintEvent *) override;
    virtual void resizeEvent(QResizeEvent *e) override;
public slots:
    void setColor(const QColor &color, int phase);
private:
    QColor      m_color;
    ColorSettingWgt *m_colorSettingWgt;
    QWidget      *m_caller = nullptr;
    QPoint      m_point;
};

#endif // COLORPICKWIDGET_H
