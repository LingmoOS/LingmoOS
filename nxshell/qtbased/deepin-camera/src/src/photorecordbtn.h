// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PHOTO_RECORD_BTN_H
#define PHOTO_RECORD_BTN_H

#include <QPushButton>

class QColor;
class QSvgRenderer;
class photoRecordBtn : public QWidget
{
    Q_OBJECT
public:
    typedef enum _record_state{
        Normal     = 0,         //未开始
        preRecord  = 1,         //倒计时
        Recording  = 2          //正在录像
    }RecordState;

    explicit photoRecordBtn(QWidget *parent = nullptr);

    /**
     * @brief setState 设置拍照，录像状态
     * @param bPhoto  true 拍照状态， false 录像状态
     */
    void setState(bool bPhoto) {m_bPhoto = bPhoto; update();}

    /**
     * @brief setRecordState 设置录像状态
     * @param state 录像状态，相见上枚举定义
     */
    void setRecordState(int state) {m_recordState = state; update();}

    /**
     * @brief photoState 获取是否拍照状态
     */
    bool photoState() {return m_bPhoto;}
signals:
    void clicked();
protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    bool m_bPhoto;          //true 拍照状态， false 录像状态
    bool m_bFocus;          //焦点   hover或者tab键选中
    bool m_bPress;          //press
    int  m_recordState;     //录制状态
};

#endif // CIRCLEPUSHBUTTON_H
