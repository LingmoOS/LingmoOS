// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#if 0

#ifndef DMENUCOMBOBOX_H
#define DMENUCOMBOBOX_H

#include <DFrame>
#include <DWidget>
#include <DPushButton>

#include <QMenu>
#include <QAction>

DWIDGET_USE_NAMESPACE

/*
* @bref: DMenuComboBox 该组件效果类似DComboBox下拉框效果，但是没有选中的箭头
*/

class DMenuComboBox : public DWidget
{
    Q_OBJECT
public:
    explicit DMenuComboBox(DWidget *parent = nullptr);

    /*
    * @bref: addItem 添加子选项
    */
    void addItem(const QString &itemText);
    void addItem(QString itemText, QIcon icon);
    void addItem(QAction *action);

    /*
    * @bref: removeItem 删除子选项
    */
    void removeItem(const QString &itemText);
    void removeItem(int index);
    void removeItem(QAction *action);

    /*
    * @bref: setCurrentIndex 设置选中项
    */
    void setCurrentIndex(int index);
//    int getCurrentIndex() const;

    /*
    * @bref: setCurrentText 设置选中项
    */
    void setCurrentText(const QString &text);
//    QString getCurrentText() const;

    /*
    * @bref: setMenuFlat 设置flat
    */
    void setMenuFlat(bool flat);

//    /*
//    * @bref: setArrowDirction 设置菜单箭头显示位置
//    */
//    void setArrowDirction(Qt::LayoutDirection dir = Qt::LayoutDirection::RightToLeft);

    /*
    * @bref: setArrowICon 设置菜单图标
    */
    void setArrowICon(QIcon icon);

    /*
    * @bref: setItemICon 设置子菜单图标
    * @parma: text 子菜单选项文本名字
    * @parma: index 子菜单选项索引
    */
    void setItemICon(const QString &text, QIcon icon);
    void setItemICon(int index, QIcon icon);
//    /*
//    * @bref: setMenuButtonICon 设置子菜单图标
//    * @parma: text 下拉按钮文本名字
//    * @parma: icon 下拉按钮图标
//    */
//    void setMenuButtonICon(QString text, QIcon icon);

//    /*
//    * @bref: updateButtonTextAndIcon 手动调用更新点击按钮的文字和图标，用于切换主题时调用
//    */
//    void updateButtonTextAndIcon();

//    /*
//    * @bref: setMenuMaxWidth 设置菜单最大宽度
//    */
//    void setMenuMaxWidth(int width);

signals:
    /*
    * @bref: signalCurrentIndexChanged 下标索引改变信号
    */
    void signalCurrentIndexChanged(int index);
    /*
    * @bref: signalCurrentTextChanged 文本改变信号
    */
    void signalCurrentTextChanged(QString text);
    /*
    * @bref: signalActionToggled 处理子选项点击事件
    */
    void signalActionToggled(QString text);
    /*
    * @bref: signalAboutToShow 菜单显示信号
    */
    void signalAboutToShow();

protected slots:
    /*
    * @bref: slotActionToggled 处理子选项点击事件
    */
    void slotActionToggled(QAction *action);

    /*
    * @bref: slotAboutToShow 显示菜单
    */
    void slotAboutToShow();

private:
    DPushButton *m_btn;
    QMenu *m_menu;
    QList<QAction *> m_actions;
    int m_currentIndex;

    void initUI();
    void initConnection();
};

#endif // DMENUCOMBOBOX_H

#endif
