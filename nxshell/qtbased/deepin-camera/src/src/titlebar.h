// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QScopedPointer>
#include <DTitlebar>
#include <DWidget>
#include <QHBoxLayout>
#include <DBlurEffectWidget>
#include <DLabel>
#include <QGraphicsDropShadowEffect>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

class TitlebarPrivate;

class Titlebar : public DBlurEffectWidget
{
    Q_OBJECT

public:
    Titlebar(QWidget *parent = 0);
    ~Titlebar();

    /**
     * @brief titlebar 返回DTitlebar指针
     */
    DTitlebar *titlebar();

public slots:
    /**
     * @brief slotThemeTypeChanged 主题变化事件槽函数
     */
    void slotThemeTypeChanged();

protected:
    /**
     * @brief paintEvent 绘制事件函数
     * @param pPaintEvent 绘制事件
     */
    virtual void paintEvent(QPaintEvent *pPaintEvent) override;

private:
    QScopedPointer<TitlebarPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), Titlebar)
};

#endif // TITLEBAR_H
