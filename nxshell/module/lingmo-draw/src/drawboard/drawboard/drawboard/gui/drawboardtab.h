// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CMULTIPTABBARWIDGET_H
#define CMULTIPTABBARWIDGET_H

#include "config.h"

#include <QMenu>
#include <QFileDialog>
#include <QMouseEvent>

#ifdef USE_DTK
#include <DTabBar>
#define TABBAR DTabBar
DWIDGET_USE_NAMESPACE
#else
#include <QTabBar>
#define TABBAR QTabBar
#endif

#include <QStackedWidget>

/**
 * @description: 实现多标签拖拽选项栏
*/
class DrawBoard;
class TabBarWgt : public TABBAR
{
    Q_OBJECT
private:
    explicit TabBarWgt(DrawBoard *parent);
    ~TabBarWgt();

public:
    DrawBoard *pageManager() const;

    void addItem(const QString &name, const QString &key);
    void removeItem(const QString &key);

    int index(const QString &key)const;
    QString key(int index) const;

    Q_SLOT void onTabCountChanged(int index);


private:
    QMenu *menu()const;
    void mousePressEvent(QMouseEvent *event) override;

    friend class DrawBoard;
};

class FileSelectDialog: public QFileDialog
{
    Q_OBJECT
public:
    explicit FileSelectDialog(DrawBoard *parent = nullptr);

    DrawBoard *borad() const;

    int exec() override;

    QString resultFile()const;
private:
    void saveSetting();

private:
    QString _resultFile;

    friend class DrawBoard;

    friend class Page;
};


class PageStackedWidget: public QStackedWidget
{
    Q_OBJECT
public:
    using QStackedWidget::QStackedWidget;
    ~PageStackedWidget();
};

#endif // CMULTIPTABBARWIDGET_H
