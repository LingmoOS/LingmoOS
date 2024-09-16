// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 *@file 这个文件是影片信息窗口相关的类
 */
#ifndef _DMR_MOVIE_INFO_DIALOG_H
#define _DMR_MOVIE_INFO_DIALOG_H

#include <QtWidgets>
#include <QGuiApplication>
#include <QPainterPath>

#include <DDialog>
#include <DImageButton>
#include <DLabel>
#include <DFontSizeManager>
#include <DThemeManager>
#include <DApplicationHelper>
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DArrowLineDrawer>

#include "../accessibility/ac-deepin-movie-define.h"

#define LINE_MAX_WIDTH 200
#define LINE_HEIGHT 30
const QString LOGO_BIG = ":/resources/icons/logo-big.svg";
const QString INFO_CLOSE_LIGHT = ":/resources/icons/light/info_close_light.svg";
const QString INFO_CLOSE_DARK = ":/resources/icons/dark/info_close_dark.svg";

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace dmr {
struct PlayItemInfo;
/**
 * @brief The PosterFrame class
 * 影片信息中的缩略图控件
 */
class PosterFrame: public QLabel
{
    Q_OBJECT
public:
    /**
     * @brief PosterFrame 构造函数
     * @param parent 父窗口
     */
    explicit PosterFrame(QWidget *parent) : QLabel(parent)
    {
        QGraphicsDropShadowEffect *pShadowEffect = new QGraphicsDropShadowEffect(this);
        //参考设计图
        pShadowEffect->setColor(QColor(0, 0, 0, 76));
        pShadowEffect->setOffset(0, 3);
        pShadowEffect->setBlurRadius(6);
        setGraphicsEffect(pShadowEffect);
    }
};

/**
 * @brief The InfoBottom class
 * 这个是一个信息的窗口，在影片信息中有多个信息分类，
 * 每个分类占一个这样的小窗口
 */
class InfoBottom: public DFrame
{
    Q_OBJECT
public:
    /**
     * @brief InfoBottom 构造函数
     */
    InfoBottom() {}

protected:
    /**
     * @brief paintEvent 重载绘制事件函数
     * @param pPaintEvent 绘制事件
     */
    virtual void paintEvent(QPaintEvent *pPaintEvent)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        //这部分颜色参考设计图
        if (DGuiApplicationHelper::LightType == DGuiApplicationHelper::instance()->themeType()) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(QColor(255, 255, 255, 0)));
        } else if (DGuiApplicationHelper::DarkType == DGuiApplicationHelper::instance()->themeType()) {
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(QColor(45, 45, 45, 0)));
        }

        QRect rect = this->rect();
        rect.setWidth(rect.width() - 1);
        rect.setHeight(rect.height() - 1);

        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, 10, 10);
        painter.drawPath(painterPath);
    }
};

/**
 * @brief The ArrowLine class
 * 这个类是负责将每个信息分类进行折叠
 */
class ArrowLine: public DArrowLineDrawer
{
    Q_OBJECT
public:
    /**
     * @brief ArrowLine 构造函数
     */
    ArrowLine() {}

protected:
    /**
     * @brief paintEvent 重载绘制事件函数
     * @param pPaintEvent 绘制事件
     */
    virtual void paintEvent(QPaintEvent *pPaintEvent)
    {
        Q_UNUSED(pPaintEvent);
        QPainter painter(this);
        QRectF bgRect;
        bgRect.setSize(size());
        const QPalette pal = QGuiApplication::palette();
        QColor bgColor = pal.color(QPalette::Background);

        QPainterPath path;
        path.addRoundedRect(bgRect, 8, 8);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillPath(path, bgColor);
        painter.setRenderHint(QPainter::Antialiasing, false);
    }
};
/**
 * @brief The MovieInfoDialog class
 * 这个类是影片信息大窗口的类
 */
class MovieInfoDialog: public DDialog
{
    Q_OBJECT
public:
    /**
     * @brief MovieInfoDialog 构造函数
     * @param strPlayItemInfo 播放项的相关信息
     * @param pParent 父窗口
     */
    MovieInfoDialog(const struct PlayItemInfo &strPlayItemInfo, QWidget *pParent);

protected:
    /**
     * @brief paintEvent 重载绘制事件函数
     * @param pPaintEvent 绘制事件
     */
    void paintEvent(QPaintEvent *pPaintEvent);

    void showEvent(QShowEvent *pEvent);

private slots:
    /**
     * @brief onFontChanged 字体变化槽函数（跟随系统变化）
     * @param font 变化后的字体
     */
    void onFontChanged(const QFont &font);
    /**
     * @brief changedHeight 高度变化槽函数
     */
    void changedHeight(const int);
    //把lambda表达式改为槽函数，modify by myk
    /**
     * @brief slotThemeTypeChanged
     * 主题变化槽函数
     */
    void slotThemeTypeChanged();

private:
    /**
     * @brief addRow 增加信息条目函数
     * @param title 信息名称
     * @param field 信息内容
     * @param form 布局管理
     * @retval tipList 传回当前添加信息的label控件
     */
    void addRow(QString sTitle, QString sField, QFormLayout *pForm, QList<DLabel *> &tipList);
    /**
     * @brief initMember 初始化成员变量
     */
    void initMember();

private:
    DLabel *m_pFileNameLbl;             ///文件名控件
    DLabel *m_pFilePathLbl;             ///文件路径控件
    QString m_sFilePath;                ///文件路径
    QList<DLabel *> m_titleList;        ///信息label控件列表
    QList<DDrawer *> m_expandGroup;     ///信息分类组列表
    QScrollArea *m_pScrollArea;         ///包含多个分类组的滚动条窗口
    int m_nLastHeight;                  ///保存上次文本高度
};


}

#endif /* ifndef _DMR_MOVIE_INFO_DIALOG_H */
