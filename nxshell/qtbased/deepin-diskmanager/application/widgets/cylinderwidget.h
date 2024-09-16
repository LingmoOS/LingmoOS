// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef CYLINDERWIDGET_H
#define CYLINDERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVariant>

/**
 * @class CylinderWidget
 * @brief 柱面小方块类
 */

class CylinderWidget : public QLabel
{
    Q_OBJECT
public:
    explicit CylinderWidget(QWidget *parent = nullptr);

    /**
     * @brief 设置当前柱面的检测信息数据
     * @param userData 当前柱面检测信息
     */
    void setUserData(const QVariant &userData);

    /**
     * @brief 获取当前柱面的检测信息数据
     * @return 当前柱面检测信息
     */
    QVariant getUserData();

signals:
    /**
     * @brief 鼠标进入信号
     */
    void enter();

    /**
     * @brief 鼠标离开信号
     */
    void leave();

public slots:

protected:
    /**
     * @brief 重写进入事件
     */
    void enterEvent(QEvent *event);

    /**
     * @brief 重写离开事件
     */
    void leaveEvent(QEvent *event);

private:
    QVariant m_userData;
};

#endif // CYLINDERWIDGET_H
