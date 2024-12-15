// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PLANWIDGET_H
#define PLANWIDGET_H

#include <DWidget>
#include <DLabel>
#include <DSwitchButton>

#include <QVBoxLayout>

namespace GrandSearch {

class SwitchWidget;
class PlanWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit PlanWidget(QWidget *parent = nullptr);
    ~PlanWidget();

private slots:
    void onSwitchStateChanged(const bool checked);

private:
    void updateIcons();

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_groupLabel = nullptr;
    QLabel *m_contentLabel = nullptr;
    QLabel *m_tipsLabel = nullptr;

    QHash<QString, QString> m_groupName;        // <searchGroupName, displayGroupName>
    SwitchWidget *m_switchWidget;

    QString m_displayIcon;                      // 显示图标
};

}

#endif // PLANWIDGET_H
