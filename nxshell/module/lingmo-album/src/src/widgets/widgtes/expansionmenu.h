// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXPANSIONMENU_H
#define EXPANSIONMENU_H

#include <QWidget>
#include <QLabel>
#include <DCommandLinkButton>
#include "toolbutton.h"
#include "expansionpanel.h"

class FilterLabel : public QLabel
{
    Q_OBJECT

public:
    explicit FilterLabel(QWidget *parent);
    ~FilterLabel() override;
protected:
    void mouseReleaseEvent(QMouseEvent *ev) override;
signals:
    void clicked();
public:
};

class FilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilterWidget(QWidget *parent);
    ~FilterWidget() override;
    void resizeEvent(QResizeEvent *e) override;

    void setIcon(QIcon icon);
    void setText(QString text);
    void setFilteData(ExpansionPanel::FilteData &data);
    ExpansionPanel::FilteData getFilterData();
    ItemType getFilteType();
public slots:
    void onClicked();
    //主题变化
    void themeTypeChanged(int type);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
signals:
    void clicked();
    void currentItemChanged(ExpansionPanel::FilteData &data);
    void sigWidthChanged(int width);
public:
    FilterLabel        *m_btn = nullptr;
    FilterLabel        *m_leftLabel = nullptr;
    FilterLabel        *m_rightLabel = nullptr;
    ExpansionPanel::FilteData m_data;
};

class ExpansionMenu : public QObject
{
    Q_OBJECT
public:
    explicit ExpansionMenu(QWidget *parent = nullptr);

    FilterWidget *mainWidget();

    //动态添加按钮
    void addNewButton(ExpansionPanel::FilteData &data);
    void setDefaultFilteData(ExpansionPanel::FilteData &data);

signals:

public slots:

private slots:
    void onCurrentItemChanged(ExpansionPanel::FilteData &data);
    void onMainButtonClicked();

private:
    FilterWidget *mainButton = nullptr;
    ExpansionPanel *panel;
};

#endif // EXPANSIONMENU_H
