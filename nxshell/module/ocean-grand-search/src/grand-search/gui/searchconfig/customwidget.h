// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMWIDGET_H
#define CUSTOMWIDGET_H

#include <DWidget>
#include <QVBoxLayout>
#include <QLabel>

namespace GrandSearch {

class PlanWidget;
class BestMatchWidget;
class TailerWidget;
class SearchEngineWidget;
class SwitchWidget;
class CustomWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit CustomWidget(QWidget *parent = nullptr);
    ~CustomWidget();

private slots:
    void onSwitchStateChanged(const bool checked);

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_groupLabel = nullptr;
    PlanWidget *m_searchPlanWidget = nullptr;
    TailerWidget *m_tailerWidget = nullptr;
    BestMatchWidget *m_bestMatchWidget = nullptr;
    SearchEngineWidget *m_searchEngineWidget = nullptr;
    QVBoxLayout *m_innerLayout = nullptr;

    QHash<QString, QString> m_groupName;        // <searchGroupName, displayGroupName>
    QList<SwitchWidget*> m_switchWidgets;

    QStringList m_displayIcons;                 // 显示图标
};

}

#endif // CUSTOMWIDGET_H
