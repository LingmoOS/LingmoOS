// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CDYNAMICICON_H
#define CDYNAMICICON_H

#include <DApplication>
#include <DTitlebar>

#include <QPixmap>
#include <QDate>
#include <QIcon>
#include <QSvgRenderer>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

class CDynamicIcon
{
private:
    explicit CDynamicIcon(int width, int height);
    ~CDynamicIcon();
    //禁用拷贝构造，拷贝赋值
    CDynamicIcon(const CDynamicIcon &) = delete;
    CDynamicIcon &operator=(const CDynamicIcon &) = delete;

public:
    static CDynamicIcon *getInstance();
    static void releaseInstance();
public:
    void setDate(const QDate &date);
    QDate getDate() const
    {
        return m_Date;
    }
    void setIcon();
    void setTitlebar(DTitlebar *titlebar);
    QPixmap getPixmap() const
    {
        return *m_pixmap;
    }
public:
    static CDynamicIcon *m_Icon;
private:
    void paintPixmap(QPixmap *pixmap);
private:
    QPixmap *m_pixmap = nullptr;
    QDate m_Date;

    QSvgRenderer *m_Dayrenderer = nullptr;
    QSvgRenderer *m_Weekrenderer = nullptr;
    QSvgRenderer *m_Monthrenderer = nullptr;
    QSvgRenderer *m_backgroundrenderer = nullptr;

    DTitlebar       *m_Titlebar {nullptr};


};

#endif // CDYNAMICICON_H
