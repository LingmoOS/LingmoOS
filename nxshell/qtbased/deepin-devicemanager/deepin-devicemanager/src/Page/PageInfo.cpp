// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "PageInfo.h"
#include "MacroDefinition.h"

// Dtk头文件
#include <DApplicationHelper>
#include <DApplication>

// Qt库文件
#include <QPainter>
#include <QStyleOptionFrame>
#include <QLoggingCategory>
#include <QPainterPath>
#include <QProcess>


DWIDGET_USE_NAMESPACE
PageInfo::PageInfo(QWidget *parent)
    : QWidget(parent)
    , m_AllInfoNum(0)
    , m_multiFlag(false)
{

}

void PageInfo::updateInfo(const QMap<QString, QString> &)
{

}

void PageInfo::setLabel(const QString &, const QString &)
{

}

void PageInfo::clearContent()
{

}

void PageInfo::setDeviceInfoNum(int num)
{
    // 设置设备信息数目
    m_AllInfoNum = num;
}

int PageInfo::getDeviceInfoNum()
{
    // 获取设备信息数目
    return m_AllInfoNum;
}

void PageInfo::setMultiFlag(const bool &flag)
{
    m_multiFlag = flag;
}

bool PageInfo::getMultiFlag()
{
    return m_multiFlag;
}

bool PageInfo::packageHasInstalled(const QString &packageName)
{
    QProcess p;
    QString cmd = "dpkg -s " + packageName;
    p.start(cmd);
    p.waitForFinished(-1);

    QByteArray r = p.readAll();
    return r.contains("installed");
}

void PageInfo::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.save();
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setOpacity(1);
    painter.setClipping(true);
    QRect rect = this->rect();

    // 获取调色板
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();

    // 获取系统默认的圆角半径
    int radius = 8;

    // 获取窗口当前的状态,激活，禁用，未激活
    DPalette::ColorGroup cg;
    DWidget *wid = DApplication::activeWindow();
    if (wid /* && wid == this*/)
        cg = DPalette::Active;
    else
        cg = DPalette::Inactive;

    // 开始绘制边框 *********************************************************
    // 计算绘制区域
    QPainterPath paintPath;
    paintPath.addRoundedRect(rect, radius, radius);
    QBrush bgBrush(palette.color(cg, DPalette::Base));
    painter.fillPath(paintPath, bgBrush);

    painter.restore();
    DWidget::paintEvent(e);
}

