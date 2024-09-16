// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGECONTROL_H
#define PAGECONTROL_H

#include <DFloatingButton>

#include <QWidget>

DWIDGET_USE_NAMESPACE

#define     PAGE_ICON_SPACE  0
#define     PAGE_ICON_SIZE   10
#define     PAGE_BUTTON_SIZE 20

class PageControl : public QWidget
{
    Q_OBJECT

public:
    explicit PageControl(QWidget *parent = nullptr);

    void setPageCount(int count);
    void setCurrent(int pageIndex);
    void updateIconSize(double scaleX, double scaleY);
    void createButtons();

protected:
    void paintEvent(QPaintEvent *event);

signals:
    void onPageChanged(int pageIndex);

public slots:
    void pageBtnClicked(bool checked = false);

private:
    void addButton(DIconButton *pageButton);

private:
    int m_pageCount;
    QPixmap m_pixChecked;
    QList<DIconButton *> m_buttonList;
};

#endif // PAGECONTROL_H
