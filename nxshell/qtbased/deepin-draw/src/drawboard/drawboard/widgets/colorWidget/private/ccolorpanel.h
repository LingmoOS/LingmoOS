// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CCOLORPANEL_H
#define CCOLORPANEL_H

#include <QPushButton>
#include <QWidget>
#include <QColor>
#include <QPaintEvent>
#include <QApplication>
#include "editlabel.h"
#include "globaldefine.h"
#include "adaptivedef.h"

class CIconButton;

#ifdef USE_DTK
#include <DLineEdit>
DWIDGET_USE_NAMESPACE
#else
#include <QLineEdit>
#endif

class PanelWidget: public QWidget
{
    Q_OBJECT
public:
    PanelWidget(QWidget *parent = nullptr);
protected:
    virtual void paintEvent(QPaintEvent *event) override;

};

class CColorButton : public QPushButton
{
    Q_OBJECT
public:
    CColorButton(const QColor &color, QWidget *parent = nullptr);
    ~CColorButton();

    /**
     * @brief color  按钮代表的颜色
     * @param disable
     */
    QColor color();

protected:
    void paintEvent(QPaintEvent *);

private:
    const QColor m_color;
};

class CAlphaControlWidget;
class PickColorWidget;
class CColorPickWidget;

class ColorPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPanel(QWidget *parent = nullptr, bool bOldStyle = true);

    ~ColorPanel() override;

    /**
     * @brief updateColor 设置颜色
     */
    void setColor(const QColor &c, bool internalChanged = true, EChangedPhase phase = EChanged);

    /**
     * @brief color 当前颜色
     */
    QColor color();

    /**
     * @brief colorLineEditor 颜色编辑控件
     */
    DLineEdit *colorLineEditor();

    /**
     * @brief setExpandWidgetVisble 设置扩展界面是否显示
     */
    void setExpandWidgetVisble(bool visble);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    virtual void resizeEvent(QResizeEvent *event) override;
signals:
    void colorChanged(const QColor &color, EChangedPhase phase);
    void sizeChanged();
public slots:
    /**
     * @brief setTheme　改变按钮主题
     */
    void setTheme(int theme);

    PRIVATECLASS(ColorPanel)
};

#endif // CCOLORPANEL_H
