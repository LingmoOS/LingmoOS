// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file 此文件实现标题栏相关
 */
#ifndef DMR_TITLEBAR_H
#define DMR_TITLEBAR_H 
#include <QScopedPointer>
#include <DTitlebar>
#include <DWidget>
#include <QHBoxLayout>
#include <DBlurEffectWidget>
#include <DLabel>
#include <QGraphicsDropShadowEffect>
#include <DFontSizeManager>
#include <QTimer>

DWIDGET_USE_NAMESPACE

namespace dmr {
class FullScreenTitlebar : public QFrame
{
    Q_OBJECT
public:
    FullScreenTitlebar(QWidget *parent = 0);

    void setTitletxt(const QString &sTitle);
    void setTime(const QString &sTime);

protected:
    void paintEvent(QPaintEvent *pPaintEvent) override;

private:
    DLabel     *m_iconLabel;
    DLabel     *m_textLabel;
    DLabel     *m_timeLabel;
};

class TitlebarPrivate;
/**
 * @brief The Titlebar class
 * 实现标题栏
 */
class Titlebar : public DBlurEffectWidget
{
    Q_OBJECT

public:
    /**
     * @brief Titlebar 构造函数
     * @param parent 父窗口
     */
    explicit Titlebar(QWidget *parent = 0);
    ~Titlebar();

    /**
     * @brief titlebar 获取标题栏对象指针
     * @return titlebar指针
     */
    DTitlebar *titlebar();
    /**
     * @brief setTitletxt 设置标题栏文本
     * @param title 标题栏文本
     */
    void setTitletxt(const QString &sTitle);
    /**
     * @brief setTitleBarBackground 设置标题栏背景是否为播放状态样式
     * @param flag 传入是否为播放状态
     */
    void setTitleBarBackground(bool flag);
    /**
     * @brief setIcon设置标图标
     * @param mp 图标
     */
    void setIcon(QPixmap& mp);
public slots:
	//把lambda表达式改为槽函数，modify by myk
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
}
#endif /* ifndef DMR_TITLEBAR_H */
