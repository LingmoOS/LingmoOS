// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPICKCOLORWIDGET_H
#define CPICKCOLORWIDGET_H

#include <QWidget>

#include "editlabel.h"
#include "colorlabel.h"
#include "colorslider.h"
#include "globaldefine.h"

#ifdef USE_DTK
DWIDGET_USE_NAMESPACE
#endif

class CIconButton;
class ColorPickerInterface;
class CAlphaControlWidget;

class DRAWLIB_EXPORT PickColorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PickColorWidget(QWidget *parent, bool bUseOldUi = false);

    ~PickColorWidget();

    /**
     * @brief color 当前颜色
     */
    QColor color();

    /**
     * @brief setColor 设置颜色
     * @param c　颜色
     * @param internalChanged　由内变化
     */
    void setColor(const QColor &c, bool internalChanged = true);
    void setAlpha(int alpha, bool internalChanged);

    /**
     * @brief setTheme　更新按钮主题
     */
    void setTheme(int);
    void setColor(const QColor &c,
                  bool internalChanged,
                  EChangedPhase phase);
protected:
    void hideEvent(QHideEvent *event) override;

signals:
    /**
     * @brief colorChanged　获取颜色信号
     * @param color　颜色
     */
    void colorChanged(const QColor &color);
    void colorChanged(const QColor &color, EChangedPhase phase);

    /**
     * @brief previewedColorChanged　颜色预览信号
     * @param color　颜色
     */
    void previewedColorChanged(const QColor &color);
    void heightChanged();

private:
    /**
     * @brief updateColor　更新颜色
     */
    void updateColor(const QColor &color = QColor());
    void initUI();
    void initConnects();
    void initOldUi();
private:
    DLineEdit *m_hexLineEdit;
    EditLabel *m_redEditLabel;
    EditLabel *m_greenEditLabel;
    EditLabel *m_blueEditLabel;

    CIconButton *m_picker;
    ColorLabel *m_colorLabel;
    ColorSlider *m_colorSlider;
    ColorPickerInterface *m_cp;
    CAlphaControlWidget *m_alphaControlWidget;
    QColor curColor;
};

#endif // CPICKCOLORWIDGET_H
