// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEDETAILVIEWWIDGET_H
#define BASEDETAILVIEWWIDGET_H

#include <QWidget>
#include <DCommandLinkButton>
#include <DApplicationHelper>
#include <DIconButton>
#include <QVBoxLayout>
#include <QPushButton>

class BaseCommandLinkButton;
DWIDGET_USE_NAMESPACE
class BaseDetailViewWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString detail READ detail WRITE setDetail)

public:
    explicit BaseDetailViewWidget(QWidget *parent = nullptr);

    virtual ~BaseDetailViewWidget();

signals:
    void signalHideDetailClicked();

    void sigClickSwitchMutliCoreButton(bool isMutilCoreMode);

public:
    void setTitle(const QString &text);
    QString title();

    void setDetail(const QString &text);
    QString detail();

    int titleHeight();

public slots:
    void detailFontChanged(const QFont &font);

private:
    void updateWidgetGrometry();

    //!
    //! \brief onThemeTypeChanged 主题背景切换
    //!
    void onThemeTypeChanged(DGuiApplicationHelper::ColorType themeType);

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

protected:
    QFont m_contentFont;
    QFont m_titleFont;

    QString m_titleText;
    QString m_detailText;

    BaseCommandLinkButton *m_detailButton;
    Dtk::Widget::DIconButton *m_arrowButton;

    // 切换多核显示或单CPU显示  按钮
    Dtk::Widget::DIconButton *m_switchButton;
    // 切换按钮图片
    QIcon *m_switchIconLight {};
    QIcon *m_switchIconDark {};

    // 当前是否为多核模式
    bool m_isMultiCoreMode = false;

    QVBoxLayout *m_centralLayout;
};

#endif // BASEDETAILVIEWWIDGET_H
