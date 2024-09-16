// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENTRANCEWIDGET_P_H
#define ENTRANCEWIDGET_P_H

#include "entrancewidget.h"

#include <DWidget>

#include "QObject"

DWIDGET_BEGIN_NAMESPACE
class DSearchEdit;
class DLabel;
DWIDGET_END_NAMESPACE

class QHBoxLayout;
class QTimer;
class QLineEdit;
class QAction;
class QPushButton;

namespace GrandSearch {

class EntranceWidgetPrivate : public QObject
{
    Q_OBJECT
public:
    explicit EntranceWidgetPrivate(EntranceWidget *parent = nullptr);

    void delayChangeText();
    void notifyTextChanged();

    void showMenu(const QPoint& pos);

    EntranceWidget *q_p = nullptr;
    Dtk::Widget::DSearchEdit *m_searchEdit = nullptr;   // 搜索输入框控件
    QLineEdit *m_lineEdit = nullptr;                    // 输入控件
    Dtk::Widget::DLabel *m_appIconLabel = nullptr;      // 应用图标显示label
    QAction *m_appIconAction = nullptr;                 // 应用图标显示区域占位action
    QHBoxLayout *m_mainLayout = nullptr;

    QTimer *m_delayChangeTimer = nullptr;               // 延迟发出搜索文本改变

    QString m_appIconName;                              // 当前搜索框显示的默认打开应用图标名称
};

}

#endif // ENTRANCEWIDGET_P_H
