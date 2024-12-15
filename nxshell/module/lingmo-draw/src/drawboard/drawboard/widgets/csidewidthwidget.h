// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CSIDEWIDTHWIDGET_H
#define CSIDEWIDTHWIDGET_H

#include <globaldefine.h>

#include <QFrame>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QComboBox>

class DRAWLIB_EXPORT SideWidthWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SideWidthWidget(QWidget *parent = nullptr);

    void setText(const QString &text);

    void setSpace(int space);

    void setWidth(int width);
    QComboBox *menuComboBox();

    QSize sizeHint()const override;

signals:
    void widthChanged(int width, bool preview);

public:
    void setValid(bool valid);

private:
    QComboBox *m_menuComboBox;// 线宽度选择下拉框
    QLabel *_textLabel = nullptr;
    QHBoxLayout *m_layout;    // 水平布局器
    quint8 m_comboxHeight;    // 下拉框的高度
    QLabel *m_maskLable;

private:
    void initUI();
    void initConnection();

    /*
     * @bref: initLineWidthToCombox 初始化线宽度
    */
    void initLineWidthToCombox();
};

#endif // CSIDEWIDTHWIDGET_H
