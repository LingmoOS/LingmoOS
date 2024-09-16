// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BESTMATCHWIDGET_H
#define BESTMATCHWIDGET_H

#include <DWidget>
#include <QVBoxLayout>
#include <QLabel>

namespace GrandSearch {

class SwitchWidget;

class BestMatchWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit BestMatchWidget(QWidget *parent = nullptr);
    ~BestMatchWidget();

private slots:
    void onSwitchStateChanged(const bool checked);

private:
    void updateIcons();

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QLabel *m_groupLabel = nullptr;

    QHash<QString, QString> m_groupName;        // <searchGroupName, displayGroupName>
    QList<SwitchWidget*> m_switchWidgets;

    QStringList m_displayIcons;                 // 显示图标
};

}

#endif // BESTMATCHWIDGET_H
