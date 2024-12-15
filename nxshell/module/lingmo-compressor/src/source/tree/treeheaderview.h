// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TREEHEADERVIEW_H
#define TREEHEADERVIEW_H

#include <DLabel>
#include <DHeaderView>

DWIDGET_USE_NAMESPACE
#define SCROLLMARGIN 10
#define TABLE_HEIGHT_NormalMode  36
#define TABLE_HEIGHT_CompactMode 24


class TreeHeaderView;
// 返回上一级
class PreviousLabel: public Dtk::Widget::DLabel
{
    Q_OBJECT
public:
    explicit PreviousLabel(TreeHeaderView *parent = nullptr);
    ~ PreviousLabel() override;

    /**
     * @brief setPrePath    设置上一级路径
     * @param strPath   上一级路径
     */
    void setPrePath(const QString &strPath);

    /**
     * @brief isFocus Label是否focus
     * @return
     */
    bool isFocus() {return focusIn_;}
    /**
     * @brief setFocusValue 设置focusIn_
     * @return
     */
    void setFocusValue(bool bFocus);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

signals:
    void doubleClickedSignal();

private:
    TreeHeaderView *headerView_;

    bool focusIn_ = false;
};

// 表头
class TreeHeaderView : public DHeaderView
{
    Q_OBJECT
public:
    explicit TreeHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~TreeHeaderView() override;

    QSize sizeHint() const override;
    int sectionSizeHint(int logicalIndex) const;

    inline int getSpacing() const { return m_spacing; }
    inline void setSpacing(int spacing) { m_spacing = spacing; }

    /**
     * @brief getpreLbl    获取上一级指针
     * @return
     */
    PreviousLabel *getpreLbl();

//    /**
//     * @brief setPrePath    设置上一级路径
//     * @param strPath   上一级路径
//     */
//    void setPrePath(const QString &strPath);


    /**
     * @brief setPreLblVisible  设置上一级选项是否可见
     * @param bVisible  是否可见标志位
     */
    void setPreLblVisible(bool bVisible);

    /**
     * @brief isVisiable 获取表头是否可见
     * @return bool
     */
    bool isVisiable() {return m_pPreLbl->isVisible();}

    /**
     * @brief setLabelFocus 设置Label是否选中
     * @param focus bool
     */
    void setLabelFocus(bool focus);

    /**
     * @brief isLabelFocus Label是否选中
     * @return bool
     */
    bool isLabelFocus() {return m_pPreLbl->isFocus();}

protected:
    //void paintEvent(QPaintEvent *e) override;
    virtual void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

    void resizeEvent(QResizeEvent *event) override;

private:
    int m_spacing {1};

    PreviousLabel *m_pPreLbl;
};

#endif // TREEHEADERVIEW_H
