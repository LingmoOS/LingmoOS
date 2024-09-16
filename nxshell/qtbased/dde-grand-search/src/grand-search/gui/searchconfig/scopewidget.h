// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCOPEWIDGET_H
#define SCOPEWIDGET_H

#include <DWidget>
#include <DLabel>
#include <DSwitchButton>

#include <QVBoxLayout>

namespace GrandSearch {

class SwitchWidget;
class ScopeWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit ScopeWidget(QWidget *parent = nullptr);
    ~ScopeWidget();

private slots:
    void onSwitchStateChanged(const bool checked);

private:
    void updateIcons();
    bool isValid(const QString &item);

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_groupLabel = nullptr;

    QHash<QString, QString> m_groupName;        // <searchGroupName, displayGroupName>
    QList<SwitchWidget*> m_switchWidgets;

    QStringList m_displayIcons;                 // 显示图标
};

}

#endif // SCOPEWIDGET_H
