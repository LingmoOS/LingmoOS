// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef DDBASE_H
#define DDBASE_H

#include <DDialog>
#include <DFrame>

#include <QAbstractButton>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

/**
 * @class DDBase
 * @brief 基础窗口类
*/

class DDBase : public DDialog
{
    Q_OBJECT
public:
    explicit DDBase(QWidget *parent = nullptr);

    DFrame *m_mainFrame = nullptr;
    int m_okCode = -1;

protected:
    /**
     * @brief 键盘按下事件
     * @param event事件
     */
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // DDBASE_H
