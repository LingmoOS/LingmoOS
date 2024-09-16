// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef VGINFOSHOWWIDGET_H
#define VGINFOSHOWWIDGET_H

#include <QWidget>

#include <DFrame>
#include <DScrollArea>

DWIDGET_USE_NAMESPACE

/**
 * @class VGInfoShowWidget
 * @brief 逻辑卷组下的逻辑卷信息展示类，主界面图示区域“…”鼠标悬浮窗口
 */

class QGridLayout;

class VGInfoShowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VGInfoShowWidget(QWidget *parent = nullptr);

    void setData(const QList< QMap<QString, QVariant> > &lstInfo);

signals:

public slots:

private:
    /**
     * @brief 初始化界面
     */
    void initUi();

    /**
     * @brief 初始化连接
     */
    void initConnection();
};

#endif // VGINFOSHOWWIDGET_H
