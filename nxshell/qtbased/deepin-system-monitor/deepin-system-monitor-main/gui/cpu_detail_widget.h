// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CPU_DETAIL_WIDGET_H
#define CPU_DETAIL_WIDGET_H

#include <QWidget>
#include <DPushButton>
#include <QScrollArea>

#include "base/base_detail_view_widget.h"

class CPUInfoModel;
class QScrollArea;
class CPUDetailGrapTableItem : public QWidget
{
    Q_OBJECT

public:
    explicit CPUDetailGrapTableItem(CPUInfoModel *model, int index, QWidget *parent = nullptr);

    ~CPUDetailGrapTableItem();

    void setMode(int mode);

    inline void setMultiCoreMode(bool isMutilCoreMode) { m_isMutliCoreMode = isMutilCoreMode; }

    void sethorizontal(bool isHorizontalLast);

    void setVerticalLast(bool isVerticalLast);

    void setColor(QColor color);

public slots:
    void updateStat();

protected:
    void paintEvent(QPaintEvent *event);

    void drawNormalMode(QPainter &painter);

    void drawSimpleMode(QPainter &painter);

    void drawTextMode(QPainter &painter);

    //!
    //! \brief drawSingleCoreMode 绘制单核模式的视图
    //! \param painter 绘图工具类
    //!
    void drawSingleCoreMode(QPainter &painter);

    /**
     * @brief drawBackground
     * 绘制边框和表格线
     * @param painter
     * @param graphicRect
     */
    void drawBackground(QPainter &painter, const QRect &graphicRect);

private:
    QList<qreal>  m_cpuPercents;
    CPUInfoModel *m_cpuInfomodel = nullptr;
    QColor m_color;
    int m_mode  = 1;        //1:normal 2:simple 3:text
    int m_index = -1;
    bool m_isHorizontalLast = false;
    bool m_isVerticalLast = false;
    bool m_isMutliCoreMode = false; // 是否多核显示
};

class CPUDetailGrapTable : public QWidget
{
    Q_OBJECT
public:
    CPUDetailGrapTable(CPUInfoModel *model, QWidget *parent);

    //!
    //! \brief setMutliCoreMode 设置多核单核模式
    //! \param isMutliCoreMode 参数是否是多核模式
    //!
    void setMutliCoreMode(bool isMutliCoreMode);

    //!
    //! \brief setSingleModeLayout 设置单核模式布局
    //! \param model
    //!
    void setSingleModeLayout(CPUInfoModel *model);
    //!
    //! \brief setMultiModeLayout 设置多核模式布局
    //! \param model
    //!
    void setMultiModeLayout(CPUInfoModel *model);

private:
    bool m_isMutliCoreMode = false;

    CPUInfoModel *m_cpuInfoModel {};
};

class CPUDetailSummaryTable;
class CPUDetailWidget : public BaseDetailViewWidget
{
    Q_OBJECT

public:
    explicit CPUDetailWidget(QWidget *parent = nullptr);

private slots:
    void detailFontChanged(const QFont &font);

private:
    CPUDetailGrapTable *m_graphicsTable = nullptr;
    CPUDetailSummaryTable *m_summary = nullptr;
};

#endif // CPU_DETAIL_WIDGET_H
