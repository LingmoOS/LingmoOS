// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENSIONPANEL_H
#define EXTENSIONPANEL_H

#include <DDialog>
#include <DFloatingWidget>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QScrollArea>
#include <DTitlebar>
#include <QShortcut>

#include "widgets/blureframe.h"
#include <DWidget>
DWIDGET_USE_NAMESPACE
typedef DWidget QWdToDWidget;

class ExtensionPanel : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit ExtensionPanel(QWidget *parent = nullptr);
    void setContent(QWidget *content);

//    void moveWithAnimation(int x, int y);

    void init();
signals:
    void requestStopAnimation();
public slots:
    void updateRectWithContent(int height);
private:
    QColor m_coverBrush;
    QWdToDWidget *m_content;
    QVBoxLayout *m_contentLayout;

    QVBoxLayout *m_mainLayout {nullptr};
    QScrollArea *m_scrollArea {nullptr};

    Dtk::Widget::DTitlebar *m_titleBar {nullptr};

    QShortcut *m_scImageInfo{nullptr};
    QShortcut *m_scImageInfonum{nullptr};
    QShortcut *m_scEsc {nullptr};
};

#endif  // EXTENSIONPANEL_H
