// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SHOTTOOLWIDGET_H
#define SHOTTOOLWIDGET_H

#include "toolbutton.h"
#include "filter.h"

#include <DLabel>
#include <DStackedWidget>

#include <QObject>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
class MainWindow;
class ShotToolWidget : public DStackedWidget
{
    Q_OBJECT
public:
    explicit ShotToolWidget(MainWindow *pmainwindow, DWidget *parent = nullptr);
    ~ShotToolWidget();

    void initWidget();
    //截屏文本按钮二级控件初始化
    void initTextLabel();

    void initEffectLabel();

    //粗细控件初始化（此控件针对矩形、圆形、直线、箭头、画笔）
    void initThicknessLabel();

    void installTipHint(QWidget *w, const QString &hintstr);

signals:
    void changeArrowAndLine(int line); //0 for line, 1 for arrow

public slots:
    void switchContent(QString shapeName);
    void colorChecked(QString colorType);

private:

    DLabel *m_textSubTool;
    /**
     * @brief 粗细调整面板
     */
    DLabel *m_thicknessLabel;


    DLabel *m_effectSubTool;
    /**
     * @brief 当前选中的图形名称
     */
    QString m_shapeName;

    /**
     * @brief 粗细面板初始化标志
     */
    bool m_thicknessInitFlag;

    bool m_textInitFlag;

    QString m_currentType;

    QButtonGroup *m_thicknessBtnGroup;

    HintFilter *hintFilter = nullptr;

    MainWindow *m_pMainWindow = nullptr;
};

#endif // SHOTTOOLWIDGET_H
